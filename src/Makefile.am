if PROJ_DEBUG
if SIP
SUBDIRS = util util/test gutil tcp tcp/test gst gst/test main sip sip/test
endif
if !SIP
SUBDIRS = util util/test gutil tcp tcp/test gst gst/test main 
endif

else
if SIP
SUBDIRS = util gutil tcp gst main sip
endif
if !SIP
SUBDIRS = util gutil tcp gst main  
endif
endif
