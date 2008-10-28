if PROJ_DEBUG
if SIP
SUBDIRS = util util/test gutil tcp tcp/test gst gst/test main sip sip/test
else
SUBDIRS = util util/test gutil tcp tcp/test gst gst/test main 
endif

else
if SIP
SUBDIRS = util gutil tcp gst main sip
else
SUBDIRS = util gutil tcp gst main  
endif
endif
