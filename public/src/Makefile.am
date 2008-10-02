if DEBUG
if SIP
	SIPDIR = sip sip/test
endif
SUBDIRS = util util/test tcp tcp/test gst gst/test main $(SIPDIR)
else
if SIP
	SIPDIR = sip 
endif
SUBDIRS = util tcp gst main $(SIPDIR) 
endif
