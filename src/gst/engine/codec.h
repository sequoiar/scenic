// codec.h
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

#ifndef _CODEC_H_
#define _CODEC_H_

#include "gstLinkable.h"

// forward declarations
class _GstElement;
class RtpPay;


/** 
 *  Abstract base class that wraps a single GstElement, and which exposes both a source and sink.
 */
class Codec : public GstLinkableFilter
{
    public:
        Codec();
        ~Codec();
        virtual void init() = 0;

    protected:
        _GstElement *codec_;

    private:
        _GstElement *srcElement() { return codec_; }
        _GstElement *sinkElement() { return codec_; }

        /// No Copy Constructor
        Codec(const Codec&);     
        /// No Assignment Operator
        Codec& operator=(const Codec&);     
};

/** 
 *  Abstract child of Codec that wraps a single GstElement, and which exposes both a source and sink 
 *  and whose concrete subclasses will provide specifc encoding of raw media streams.
 */
class Encoder : public Codec
{
    public:
        /// Abstract Factory method that will create payloaders corresponding to this Encoder's codec type 
        virtual RtpPay* createPayloader() const = 0;
        int getBitrate();
        void postBitrate();
        virtual void setBitrate(unsigned bitrate);
};

/** 
 *  Abstract child of Codec that wraps a single GstElement, and which exposes both a source and sink 
 *  and whose concrete subclasses will provide specifc decoding of encoded media streams.
 */
class Decoder : public Codec
{
    public:
        /// Abstract Factory method that will create depayloaders corresponding to this Decoder's codec type 
        virtual RtpPay* createDepayloader() const = 0;
        virtual void adjustJitterBuffer() {}; // buy default, do nothing
        virtual bool adjustsBufferTime() { return false; }
        virtual unsigned long long minimumBufferTime() { THROW_ERROR("Unimplemented"); return 0; }
};

/// Abstract child of encoder that wraps audioconvert functionality

class AudioConvertedEncoder : public Encoder
{
    protected:
        AudioConvertedEncoder();
        ~AudioConvertedEncoder();
        _GstElement *aconv_;
        void init();

    private:
        _GstElement *sinkElement() { return aconv_; }
    
        /// No Copy Constructor 
        AudioConvertedEncoder(const AudioConvertedEncoder&);     
        ///No Assignment Operator
        AudioConvertedEncoder& operator=(const AudioConvertedEncoder&);     
};


class AudioConvertedDecoder : public Decoder
{
    protected: 
        AudioConvertedDecoder();
        ~AudioConvertedDecoder();
        _GstElement *aconv_;
        void init();

    private:
        _GstElement *srcElement() { return aconv_; }

        ///No Copy Constructor
        AudioConvertedDecoder(const AudioConvertedDecoder&);     
        ///No Assignment Operator
        AudioConvertedDecoder & operator=(const AudioConvertedDecoder&);     
};

class VideoEncoder : public Encoder 
{
    public: 
        VideoEncoder();
        ~VideoEncoder();
        void doDeinterlace() { doDeinterlace_ = true; }
        virtual void init() = 0;

    protected:
        bool doDeinterlace_;
        _GstElement *colorspc_;
        _GstElement *sinkQueue_;
        _GstElement *srcQueue_;
        _GstElement *deinterlace_;
        bool supportsInterlaced_;

    private:
        
        _GstElement *sinkElement() 
        { 
            return sinkQueue_;
        }
        _GstElement *srcElement() 
        { 
            return srcQueue_;
        }

        /// No Copy Constructor
        VideoEncoder(const VideoEncoder&);     
        /// No Assignment Operator
        VideoEncoder& operator=(const VideoEncoder&);     
};

/// Encoder that encodes raw video into H.264 using the x264 encoder
class H264Encoder : public VideoEncoder
{
    public: 
        H264Encoder();
        void setBitrate(unsigned);

    private:
        ~H264Encoder();
        void init();
        RtpPay* createPayloader() const;
};

/// Decoder that decodes H.264 into raw video using the ffdec_h264 decoder.
class H264Decoder : public Decoder
{
    private: 
        void init();
        RtpPay* createDepayloader() const;
        void adjustJitterBuffer(); 
        static const unsigned long long DEFAULT_JITTER_BUFFER_MS = 30;
};



