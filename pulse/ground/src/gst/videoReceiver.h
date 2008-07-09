
// videoReceiver.h
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

#ifndef _VIDEO_RECEIVER_H_
#define _VIDEO_RECEIVER_H_

#include "mediaBase.h"
#include "videoConfig.h"
#include "rtpReceiver.h"

class VideoReceiver : public MediaBase
{
public:
	VideoReceiver(const VideoConfig & config);
	virtual bool start();
	virtual bool stop();
	virtual ~VideoReceiver();

private:
	void set_caps(const char* capsStr);
	virtual void init_source();
	virtual void init_codec();
	virtual void init_sink();

	RtpReceiver session_;
	const VideoConfig &config_;
	GstElement *depayloader_, *decoder_, *sink_;
};

#endif
