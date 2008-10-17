if DEBUG
if SIP
	SIPDIR = sip sip/test
endif
SUBDIRS = util util/test gutil tcp tcp/test gst gst/test main $(SIPDIR)
else
if SIP
	SIPDIR = sip 
endif
SUBDIRS = util gutil tcp gst main $(SIPDIR) 
endif