/// Encoder that encodes raw video into H.263 using the ffmpeg h263 encoder
class H263Encoder : public VideoEncoder
{
    public: 
        H263Encoder();

    private:
        ~H263Encoder();

        void init();
        
        RtpPay* createPayloader() const;
};

/// Decoder that decodes H.263 into raw video using the ffmpeg hq263 decoder.
class H263Decoder : public Decoder
{
    private: 
        void init();
        RtpPay* createDepayloader() const;
};



/// Encoder that encodes raw video into mpeg4 using the ffmpeg mpeg4 encoder
class Mpeg4Encoder : public VideoEncoder
{
    public:
        Mpeg4Encoder();
        ~Mpeg4Encoder();

    private:
        void init();
        RtpPay* createPayloader() const;
};


/// Decoder that decodes mpeg4 into raw video using the ffmpeg mpeg4 decoder.
class Mpeg4Decoder: public Decoder
{
    private: 
        void init();
        RtpPay* createDepayloader() const;
};


/// Encoder that encodes raw video into mpeg4 using the ffmpeg mpeg4 encoder
class TheoraEncoder : public VideoEncoder
{
    public:
        TheoraEncoder();
        ~TheoraEncoder();
        void setBitrate(unsigned);

    private:
        static const int MAX_SPEED_LEVEL = 2;
        static const int MIN_SPEED_LEVEL = 0;
        static const int QUALITY = 20;
        void init();
        RtpPay* createPayloader() const;
};


/// Decoder that decodes mpeg4 into raw video using the ffmpeg mpeg4 decoder.
class TheoraDecoder: public Decoder
{
    private: 
        void init();
        RtpPay* createDepayloader() const;
};


/// Encoder that encodes raw audio using the vorbis encoder.
class VorbisEncoder : public AudioConvertedEncoder 
{
    public: 
        VorbisEncoder();

    private:
        _GstElement *srcQueue_;
        _GstElement *sinkQueue_;
        ~VorbisEncoder();
        void init();
        RtpPay* createPayloader() const;

        _GstElement* srcElement() { return srcQueue_; }

        /// No Copy Constructor 
        VorbisEncoder(const VorbisEncoder&);     
        ///No Assignment Operator
        VorbisEncoder& operator=(const VorbisEncoder&);     
};

/// Decoder that decodes vorbis into raw audio using the vorbis decoder.
class VorbisDecoder : public Decoder
{
    public: 
        bool adjustsBufferTime() { return true; }
        unsigned long long minimumBufferTime();
    private: 
        void init();
        RtpPay* createDepayloader() const;
        static const unsigned long long MIN_BUFFER_USEC = 100000;
};

/// Encoder that simply performs datatype conversion on raw audio.
class RawEncoder : public AudioConvertedEncoder 
{
    public:
        RawEncoder();
        _GstElement *srcElement() { return aconv_; }

    private:
        void init();
        RtpPay* createPayloader() const;
    
        /// No Copy Constructor 
        RawEncoder(const RawEncoder&);     
        ///No Assignment Operator
        RawEncoder& operator=(const RawEncoder&);     
};

/// Decoder that simply performs datatype conversion on raw audio.
class RawDecoder : public AudioConvertedDecoder
{
    public:
        RawDecoder();

    private:
        RtpPay* createDepayloader() const;

        _GstElement *sinkElement() { return aconv_; }
        
        ///No Copy Constructor
        RawDecoder(const RawDecoder&);     
        ///No Assignment Operator
        RawDecoder& operator=(const RawDecoder&);     
};


/// Encoder that encodes raw audio to mpeg.
class LameEncoder : public AudioConvertedEncoder 
{
    public:
        LameEncoder();

    private:
        void init();
        void checkNumChannels();
        RtpPay* createPayloader() const;
};

/// Decoder that decodes mpeg to raw audio.

class MadDecoder : public AudioConvertedDecoder
{
    public:
        MadDecoder();
    private:
        void init();
        RtpPay* createDepayloader() const;
};

#endif //_CODEC_H_

