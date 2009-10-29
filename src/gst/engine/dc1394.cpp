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

#include <dc1394/control.h>
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


int dc1394_caps_get_format_vmode_caps(int mode)
{
  int retval = 0;

  switch (mode) 
  {
#define PRINT_CASE(x) \
            case (DC1394_VIDEO_MODE_ ##x): \
                      LOG_PRINT(# x << std::endl); \
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
        retval = -1;
  }
  return retval;
#undef PRINT_CASE
}


void printSupportedFramerates(dc1394framerates_t framerates)
{
    LOG_PRINT("Supported framerates: ");
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
        LOG_INFO("There were no cameras");
        cleanup(dc1394, camera, cameras);
        return;
    }

    for (unsigned srcCamNum = 0; srcCamNum < cameras->num; ++srcCamNum)
    {
        camera =
            dc1394_camera_new_unit (dc1394, cameras->ids[srcCamNum].guid,
                    cameras->ids[srcCamNum].unit);
        LOG_PRINT("\n");
        LOG_PRINT("Camera " << srcCamNum << "'s guid is " << std::hex << cameras->ids[srcCamNum].guid << std::endl);

        camerr = dc1394_video_get_supported_modes(camera, &modes);
        if (camerr != DC1394_SUCCESS) 
        {
            LOG_ERROR("Error getting supported modes\n");
            cleanup(dc1394, camera, cameras);
            return;
        }

        LOG_PRINT("Modes supported by camera " << srcCamNum << ":\n");
        for (int i = modes.num - 1; i >= 0; --i)
        {
            int m = modes.modes[i];

            if (m < DC1394_VIDEO_MODE_EXIF) 
            {
                if (dc1394_caps_get_format_vmode_caps(m) < 0)
                {
                    LOG_ERROR("attempt to query mode " << m << " failed");
                    cleanup(dc1394, camera, cameras);
                    return;
                }

                camerr = dc1394_video_get_supported_framerates(camera,(dc1394video_mode_t)m, &framerates);
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

