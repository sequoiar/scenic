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

#include <glib/gfileutils.h>
#include "dc1394.h"
#include "noncopyable.h"
#include "util/logWriter.h"

#include <fstream>
#include <dc1394/control.h>
#include <libraw1394/raw1394.h>
#include <iomanip>

class Dc1394Handle : private boost::noncopyable {
    public:
        // camera-less version
        Dc1394Handle() : cameraId_(0), dc1394_(0), cameras_(0), camera_(0)
        {
            // override log handler to catch library errors
            dc1394log_t type = DC1394_LOG_ERROR;
            dc1394_log_register_handler(type, log_handler, 0);

            dc1394_ = dc1394_new();
            if (dc1394_ == 0)
                LOG_ERROR("Could not get handle to dc1394, are /dev/raw1394 and /dev/video1394 present?");
            dc1394error_t camerr = dc1394_camera_enumerate(dc1394_, &cameras_);
            if (camerr != DC1394_SUCCESS or cameras_ == 0)
                LOG_ERROR("Can't find cameras error : " << camerr);
        }

        explicit Dc1394Handle(int id) : cameraId_(id), dc1394_(0), cameras_(0), camera_(0)
        {
            // override log handler to catch library errors
            dc1394log_t type = DC1394_LOG_ERROR;
            dc1394_log_register_handler(type, log_handler, 0);

            if (cameraId_ < 0)
                LOG_ERROR("Invalid camera id " << cameraId_);
            dc1394_ = dc1394_new();
            if (dc1394_ == 0)
                LOG_ERROR("Could not get handle to dc1394, are /dev/raw1394 and /dev/video1394 present?");
            dc1394error_t camerr = dc1394_camera_enumerate(dc1394_, &cameras_);
            if (camerr != DC1394_SUCCESS or cameras_ == 0)
                LOG_ERROR("Can't find cameras error : " << camerr);

            if (cameras_->num > 0)
            {
                camera_ = dc1394_camera_new_unit(dc1394_, cameras_->ids[cameraId_].guid, cameras_->ids[cameraId_].unit);
                if (camera_ == 0)
                    LOG_ERROR("Could not get handle to dc1394 camera");
            }
        }

        ~Dc1394Handle() 
        {
            LOG_DEBUG("Destroying dc1394handle");
            if (camera_ != 0) 
                dc1394_camera_free(camera_);
            if (cameras_ != 0)
                dc1394_camera_free_list(cameras_);
            if (dc1394_ != 0)
                dc1394_free(dc1394_);
        }

        unsigned long long guid() const
        {
            if (cameraId_ >= 0)
                return cameras_->ids[cameraId_].guid;
            else
            {
                LOG_ERROR("Cannot get guid for invalid camera id " << cameraId_);
                return 0;
            }
        }

        int nCameras() const
        {
            if (cameras_ == 0)
                LOG_ERROR("Cannot query number of cameras");
            return cameras_->num;
        }

        void printInfo() const;

        int capsToMode(int width, int height, 
                const std::string &colourspace, int framerate) const;

    private:
        static void log_handler(dc1394log_t /*type*/, const char *message, void * /*user*/)
        {
            LOG_DEBUG(message);
        }

        int cameraId_;
        dc1394_t * dc1394_; 
        dc1394camera_list_t * cameras_;
        dc1394camera_t * camera_; 
};


