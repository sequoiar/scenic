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

// forward declarations
class _GstElement;
class RtpPay;

/** \class Codec
 *  Abstract base class that wraps a single GstElement, and which exposes both a source and sink.
 */

class Codec : public GstLinkableFilter
{
    public:
        /** Constructor */
        Codec()
            : codec_(0) {};
        /** 
         * Destructor */
        ~Codec();

    protected:

        _GstElement *codec_;

    private:

        _GstElement *srcElement() { return codec_; }
        _GstElement *sinkElement() { return codec_; }

        Codec(const Codec&);     //No Copy Constructor
        Codec& operator=(const Codec&);     //No Assignment Operator
};

/** \class Encoder
 *  Abstract child of Codec that wraps a single GstElement, and which exposes both a source and sink 
 *  and whose concrete subclasses will provide specifc encoding of raw media streams.
 */

class Encoder : public Codec
{
    public:
        /** Abstract Factory method that will create payloaders corresponding to this Encoder's codec type */
        virtual RtpPay* createPayloader() const = 0;
};

/** \class Decoder
 *  Abstract child of Codec that wraps a single GstElement, and which exposes both a source and sink 
 *  and whose concrete subclasses will provide specifc decoding of encoded media streams.
 */

class Decoder : public Codec
{
    public:
        /** Abstract Factory method that will create depayloaders corresponding to this Decoder's codec type */
        virtual RtpPay* createDepayloader() const = 0;
        //virtual void setSrcCaps(){ LOG_DEBUG("BASE DECODER");};
};

/** \class H264Encoder
 *  Encoder that encodes raw video into H.264 using the x264 encoder
 */

class H264Encoder : public Encoder
{
    public: 

        /** Constructor */
        H264Encoder() : colorspc_(0) {};

    private:
        /** 
         * Destructor */
        ~H264Encoder();

        void init();
        /** 
         * Creates an h.264 rtp payloader */
        RtpPay* createPayloader() const;

        /** Exposes the sink of this encoder, which is a colorspace converter */
        _GstElement *sinkElement() { return colorspc_; }
        
        _GstElement *colorspc_;
        
        H264Encoder(const H264Encoder&);     //No Copy Constructor
        H264Encoder& operator=(const H264Encoder&);     //No Assignment Operator
};

/** \class H264Decoder
 *  Decoder that decodes H.264 into raw video using the ffdec_h264 decoder.
 */

class H264Decoder : public Decoder
{
    private: 

        void init();
        /** Creates an h.264 RtpDepayloader */
        RtpPay* createDepayloader() const;
};

/** \class VorbisEncoder
 *  Encoder that encodes raw audio using the vorbis encoder.
 */

class VorbisEncoder : public Encoder 
{
    public: 
        /** Constructor */
        VorbisEncoder() : aconv_(0){};

    private:
        /** Destructor */
        ~VorbisEncoder();
        void init();
        /** Creates an RtpVorbisPayloader */
        RtpPay* createPayloader() const;

        _GstElement *sinkElement() { return aconv_; }

        _GstElement *aconv_;
        /** No Copy Constructor */
        VorbisEncoder(const VorbisEncoder&);     
        /**No Assignment Operator*/
        VorbisEncoder& operator=(const VorbisEncoder&);     
};

/** \class VorbisDecoder
 *  Decoder that decodes vorbis into raw audio using the vorbis decoder.
 */

class VorbisDecoder : public Decoder
{
    private: 

        void init();
//        void setSrcCaps();
        /** Creates an RtpVorbisDepayloader */
        RtpPay* createDepayloader() const;
};

/** \class RawEncoder
 *  Encoder that simply performs datatype conversion on raw audio.
 */

// FIXME: DRY!!!!
class RawEncoder : public Encoder 
{
    public: 
        /** Constructor */
        RawEncoder() : aconv_(0) {};
    
    private:
        /** 
         * Destructor */
        ~RawEncoder();
        void init();
        /** Creates an RtpL16Payloader */
        RtpPay* createPayloader() const;
    
        _GstElement *srcElement() { return aconv_; }
        _GstElement *sinkElement() { return aconv_; }
        _GstElement *aconv_;
        
        /** No Copy Constructor */
        RawEncoder(const RawEncoder&);     
        /**No Assignment Operator*/
        RawEncoder& operator=(const RawEncoder&);     
};

/** \class RawDecoder
 *  Decoder that simply performs datatype conversion on raw audio.
 */

class RawDecoder : public Decoder
{
    public: 
        /** Constructor */
        RawDecoder() : aconv_(0) {};

    private:
        /** 
         * Destructor */
        ~RawDecoder();
        void init();
        /** 
         * Creates an RtpL16Depayloader */
        RtpPay* createDepayloader() const;

        _GstElement *srcElement() { return aconv_; }
        _GstElement *sinkElement() { return aconv_; }
        _GstElement *aconv_;
        
        /**No Copy Constructor*/
        RawDecoder(const RawDecoder&);     
        /**No Assignment Operator*/
        RawDecoder& operator=(const RawDecoder&);     
};

/** \class LameEncoder
 *  Encoder that encodes raw audio to mpeg.
 */

class LameEncoder : public Encoder 
{
    public:
        /** Constructor */
        LameEncoder() : aconv_(0) {};
    private:
        /** 
         * Destructor */
        ~LameEncoder();
        void init();
        /** 
         * Creates an RtpMpaPayloader */
        RtpPay* createPayloader() const;
    
        _GstElement *sinkElement() { return aconv_; }

        _GstElement *aconv_;
        /** No Copy Constructor */
        LameEncoder(const LameEncoder&);     
        /** No Assignment Operator */
        LameEncoder& operator=(const LameEncoder&);     
};

/** \class MadDecoder
 * Decoder that decodes mpeg to raw audio.
 */

class MadDecoder : public Decoder
{
    public: 
        /** Constructor */  
        MadDecoder() : aconv_(0) {};
    private:
        /** Destructor */  
        ~MadDecoder();
        void init();
        /** 
         * Creates an RtpMpaDepayloader */
        RtpPay* createDepayloader() const;

        _GstElement *srcElement() { return aconv_; }
        _GstElement *aconv_;
        /** No Copy Constructor */
        MadDecoder(const MadDecoder&);     
        /**No Assignment Operator*/
        MadDecoder& operator=(const MadDecoder&);     
};

#endif //_CODEC_H_

