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
#include "noncopyable.h"

// forward declarations
class _GstElement;
class RtpPay;
class Pay;
class Pipeline;

/** 
 *  Abstract child of Codec that wraps a single GstElement, and which exposes both a source and sink 
 *  and whose concrete subclasses will provide specifc encoding of raw media streams.
 */
class Encoder : public GstLinkableFilter, boost::noncopyable
{
    public:
        Encoder(const Pipeline &pipeline, const char *encoder);
        Encoder(const Pipeline &pipeline);
        virtual ~Encoder();
        /// Abstract Factory method that will create payloaders corresponding to this Encoder's codec type 
        virtual Pay* createPayloader() const = 0;
        int getBitrate() const;
        virtual void setBitrate(int bitrate);

    protected:
        const Pipeline &pipeline_;
        virtual void setBitrateInKbs(int bitrate);
        _GstElement *encoder_;

    private:
        _GstElement *srcElement() { return encoder_; }
        _GstElement *sinkElement() { return encoder_; }
};

/** 
 *  Abstract child of Codec that wraps a single GstElement, and which exposes both a source and sink 
 *  and whose concrete subclasses will provide specifc decoding of encoded media streams.
 */
class Decoder : public GstLinkableFilter, boost::noncopyable
{
    public:
        Decoder(const Pipeline &pipeline, const char *decoder);
        Decoder(const Pipeline &pipeline);
        virtual ~Decoder();
        /// Abstract Factory method that will create depayloaders corresponding to this Decoder's codec type 
        virtual RtpPay* createDepayloader() const = 0;
        virtual void adjustJitterBuffer() {}; // buy default, do nothing
        virtual bool adjustsBufferTime() { return false; }
        virtual unsigned long long minimumBufferTime() { THROW_ERROR("Unimplemented"); return 0; }
        
    protected:
        const Pipeline &pipeline_;
        _GstElement *decoder_;

    private:
        _GstElement *srcElement() { return decoder_; }
        _GstElement *sinkElement() { return decoder_; }
};


class VideoEncoder : public Encoder 
{
    public: 
        VideoEncoder(const Pipeline &pipeline, const char *encoder, bool supportsInterlaced);
        ~VideoEncoder();

    protected:
        _GstElement *colorspace_;
        bool supportsInterlaced_;

    private:
        
        _GstElement *sinkElement() 
        { 
            return colorspace_;
        }
};


class VideoDecoder : public Decoder 
{
    public: 
        VideoDecoder(const Pipeline &pipeline, const char *decoder, bool doDeinterlace);
        ~VideoDecoder();
        virtual void adjustJitterBuffer();
    
    protected:
        void addDeinterlace();
        bool doDeinterlace_;
        _GstElement *colorspace_;
        _GstElement *deinterlace_;
        static const unsigned long long LONGER_JITTER_BUFFER_MS = 60;

    private:
        _GstElement *srcElement() 
        { 
            if (!doDeinterlace_)
                return decoder_;
            else 
                return deinterlace_;
        }
};

/// Encoder that encodes raw video into H.264 using the x264 encoder
class H264Encoder : public VideoEncoder
{
    public: 
        H264Encoder(const Pipeline &pipeline, int bitrate);
        void setBitrate(int bitrate);

    private:
        ~H264Encoder();
        Pay* createPayloader() const;
        int bitrate_;
};

/// Decoder that decodes H.264 into raw video using the ffdec_h264 decoder.
class H264Decoder : public VideoDecoder {
    public:
        H264Decoder(const Pipeline &pipeline, bool doDeinterlace);
    private: 
        RtpPay* createDepayloader() const;
        void adjustJitterBuffer(); 
};



/// Encoder that encodes raw video into H.263 using the ffmpeg h263 encoder
class H263Encoder : public VideoEncoder
{
    public: 
        H263Encoder(const Pipeline &pipeline, int bitrate);

    private:
        int bitrate_;
        ~H263Encoder();

        Pay* createPayloader() const;
};

/// Decoder that decodes H.263 into raw video using the ffmpeg h263 decoder.
class H263Decoder : public VideoDecoder
{
    public:
        H263Decoder(const Pipeline &pipeline, bool doDeinterlace);
    private: 
        RtpPay* createDepayloader() const;
};



