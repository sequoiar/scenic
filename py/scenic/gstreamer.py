#!/usr/bin/env python
"""
Gstreamer-related tools.
"""
import pygst
pygst.require('0.10')
import gst
from scenic import logger

log = logger.start(name="gst")

def is_gstreamer_element_found(name):
    """
    Checks if a given Gstreamer element is installed.
    @rettype: bool
    """
    ok = gst.element_factory_find(name) is not None 
    if not ok:
        log.info("Could not find Gstreamer element %s." % (name))
    return ok

def is_codec_supported(codec):
    """
    Checks if a codec is supported by the Gstreamer elements found on the system.
    """
    _elements = {
        "mp3": ["lamemp3enc", "mp3parse", "mad"],
        "theora": ["theoraenc", "theoradec"],
        "h263": ["ffenc_h263p"],
        "h264": ["x264enc"],
        "mpeg4": ["ffenc_mpeg4"]
        }
    if not _elements.has_key(codec):
        return True
    else:
        needed = _elements[codec]
        ret = True
        for element in needed:
            if not is_gstreamer_element_found(element):
                ret = False
        if not ret:
            log.warning("Codec %s is NOT supported." % (codec))
        return ret

