
// videoSender.h
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


#ifndef _VIDEO_SENDER_H_
#define _VIDEO_SENDER_H_

#include <string>
#include "defaultAddresses.h"

#include "mediaBase.h"

class VideoSender : public MediaBase
{
    public:
        VideoSender();
        virtual ~VideoSender(); 
        bool init(const std::string media = "test",
                  const int port = DEF_PORT, 
                  const std::string addr = THEIR_ADDRESS );
        virtual bool start();

    private:
        void initDv();
        void initDvRtp();
        void initV4l();
        void initV4lRtp();
        void initTest();

        std::string remoteHost_;
};

#endif

