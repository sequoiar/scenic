include $(top_srcdir)/globals.mak

TESTS_ENVIRONMENT = PYTHONPATH=$(top_srcdir)/src/python/.libs $(PYTHON) 
TESTS = 
#test.py

lib_LTLIBRARIES = pymilhouse.la

pymilhouse_la_SOURCES = exports.cpp pyMsgThread.cpp ../main/msgThreadFactory.cpp pyCallback.h pyMsgThread.h



pymilhouse_la_LDFLAGS = -version-info 1:1:1 -module $(BOOST_LDFLAGS)

# add your libs i.e. ../hello/libhello.la  
pymilhouse_la_LIBADD =  -l$(BOOST_PYTHON_LIB)-mt $(GLIB_LIBS) \
						   $(ALL_GST_LIBS) $(JACK_LIBS) $(BOOST_ASIO_LIB) \
						   $(top_builddir)/src/gst/control/libgstthread.la \
						   $(top_builddir)/src/gst/engine/libgst.la $(GTK_LIBS) \
						   $(LIBAVC1394_LIBS) $(GL_LIBS) $(GLU_LIBS) $(top_builddir)/src/util/libutil.la \
						   $(top_builddir)/src/boost/libbutil.la $(BOOST_THREAD_LIB) $(top_builddir)/src/tcp/libtcp.la 

pymilhouse_la_CPPFLAGS = -I$(PYTHON_INCLUDE_DIR) $(AM_CPPFLAGS) $(BOOST_CPPFLAGS)

install-exec-hook:
	@echo "install -D $(DESTDIR)$(libdir)/$(lib_LTLIBRARIES) $(DESTDIR)$(pyexecdir)/$(lib_LTLIBRARIES) " 
	@install -D "$(DESTDIR)$(libdir)/$(lib_LTLIBRARIES)" "$(DESTDIR)$(pyexecdir)/$(lib_LTLIBRARIES)" 

#Oinstall-libLTLIBRARIES: $(lib_LTLIBRARIES)
#   	@$(NORMAL_INSTALL)
#   	test -z "$(libdir)" || $(mkdir_p) "$(DESTDIR)$(libdir)"

#install-libLTLIBRARIES: $(lib_LTLIBRARIES)
#	@$(NORMAL_INSTALL)
#	test -z "$(libdir)" || $(MKDIR_P) "$(DESTDIR)$(libdir)"
#	@list='$(lib_LTLIBRARIES)'; for p in $$list; do \
#	  if test -f $$p; then \
#	    f=$(am__strip_dir) \
#	    echo " $(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) '$$p' '$(DESTDIR)$(libdir)/$$f'"; \
#	    $(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) "$$p" "$(DESTDIR)$(libdir)/$$f"; \
#	  else :; fi; \
#    done

#install-libLTLIBRARIES: $(lib_LTLIBRARIES)
#@list='$(lib_LTLIBRARIES)'; for p in $$list; do \
#if test -f $$p; then \
#if  ! \[ -d '$(DESTDIR)$(libdir)' \]; then \
#echo "mkdir -p '$(DESTDIR)$(libdir)'"; \
#mkdir -p "$(DESTDIR)$(libdir)"; \
#fi; \
#f=$(am__strip_dir) \
#echo " $(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) '$$p' '$(DESTDIR)$(libdir)/$$f'"; \
#$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) "$$p" "$(DESTDIR)$(libdir)/$$f"; \
#echo " install -D '$(DESTDIR)$(libdir)/$$f' '$(DESTDIR)$(pyexecdir)/$$f'"; \
#install -D "$(DESTDIR)$(libdir)/$$f" "$(DESTDIR)$(pyexecdir)/$$f"; \
#else :; fi; \
#done
#
#uninstall-libLTLIBRARIES:
#@$(NORMAL_UNINSTALL)
#@list='$(lib_LTLIBRARIES)'; for p in $$list; do \
#p=$(am__strip_dir) \
#echo " $(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=uninstall rm -f '$(DESTDIR)$(libdir)/$$p'"; \
#$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=uninstall rm -f "$(DESTDIR)$(libdir)/$$p"; \
#rm -f "$(DESTDIR)$(pyexecdir)/$$p"; \
#done

