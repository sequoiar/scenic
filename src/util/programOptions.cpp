/* programOptions.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "programOptions.h"
#include "videoSize.h"

namespace po = boost::program_options;

po::options_description ProgramOptions::createDefaultOptions()
{
    using std::string;
    po::options_description desc;

    // TODO: maybe add groupings (audioreceiver, audiosender, videoreceiver, videosender)
    desc.add_options()
        ("help,h", "produce help")
        ("receiver,r", po::bool_switch(), "this process is a receiver")
        ("sender,s", po::bool_switch(), "this process is a sender")
        ("address,i", po::value<string>()->default_value("127.0.0.1"), "provide ip address of remote host")
        ("videocodec,v", po::value<string>()->default_value("mpeg4"), "videocodec (mpeg4,h263,h264,theora)")
        ("audiocodec,a", po::value<string>()->default_value("raw"), "audiocodec (raw,vorbis,mp3)")
        ("videosink,k", po::value<string>()->default_value("xvimagesink"), "video output "
         "(xvimagesink,ximagesink,glimagesink,sharedvideosink)")
        ("audiosink,l", po::value<string>()->default_value("jackaudiosink"), "audio output (jackaudiosink,alsasink,pulsesink)")
        ("audioport,t", po::value<int>(), "audioport number (1024-65535")
        ("videoport,p", po::value<int>(), "videoport number (1024-65535)")
        ("fullscreen,f", po::bool_switch(), "display video in fullscreen")
        ("shared-video-id,B", po::value<string>()->default_value("shared_memory"), "name for shared video buffer output")
        ("deinterlace,o", po::bool_switch(), "deinterlace video on reception")
        ("videodevice,d", po::value<string>()->default_value("/dev/video0"), "name of video device (/dev/video0,/dev/video1)")
        ("videodisplay", po::value<string>(), "display to use for video output (:0.0,:0.1)")
        ("audiodevice,q", po::value<string>()->default_value(""), "name of audio device (hw:0,hw:2,plughw:0,plughw:2)")
        ("videolocation,L", po::value<string>()->default_value(""), "video filename")
        ("audiolocation,K", po::value<string>()->default_value(""), "audio filename")
        ("version,w", po::bool_switch(), "display version info")
        ("numchannels,c", po::value<int>()->default_value(2), "number of audio channels")
        ("videobitrate,x", po::value<int>()->default_value(3000000), "video bitrate (1000000,3000000)")
        ("videoquality,X", po::value<int>()->default_value(32), 
         "use specified video quality instead of bitrate for theora (0-63)")
        ("audioquality", po::value<double>()->default_value(-1.0), "quality for compressed audio (0.0-1.0)")
        ("audiobitrate", po::value<int>()->default_value(0), "bitrate for compressed audio in kbps "
         "(7,16,24,32,40,48,56,64,80,96,112,128,160,192,224,256,320)")
        ("audiosource,e", po::value<string>()->default_value("jackaudiosrc"), 
         "audio input (jackaudiosrc,alsasrc,pulsesrc)")
        ("videosource,u", po::value<string>()->default_value("videotestsrc"), 
         "video input (v4l2src,dc1394src,v4lsrc,dv1394src)")
        ("timeout,z", po::value<int>()->default_value(0), "time in ms to wait before quitting, 0=run indefinitely")
        ("audio-buffer,b", po::value<int>()->default_value(15), "size of audio buffer in milliseconds, must be >= 10")
        ("jitterbuffer,g", po::value<int>()->default_value(50), "size of receiver's rtp jitterbuffers in milliseconds, must be > 1")
        ("camera-number,G", po::value<int>()->default_value(-1), "camera id for dc1394src")
        ("camera-guid,U", po::value<std::string>()->default_value("0"), "camera guid for dc1394src")
        ("multicast-interface,I", po::value<string>()->default_value(""), "interface to use for multicast (eth0,eth1)")
        ("enable-controls,j", po::bool_switch(), "enable gui controls for adjusting the jitterbuffer")
        ("disable-jack-autoconnect,J", po::bool_switch(), "make sure milhouse's "
         "jack audio ports don't connect on startup")
        ("jack-client-name,O", po::value<string>()->default_value(""), "name of jack-client")
        ("debug,D", po::value<string>()->default_value("info"), "level of logging verbosity (string/int) "
         "(critical=1,error=2,warning=3,info=4,debug=5,gst-debug=6)")
        ("window-title,W", po::value<string>()->default_value("Milhouse"), "title for video window")
        ("framerate,F", po::value<int>()->default_value(30), "framerate for video (15,30)")
        ("list-cameras,H", po::bool_switch(), "list connected cameras")
        ("list-v4l2", po::bool_switch(), "list connected V4L2 devices")
        ("width,N", po::value<int>()->default_value(videosize::WIDTH), "width for video capture")
        ("height,Y", po::value<int>()->default_value(videosize::HEIGHT), "height for video capture")
        ("display-width,P", po::value<int>(), "width for video on display"
         "(scales output video width)")
        ("display-height,Q", po::value<int>(), "height for video "
         "(scales output video height)")
        ("grayscale,M", po::bool_switch(), "force dc1394 capture to grayscale")
        ("aspect-ratio,A", po::value<string>()->default_value("4:3"), "picture aspect ratio (4:3,16:9)")
        ("localvideo", po::bool_switch(), "display local video only")
        ("localaudio", po::bool_switch(), "local audio only")
        ("flip-video", po::value<string>()->default_value("none"), "flip video (none, clockwise, rotate-180, "
         "counterclockwise, horizontal-flip, vertical-flip, upper-left-diagonal, upper-right-diagonal)")
        ("v4l2-standard", po::value<string>(), "set v4l2 standard (NTSC,PAL)")
        ("v4l2-input", po::value<int>(), "set v4l2 input (0,1,2)")
        ("x-window-id", po::value<unsigned long>()->default_value(0), "set x-window-id to display video in an existing window")
        ("vumeter-id", po::value<unsigned long>()->default_value(0), "id of the socket where the vumeters will be displayed")
        ("max-channels", po::bool_switch(), "print the maximum number of "
         "channels milhouse can send for the selected audiocodec")
        ("dump-pipeline", po::bool_switch(), "outputs a dotfile representation of this process' pipeline")
        ("text-overlay", po::value<string>(), "overlays given text on video sink (receiver side only)")
        ("gst-version", po::bool_switch(), "prints the version number of the GStreamer (core) library currently installed")
        ("rtsp-server", po::bool_switch(), "run in RTSP server mode")
        ("rtsp-client", po::bool_switch(), "run in RTSP client mode")
        ;

    return desc;
}

