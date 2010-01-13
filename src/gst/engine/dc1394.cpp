// dc1394.cpp
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
//
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// [propulse]ART is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//

#include "util.h"
#include "./dc1394.h"

#include <boost/filesystem/operations.hpp>
#include <fstream>
#include <dc1394/control.h>
#include <libraw1394/raw1394.h>
#include <iomanip>

int cleanup(dc1394_t *& dc1394, dc1394camera_t *& camera, dc1394camera_list_t *& cameras)
{
    if (dc1394)
    {
        dc1394_free(dc1394);
        dc1394 = 0;
    }

    if (cameras) {
        dc1394_camera_free_list(cameras);
        cameras = NULL;
    }

    if (camera) {
        dc1394_camera_free(camera);
        camera = NULL;
    }
    return 1;
}


int dc1394_caps_print_format_vmode_caps(int mode)
{
  int retval = 0;

  switch (mode) 
  {
#define PRINT_CASE(x) \
            case (DC1394_VIDEO_MODE_ ##x): \
                      LOG_PRINT("\t" << # x << " (vmode " << mode << ")" << std::endl); \
            break

    PRINT_CASE(160x120_YUV444);
    PRINT_CASE(320x240_YUV422);
    PRINT_CASE(640x480_YUV411);
    PRINT_CASE(640x480_YUV422);
    PRINT_CASE(640x480_RGB8);
    PRINT_CASE(640x480_MONO8);
    PRINT_CASE(640x480_MONO16);
    PRINT_CASE(800x600_YUV422);
    PRINT_CASE(800x600_RGB8);
    PRINT_CASE(800x600_MONO8);
    PRINT_CASE(1024x768_YUV422);
    PRINT_CASE(1024x768_RGB8);
    PRINT_CASE(1024x768_MONO8);
    PRINT_CASE(800x600_MONO16);
    PRINT_CASE(1024x768_MONO16);
    PRINT_CASE(1280x960_YUV422);
    PRINT_CASE(1280x960_RGB8);
    PRINT_CASE(1280x960_MONO8);
    PRINT_CASE(1600x1200_YUV422);
    PRINT_CASE(1600x1200_RGB8);
    PRINT_CASE(1600x1200_MONO8);
    PRINT_CASE(1280x960_MONO16);
    PRINT_CASE(1600x1200_MONO16);

    default:
        LOG_WARNING("Unknown vmode " << mode);
        retval = -1;
  }
  return retval;
#undef PRINT_CASE
}


/// FIXME: add checks for other framerates
bool modeIsSupported(int mode, const dc1394video_modes_t &supportedModes, 
        int framerate, dc1394camera_t *camera)
{
    bool frameratesMatch = false;
    for (int i = supportedModes.num - 1; i >= 0; --i)
    {
        int m = supportedModes.modes[i];
        if (m < DC1394_VIDEO_MODE_EXIF) 
        {
            if (m == mode)
            {
                dc1394framerates_t framerates;
                dc1394_video_get_supported_framerates(camera,
                        (dc1394video_mode_t) m, &framerates);
                for (unsigned framerateIdx = 0; !frameratesMatch and framerateIdx < framerates.num; 
                        ++framerateIdx)
                {
                    switch (framerates.framerates[framerateIdx])
                    {
                        case DC1394_FRAMERATE_30:
                            frameratesMatch = (framerate == 30);
                            break;
                        case DC1394_FRAMERATE_15:
                            frameratesMatch = (framerate == 15);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
    if (!frameratesMatch)
        LOG_WARNING("So far only 15 fps and 30 fps are supported");
    return frameratesMatch;
}


/// FIXME: have all the modes here and this will also depend on framerate
bool DC1394::requiresMoreISOSpeed(int mode)
{
    switch (mode)
    {
        // using fall through instead of logical or
        case DC1394_VIDEO_MODE_1280x960_MONO16:
        case DC1394_VIDEO_MODE_1280x960_MONO8:
        case DC1394_VIDEO_MODE_1280x960_RGB8:
        case DC1394_VIDEO_MODE_1280x960_YUV422:
        case DC1394_VIDEO_MODE_1024x768_MONO16:
        case DC1394_VIDEO_MODE_1024x768_MONO8:
        case DC1394_VIDEO_MODE_1024x768_RGB8:
        case DC1394_VIDEO_MODE_1024x768_YUV422:
        case DC1394_VIDEO_MODE_800x600_RGB8:
            return true;
        default:
            return false;
    }
}

/// FIXME: replace with table-driven method
int DC1394::capsToMode(int cameraId, int width, int height, 
        const std::string &colourspace, int framerate)
{
    int mode = 0;
    dc1394video_modes_t modes;
    dc1394error_t camerr;
    dc1394_t * dc1394 = dc1394_new();
    dc1394camera_list_t *cameras = 0;
    camerr = dc1394_camera_enumerate(dc1394, &cameras);

    dc1394camera_t *camera =
        dc1394_camera_new_unit (dc1394, cameras->ids[cameraId].guid,
                cameras->ids[cameraId].unit);

    camerr = dc1394_video_get_supported_modes(camera, &modes);
    if (camerr != DC1394_SUCCESS) 
    {
        LOG_ERROR("Error getting supported modes\n");
        cleanup(dc1394, camera, cameras);
        return 0;
    }

#define RETURN_MODE_FROM_CAPS(WIDTH, HEIGHT, COLOURSPACE)  \
    (width == (WIDTH) and height == (HEIGHT))  \
    {                                             \
        int testMode = DC1394_VIDEO_MODE_ ## WIDTH ##x ## HEIGHT ##_ ## COLOURSPACE; \
        if (modeIsSupported(testMode, modes, framerate, camera))    \
        {                                       \
            mode = testMode;                    \
            LOG_DEBUG("Using mode " << mode);   \
            return mode;        \
        }   \
    } 

    if (colourspace == "yuv")
    {
        // 640x480 first because it's the nominal case)
        // FIXME: should be else ifs unless resolutions match
        if RETURN_MODE_FROM_CAPS(640, 480, YUV422)
            if RETURN_MODE_FROM_CAPS(640, 480, YUV411)
                if RETURN_MODE_FROM_CAPS(160, 120, YUV444) 
                    if RETURN_MODE_FROM_CAPS(320, 240, YUV422)
                        if RETURN_MODE_FROM_CAPS(800, 600, YUV422)
                            if RETURN_MODE_FROM_CAPS(1024, 768, YUV422)
                                if RETURN_MODE_FROM_CAPS(1280, 960, YUV422)
                                    if RETURN_MODE_FROM_CAPS(1600, 1200, YUV422)

                                        LOG_DEBUG("Colourspace " << colourspace << " and resolution "
                                                << width << "x" << height << " are not supported by this camera");
    }
    else if (colourspace == "gray")
    {
        // not else if because resolutions are the same
        if RETURN_MODE_FROM_CAPS(640, 480, MONO8)
            if RETURN_MODE_FROM_CAPS(640, 480, MONO16)
                if RETURN_MODE_FROM_CAPS(800, 600, MONO8)
                    if RETURN_MODE_FROM_CAPS(800, 600, MONO16)
                        if RETURN_MODE_FROM_CAPS(1024, 768, MONO8)
                            if RETURN_MODE_FROM_CAPS(1024, 768, MONO16)
                                if RETURN_MODE_FROM_CAPS(1280, 960, MONO8)
                                    if RETURN_MODE_FROM_CAPS(1600, 1200, MONO8)
                                        if RETURN_MODE_FROM_CAPS(1280, 960, MONO16)
                                            // FIXME:
                                            // This could be supported but won't work unless you use
                                            // dc1394_video_set_operation_mode(camera->camera_info, DC1394_OPERATION_MODE_1394B);
                                            // and then increase the ISO speed to 800
                                            // dc1394_video_set_iso_speed(camera->camera_info,DC1394_ISO_SPEED_800);
                                            if RETURN_MODE_FROM_CAPS(1600, 1200, MONO16)

                                                LOG_WARNING("Colourspace " << colourspace << " and resolution "
                                                        << width << "x" << height << " are not supported by this camera");
    }
    else if (colourspace == "rgb")
    {
        if RETURN_MODE_FROM_CAPS(640, 480, RGB8)
            if RETURN_MODE_FROM_CAPS(800, 600, RGB8)
                if RETURN_MODE_FROM_CAPS(1024, 768, RGB8)
                    if RETURN_MODE_FROM_CAPS(1280, 960, RGB8)
                        if RETURN_MODE_FROM_CAPS(1600, 1200, RGB8)

                            LOG_WARNING("Colourspace " << colourspace << " and resolution "
                                    << width << "x" << height << " are not supported by this camera");
    }
    else
        THROW_ERROR("Invalid colourspace " << colourspace);

    /// cleanup camera resources
    dc1394_camera_free(camera);
    camera = 0;
    cleanup(dc1394, camera, cameras);

#undef RETURN_MODE_FROM_CAPS
    return 0; 
}


void printSupportedFramerates(dc1394framerates_t framerates)
{
    LOG_PRINT("\tFramerates: ");
    std::string delimiter;

#define PRINT_CASE_FRACTION(whole, fraction) \
    case (DC1394_FRAMERATE_ ## whole ## _ ## fraction): \
                                                        LOG_PRINT(# whole  << "." << # fraction); \
    break

#define PRINT_CASE(whole) \
    case (DC1394_FRAMERATE_ ## whole): \
                                       LOG_PRINT(# whole); \
    break

    for (unsigned framerateIdx = 0; framerateIdx < framerates.num; ++framerateIdx)
    {
        LOG_PRINT(delimiter);
        switch (framerates.framerates[framerateIdx])
        {

            PRINT_CASE_FRACTION(1, 875);
            PRINT_CASE_FRACTION(3, 75);
            PRINT_CASE_FRACTION(7, 5);
            PRINT_CASE(15);
            PRINT_CASE(30);
            PRINT_CASE(60);
            PRINT_CASE(120);
            PRINT_CASE(240);
        }
        delimiter = ",";
    }
    LOG_PRINT("\n");
#undef PRINT_CASE
#undef PRINT_CASE_FRACTION
}

bool isModuleReadable(const std::string &module)
{
    std::string path("/dev/" + module);
    std::ifstream input(path.c_str());
    bool readable = input.good();
    input.close();
    return readable;
}


bool isModuleWriteable(const std::string &module)
{
    std::string path("/dev/" + module);
    std::ofstream output(path.c_str());
    bool writeable = output.good();
    output.close();
    return writeable;
}


void DC1394::listCameras()
{
    LOG_DEBUG("listing cameras");

    dc1394error_t camerr;
    dc1394video_modes_t modes;
    dc1394framerates_t framerates;

    dc1394_t * dc1394 = 0;
    dc1394camera_list_t *cameras = 0;
    dc1394camera_t *camera = 0;

    // make sure raw1394 is loaded and read/writeable
    raw1394handle_t tmpHandle = raw1394_new_handle();
    if (tmpHandle == NULL) 
    {
        // if module is present but permissions aren't good, print a warning
        // otherwise don't do anything
        if (boost::filesystem::exists("/dev/raw1394"))
        {
            if (boost::filesystem::exists("/dev/video1394"))
            {
                if (not isModuleReadable("raw1394"))
                    LOG_WARNING("Module raw1394 is not readable");
                if (not isModuleWriteable("raw1394"))
                    LOG_WARNING("Module raw1394 is not writeable");
                if (not isModuleReadable("video1394"))
                    LOG_WARNING("Module video1394 is not readable");
            }
            else
                LOG_WARNING("Module video1394 is not loaded.");
        }
        else if (boost::filesystem::exists("/dev/video1394"))
            LOG_WARNING("Module raw1394 is not loaded.");
        else
            ;// do nothing, neither module is loaded, we can assume no firewire utilization

        return;
    }
    else
        raw1394_destroy_handle(tmpHandle);

    dc1394 = dc1394_new();

    camerr = dc1394_camera_enumerate(dc1394, &cameras);

    if (camerr != DC1394_SUCCESS or cameras == 0)
    {
        LOG_ERROR("Can't find cameras error : " << camerr);
        cleanup(dc1394, camera, cameras);
        return;
    }

    if (cameras->num == 0) 
    {
        LOG_INFO("There were no dc1394 cameras");
        cleanup(dc1394, camera, cameras);
        return;
    }

    for (unsigned srcCamNum = 0; srcCamNum < cameras->num; ++srcCamNum)
    {
        camera =
            dc1394_camera_new_unit (dc1394, cameras->ids[srcCamNum].guid,
                    cameras->ids[srcCamNum].unit);
        LOG_PRINT("\nDC1394 Camera " << srcCamNum << std::endl);
        LOG_PRINT("GUID = " << std::hex << cameras->ids[srcCamNum].guid << std::endl);

        camerr = dc1394_video_get_supported_modes(camera, &modes);
        if (camerr != DC1394_SUCCESS) 
        {
            LOG_ERROR("Error getting supported modes\n");
            cleanup(dc1394, camera, cameras);
            return;
        }

        LOG_PRINT("Supported modes :\n");
        for (int i = modes.num - 1; i >= 0; --i)
        {
            int m = modes.modes[i];

            if (m < DC1394_VIDEO_MODE_EXIF) 
            {
                if (dc1394_caps_print_format_vmode_caps(m) < 0)
                {
                    LOG_ERROR("attempt to query mode " << m << " failed");
                    cleanup(dc1394, camera, cameras);
                    return;
                }

                camerr = dc1394_video_get_supported_framerates(camera,
                        (dc1394video_mode_t) m, &framerates);
                if (camerr != DC1394_SUCCESS)
                {
                    LOG_ERROR("dc1394 error : " << camerr);
                    cleanup(dc1394, camera, cameras);
                    return;
                }
                printSupportedFramerates(framerates);
            }
        }

        dc1394_camera_free(camera);
        camera = 0;
    }

    cleanup(dc1394, camera, cameras);
}


int DC1394::GUIDToCameraNumber(unsigned long long GUID)
{
    int result = -1;
    dc1394error_t camerr;

    dc1394_t * dc1394 = 0;
    dc1394camera_list_t *cameras = 0;
    dc1394camera_t *camera = 0;

    dc1394 = dc1394_new();

    camerr = dc1394_camera_enumerate(dc1394, &cameras);

    if (camerr != DC1394_SUCCESS or cameras == 0)
    {
        LOG_ERROR("Can't find cameras error : " << camerr);
        cleanup(dc1394, camera, cameras);
        return -1;
    }

    if (cameras->num == 0) 
    {
        LOG_INFO("There were no dc1394 cameras");
        cleanup(dc1394, camera, cameras);
        return -1;
    }

    bool found = false;

    for (unsigned srcCamNum = 0; !found && srcCamNum < cameras->num; ++srcCamNum)
    {
        camera =
            dc1394_camera_new_unit (dc1394, cameras->ids[srcCamNum].guid,
                    cameras->ids[srcCamNum].unit);
        LOG_DEBUG("GUID = " << std::hex << cameras->ids[srcCamNum].guid);
        if (GUID == cameras->ids[srcCamNum].guid)
        {
            result = srcCamNum;
            found = true;
        }
        dc1394_camera_free(camera);
        camera = 0;
    }

    cleanup(dc1394, camera, cameras);
    if (result == -1)
        LOG_WARNING("Could not find camera with guid " << GUID);

    return result;
}
