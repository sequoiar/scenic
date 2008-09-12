// codec.h
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

#ifndef _CODEC_H_
#define _CODEC_H_

#include "gstLinkable.h"

class _GstElement;
class RtpPay;

class Codec : public GstLinkableFilter
{
    public:

        Codec()
            : codec_(0) {};
        ~Codec();

    protected:

        _GstElement *codec_;

    private:

        _GstElement *srcElement() { return codec_; }
        _GstElement *sinkElement() { return codec_; }

        Codec(const Codec&);     //No Copy Constructor
        Codec& operator=(const Codec&);     //No Assignment Operator
};

class H264Encoder : public Codec
{
    public: 

        H264Encoder() : colorspc_(0) {};
        ~H264Encoder();
        bool init();
        RtpPay* createPayloader() const;

    private:

        _GstElement *sinkElement() { return colorspc_; }
        _GstElement *colorspc_;
        
        H264Encoder(const H264Encoder&);     //No Copy Constructor
        H264Encoder& operator=(const H264Encoder&);     //No Assignment Operator
};


class H264Decoder : public Codec
{
    public: 

        bool init();
        RtpPay* createDepayloader() const;
};


class VorbisEncoder : public Codec
{
    public: 

        bool init();
        RtpPay* createPayloader() const;
};


class VorbisDecoder : public Codec
{
    public: 

        bool init();
        RtpPay* createDepayloader() const;
};


#endif //_CODEC_H_

