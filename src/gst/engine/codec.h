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
        static bool isSupportedCodec(const std::string &codecStr);

    protected:
        _GstElement *codec_;

    private:
        static const int NUM_CODECS = 6;
        static const std::string VALID_CODECS[NUM_CODECS];
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
        void setBitrate(unsigned bitrate);
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
        virtual const char * getCaps() const { return "";}
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


/// Encoder that encodes raw video into H.264 using the x264 encoder
class H264Encoder : public Encoder
{
    public: 
        H264Encoder();
        void setBitrate(unsigned);

    private:
        ~H264Encoder();

        void init();
        
        RtpPay* createPayloader() const;

        _GstElement *colorspc_;
        _GstElement *sinkElement() { return colorspc_; }

        /// No Copy Constructor
        H264Encoder(const H264Encoder&);     
        /// No Assignment Operator
        H264Encoder& operator=(const H264Encoder&);     
};

/// Decoder that decodes H.264 into raw video using the ffdec_h264 decoder.
class H264Decoder : public Decoder
{
    private: 
        void init();
        RtpPay* createDepayloader() const;
        const char *getCaps() const;
};



/// Encoder that encodes raw video into H.263 using the ffmpeg h263 encoder
class H263Encoder : public Encoder
{
    public: 
        H263Encoder();

    private:
        ~H263Encoder();

        void init();
        
        RtpPay* createPayloader() const;

        _GstElement *colorspc_;
        _GstElement *sinkElement() { return colorspc_; }

        /// No Copy Constructor
        H263Encoder(const H263Encoder&);     
        /// No Assignment Operator
        H263Encoder& operator=(const H263Encoder&);     
};

/// Decoder that decodes H.263 into raw video using the ffmpeg hq263 decoder.
class H263Decoder : public Decoder
{
    private: 
        void init();
        RtpPay* createDepayloader() const;
        const char *getCaps() const;
};



/// Encoder that encodes raw video into mpeg4 using the ffmpeg mpeg4 encoder
class Mpeg4Encoder : public Encoder
{
    public:
        Mpeg4Encoder();

    private:
        void init();
        _GstElement *colorspc_;
        _GstElement *sinkElement() { return colorspc_; }
        RtpPay* createPayloader() const;
        
        /// No Copy Constructor
        Mpeg4Encoder(const Mpeg4Encoder&);     
        /// No Assignment Operator
        Mpeg4Encoder& operator=(const Mpeg4Encoder&);     
};


/// Decoder that decodes mpeg4 into raw video using the ffmpeg mpeg4 decoder.
class Mpeg4Decoder: public Decoder
{
    private: 
        void init();
        RtpPay* createDepayloader() const;
        const char *getCaps() const;
};


/// Encoder that encodes raw audio using the vorbis encoder.
class VorbisEncoder : public AudioConvertedEncoder 
{
    public: 
        VorbisEncoder();

    private:
        void init();
        RtpPay* createPayloader() const;

        /// No Copy Constructor 
        VorbisEncoder(const VorbisEncoder&);     
        ///No Assignment Operator
        VorbisEncoder& operator=(const VorbisEncoder&);     
};

/// Decoder that decodes vorbis into raw audio using the vorbis decoder.
class VorbisDecoder : public Decoder
{
    private: 
        void init();
        RtpPay* createDepayloader() const;
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
        RtpPay* createPayloader() const;

        /// No Copy Constructor 
        LameEncoder(const LameEncoder&);     
        /// No Assignment Operator 
        LameEncoder& operator=(const LameEncoder&);     
};

/// Decoder that decodes mpeg to raw audio.

class MadDecoder : public AudioConvertedDecoder
{
    public:
        MadDecoder();
    private:
        void init();
        RtpPay* createDepayloader() const;

        /// No Copy Constructor 
        MadDecoder(const MadDecoder&);     
        ///No Assignment Operator
        MadDecoder& operator=(const MadDecoder&);     
};

#endif //_CODEC_H_

