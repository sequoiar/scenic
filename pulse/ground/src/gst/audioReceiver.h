
// audioReceiver.h
// Copyright 2008 Koya Charles & Tristan Matthews 
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

#ifndef _AUDIO_RECEIVER_H_
#define _AUDIO_RECEIVER_H_

#include <string>

#include "lo/lo.h"

#include "mediaBase.h"
#include "audioConfig.h"
#include "rtpSession.h"

class AudioReceiver:public MediaBase
{
  public:
    AudioReceiver();
    AudioReceiver(const AudioConfig & config);
      virtual ~ AudioReceiver();
    virtual bool start();

  private:
      virtual void init_source();
    virtual void init_codec();
    virtual void init_sink();

    static int caps_handler(const char *path, const char *types, lo_arg ** argv, int argc,
                            void *data, void *user_data);

    void set_caps(const char *caps);
    static void liblo_error(int num, const char *msg, const char *path);

    void wait_for_caps();

    RtpReceiver session_;
    GstElement *depayloader_, *decoder_, *sink_;
    AudioConfig config_;
    bool gotCaps_;
};

#endif // _AUDIO_RECEIVER_H_