/// Encoder that encodes raw video into mpeg4 using the ffmpeg mpeg4 encoder
class Mpeg4Encoder : public VideoEncoder
{
    public:
        Mpeg4Encoder(const Pipeline &pipeline, int bitrate);
        ~Mpeg4Encoder();

    private:
        int bitrate_;
        Pay* createPayloader() const;
};


/// Decoder that decodes mpeg4 into raw video using the ffmpeg mpeg4 decoder.
class Mpeg4Decoder: public VideoDecoder
{
    public:
        Mpeg4Decoder(const Pipeline &pipeline, bool doDeinterlace);
    private: 
        RtpPay* createDepayloader() const;
};


/// Encoder that encodes raw video into theora using the theoraenc encoder
class TheoraEncoder : public VideoEncoder
{
    public:
        TheoraEncoder(const Pipeline &pipeline, int bitrate, int quality);
        ~TheoraEncoder();
        void setBitrate(int bitrate);
        void setQuality(int quality);
        void setSpeedLevel(int speedLevel);

    private:
        static const int MAX_SPEED_LEVEL = 2;
        static const int MIN_SPEED_LEVEL = 0;
        static const int MIN_QUALITY = 0;
        static const int MAX_QUALITY = 63;  // defined in plugin
        static const int INIT_QUALITY = 20;
        Pay* createPayloader() const;
        int bitrate_;
        int quality_;
};


/// Decoder that decodes mpeg4 into raw video using the theoradec decoder.
class TheoraDecoder: public VideoDecoder
{
    public:
        TheoraDecoder(const Pipeline &pipeline, bool doDeinterlace);
    private: 
        RtpPay* createDepayloader() const;
};


/// Encoder that encodes raw audio using the vorbis encoder.
class VorbisEncoder : public Encoder
{
    public: 
        VorbisEncoder(const Pipeline &pipeline, int bitrate, double quality);

    private:
        ~VorbisEncoder();
        _GstElement *sinkElement() { return queue_; }
        Pay* createPayloader() const;
        _GstElement *queue_;
};

/// Decoder that decodes vorbis into raw audio using the vorbis decoder.
class VorbisDecoder : public Decoder
{
    public: 
        VorbisDecoder(const Pipeline &pipeline);
        bool adjustsBufferTime() { return true; }
        unsigned long long minimumBufferTime();
    private: 
        RtpPay* createDepayloader() const;
        static const unsigned long long MIN_BUFFER_USEC = 100000;
};

/// Encoder that simply performs datatype conversion on raw audio.
class RawEncoder : public Encoder
{
    public:
        RawEncoder(const Pipeline &pipeline);
        ~RawEncoder();
        _GstElement *sinkElement() { return aconv_; }
        _GstElement *srcElement() { return aconv_; }

    private:
        _GstElement *aconv_;
        Pay* createPayloader() const;
};

/// Decoder that simply performs datatype conversion on raw audio.
class RawDecoder : public Decoder
{
    public:
        RawDecoder(const Pipeline &pipeline);
        ~RawDecoder();

    private:
        RtpPay* createDepayloader() const;
        _GstElement *aconv_;

        _GstElement *sinkElement() { return aconv_; }
        _GstElement *srcElement() { return aconv_; }
};


/// Encoder that encodes raw audio to mpeg.
class LameEncoder : public Encoder
{
    public:
        LameEncoder(const Pipeline &pipeline, int bitrate, double quality);
        ~LameEncoder();

    private:
        _GstElement *aconv_;
        _GstElement *mp3parse_;
        Pay* createPayloader() const;
        _GstElement *sinkElement() { return aconv_; }
        _GstElement *srcElement() { return mp3parse_; }
};

/// Decoder that decodes mpeg to raw audio.

class MadDecoder : public Decoder
{
    public:
        MadDecoder(const Pipeline &pipeline);
        ~MadDecoder();
    private:
        _GstElement *srcElement() { return aconv_; }
        _GstElement *aconv_;
        RtpPay* createDepayloader() const;
};

#endif //_CODEC_H_