int dc1394_caps_print_format_vmode_caps(int mode)
{
    int retval = 0;

    switch (mode) 
    {
#define PRINT_CASE(x) \
        case (DC1394_VIDEO_MODE_ ##x): \
                                       LOG_PRINT("    " << # x << " (vmode " << mode << ")" << std::endl); \
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
                for (unsigned framerateIdx = 0; 
                        not frameratesMatch and framerateIdx < framerates.num;
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
    if (not frameratesMatch)
        LOG_WARNING("So far only 15 fps and 30 fps are supported");
    return frameratesMatch;
}


/// FIXME: have all the modes here and this will also depend on framerate
bool Dc1394::requiresMoreISOSpeed(int mode)
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

int Dc1394::capsToMode(int cameraId, int width, int height, 
        const std::string &colourspace, int framerate)
{
    Dc1394Handle dc(cameraId);
    return dc.capsToMode(width, height, colourspace, framerate);
}

/// FIXME: replace with table-driven method
int Dc1394Handle::capsToMode(int width, int height, 
        const std::string &colourspace, int framerate) const
{
    int mode = 0;
    dc1394video_modes_t modes;
    dc1394error_t camerr;

    camerr = dc1394_video_get_supported_modes(camera_, &modes);
    if (camerr != DC1394_SUCCESS) 
        LOG_ERROR("Error getting supported modes\n");

#define RETURN_MODE_FROM_CAPS(WIDTH, HEIGHT, COLOURSPACE)  \
    (width == (WIDTH) and height == (HEIGHT))  \
    {                                             \
        int testMode = DC1394_VIDEO_MODE_ ## WIDTH ##x ## HEIGHT ##_ ## COLOURSPACE; \
        if (modeIsSupported(testMode, modes, framerate, camera_))    \
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
        if RETURN_MODE_FROM_CAPS(640, 480, YUV422);
        if RETURN_MODE_FROM_CAPS(640, 480, YUV411);
        if RETURN_MODE_FROM_CAPS(160, 120, YUV444);
        if RETURN_MODE_FROM_CAPS(320, 240, YUV422);
        if RETURN_MODE_FROM_CAPS(800, 600, YUV422);
        if RETURN_MODE_FROM_CAPS(1024, 768, YUV422);
        if RETURN_MODE_FROM_CAPS(1280, 960, YUV422);
        if RETURN_MODE_FROM_CAPS(1600, 1200, YUV422);
        LOG_DEBUG("Colourspace " << colourspace << " and resolution "
                << width << "x" << height << " are not supported by this camera");
    }
    else if (colourspace == "gray")
    {
        // not else if because resolutions are the same
        if RETURN_MODE_FROM_CAPS(640, 480, MONO8);
        if RETURN_MODE_FROM_CAPS(640, 480, MONO16);
        if RETURN_MODE_FROM_CAPS(800, 600, MONO8);
        if RETURN_MODE_FROM_CAPS(800, 600, MONO16);
        if RETURN_MODE_FROM_CAPS(1024, 768, MONO8);
        if RETURN_MODE_FROM_CAPS(1024, 768, MONO16);
        if RETURN_MODE_FROM_CAPS(1280, 960, MONO8);
        if RETURN_MODE_FROM_CAPS(1600, 1200, MONO8);
        if RETURN_MODE_FROM_CAPS(1280, 960, MONO16);
        // FIXME:
        // This could be supported but won't work unless you use
        // dc1394_video_set_operation_mode(camera->camera_info, DC1394_OPERATION_MODE_1394B);
        // and then increase the ISO speed to 800
        // dc1394_video_set_iso_speed(camera->camera_info,DC1394_ISO_SPEED_800);
        if RETURN_MODE_FROM_CAPS(1600, 1200, MONO16);
        LOG_WARNING("Colourspace " << colourspace << " and resolution " <<
                width << "x" << height << " are not supported by this camera");
    }
    else if (colourspace == "rgb")
    {
        if RETURN_MODE_FROM_CAPS(640, 480, RGB8);
        if RETURN_MODE_FROM_CAPS(800, 600, RGB8);
        if RETURN_MODE_FROM_CAPS(1024, 768, RGB8);
        if RETURN_MODE_FROM_CAPS(1280, 960, RGB8);
        if RETURN_MODE_FROM_CAPS(1600, 1200, RGB8);
        LOG_WARNING("Colourspace " << colourspace << " and resolution " << 
                width << "x" << height << " are not supported by this camera");
    }
    else
        THROW_ERROR("Invalid colourspace " << colourspace);

#undef RETURN_MODE_FROM_CAPS
    return 0;   /// never gets called
}


void printSupportedFramerates(dc1394framerates_t framerates)
{
    LOG_PRINT("    Framerates: ");
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


/// Returns true if cameras were found
bool Dc1394::listCameras()
{
    LOG_DEBUG("listing cameras");

    // make sure raw1394 is loaded and read/writeable
    raw1394handle_t tmpHandle = raw1394_new_handle();
    if (tmpHandle == NULL) 
    {
        // if module is present but permissions aren't good, print a warning
        // otherwise don't do anything
        if (g_file_test("/dev/raw1394", G_FILE_TEST_EXISTS))
        {
            if (g_file_test("/dev/video1394", G_FILE_TEST_EXISTS))
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
        else if (g_file_test("/dev/video1394", G_FILE_TEST_EXISTS))
            LOG_WARNING("Module raw1394 is not loaded.");
        else
            LOG_DEBUG("Neither raw1394 nor video1394 modules are loaded");
        // do nothing, neither module is loaded, we can assume no firewire utilization
        return false;
    }
    else
        raw1394_destroy_handle(tmpHandle);
    
    int nCameras = Dc1394::nCameras();

    for (int i = 0; i != nCameras; ++i)
    {
        Dc1394Handle dc(i);
        dc.printInfo();
    }
    return nCameras > 0;
}

void Dc1394Handle::printInfo() const
{
    if (camera_ != 0)
    {
        dc1394video_modes_t modes;
        dc1394framerates_t framerates;
        LOG_PRINT("\nDC1394 Camera " << cameraId_ << ": " 
                << camera_->vendor << " " << camera_->model << std::endl);
        LOG_PRINT("GUID = " << std::hex << guid() << std::endl);
        dc1394error_t camerr = dc1394_video_get_supported_modes(camera_, &modes);

        if (camerr != DC1394_SUCCESS) 
            LOG_ERROR("Error getting supported modes\n");

        LOG_PRINT("Supported modes :\n");
        for (int i = modes.num - 1; i >= 0; --i)
        {
            int m = modes.modes[i];

            if (m < DC1394_VIDEO_MODE_EXIF) 
            {
                if (dc1394_caps_print_format_vmode_caps(m) < 0)
                    LOG_ERROR("attempt to query mode " << m << " failed");

                camerr = dc1394_video_get_supported_framerates(camera_,
                        (dc1394video_mode_t) m, &framerates);
                if (camerr != DC1394_SUCCESS)
                    LOG_ERROR("dc1394 error : " << camerr);
                printSupportedFramerates(framerates);
            }
        }
    }
}

int Dc1394::nCameras()
{
    Dc1394Handle dc;
    return dc.nCameras();
}

bool Dc1394::areCamerasConnected()
{
    return nCameras() != 0;
}


int Dc1394::GUIDToCameraNumber(unsigned long long GUID)
{
    int result = -1;
    bool found = false;
    int nCameras = Dc1394::nCameras();

    for (int i = 0; not found && i != nCameras; ++i)
    {
        Dc1394Handle dc(i);
        LOG_DEBUG("GUID = " << std::hex << dc.guid());

        if (GUID == dc.guid())
        {
            result = i;
            found = true;
        }
    }

    if (result == -1)
        THROW_ERROR("Could not find camera with guid " << std::hex << GUID);

    return result;
}
