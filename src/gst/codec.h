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
#include "logWriter.h"

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


class Encoder : public Codec
{
    public:
        virtual RtpPay* createPayloader() const = 0;
};


class Decoder : public Codec
{
    public:
        virtual RtpPay* createDepayloader() const = 0;
        virtual void setSrcCaps(){ LOG_DEBUG("BASE DECODER");};
};


class H264Encoder : public Encoder
{
    public: 

        H264Encoder() : colorspc_(0) {};
        ~H264Encoder();
        void init();
        RtpPay* createPayloader() const;

    private:

        _GstElement *sinkElement() { return colorspc_; }
        _GstElement *colorspc_;
        
        H264Encoder(const H264Encoder&);     //No Copy Constructor
        H264Encoder& operator=(const H264Encoder&);     //No Assignment Operator
};


class H264Decoder : public Decoder
{
    public: 

        void init();
        RtpPay* createDepayloader() const;
};


class VorbisEncoder : public Encoder 
{
    public: 

        VorbisEncoder() : aconv_(0){};
        ~VorbisEncoder();
        void init();
        RtpPay* createPayloader() const;

    private:
        _GstElement *sinkElement() { return aconv_; }
        _GstElement *aconv_;
        VorbisEncoder(const VorbisEncoder&);     //No Copy Constructor
        VorbisEncoder& operator=(const VorbisEncoder&);     //No Assignment Operator
};


class VorbisDecoder : public Decoder
{
    public: 

        void init();
//        void setSrcCaps();
        RtpPay* createDepayloader() const;
};


// FIXME: DRY!!!!
class RawEncoder : public Encoder 
{
    public: 
        RawEncoder() : aconv_(0) {};
        ~RawEncoder();
        void init();
        RtpPay* createPayloader() const;
    
    private:

        _GstElement *srcElement() { return aconv_; }
        _GstElement *sinkElement() { return aconv_; }
        _GstElement *aconv_;
        
        RawEncoder(const RawEncoder&);     //No Copy Constructor
        RawEncoder& operator=(const RawEncoder&);     //No Assignment Operator
};


class RawDecoder : public Decoder
{
    public: 

        RawDecoder() : aconv_(0) {};
        ~RawDecoder();
        void init();
        RtpPay* createDepayloader() const;

    private:

        _GstElement *srcElement() { return aconv_; }
        _GstElement *sinkElement() { return aconv_; }
        _GstElement *aconv_;
        
        RawDecoder(const RawDecoder&);     //No Copy Constructor
        RawDecoder& operator=(const RawDecoder&);     //No Assignment Operator
};


class LameEncoder : public Encoder 
{
    public: 
        LameEncoder() : aconv_(0) {};
        ~LameEncoder();
        void init();
        RtpPay* createPayloader() const;
    
    private:

        _GstElement *sinkElement() { return aconv_; }
        _GstElement *aconv_;
        
        LameEncoder(const LameEncoder&);     //No Copy Constructor
        LameEncoder& operator=(const LameEncoder&);     //No Assignment Operator
};


class MadDecoder : public Decoder
{
    public: 

        MadDecoder() : aconv_(0) {};
        ~MadDecoder();
        void init();
        RtpPay* createDepayloader() const;

    private:

        _GstElement *srcElement() { return aconv_; }
        _GstElement *aconv_;
        
        MadDecoder(const MadDecoder&);     //No Copy Constructor
        MadDecoder& operator=(const MadDecoder&);     //No Assignment Operator
};

#endif //_CODEC_H_

