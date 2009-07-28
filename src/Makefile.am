include $(top_srcdir)/globals.mak
#if SIP
#SIPDIR = sip
#endif
if BOOST_PYTHON
PYTHONDIR = python
endif
SUBDIRS = include boost util gutil tcp gst main devices $(PYTHONDIR) $(SIPDIR)

# install-exec-hook:
# 		help2man -N -i MANPAGE.txt -n "Milhouse is a High-Quality Bi-Directional Audio/Video Streaming Tool" milhouse > milhouse.1
# 		install milhouse.1 $(DESTDIR)$(prefix)/share/man/man1/milhouse.1
# 
# uninstall-hook:
# 		rm -f $(DESTDIR)$(prefix)/share/man/man1/milhouse.1
