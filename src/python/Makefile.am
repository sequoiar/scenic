include $(top_srcdir)/src/globals.mak

lib_LTLIBRARIES = libmsgthreads.la

libmsgthreads_la_SOURCES = exports.cpp
libmsgthreads_la_LDFLAGS = -version-info 1:1:1

# add your libs i.e. ../hello/libhello.la  
libmsgthreads_la_LIBADD =  -l$(BOOST_PYTHON_LIB) $(GLIB_LIBS) \
						   $(src)/gst/control/libgstthread.a \
						   $(ALL_GST_LIBS) $(JACK_LIBS) $(src)/tcp/libtcp.a \
						   $(src)/gst/engine/libgst.a $(GTK_LIBS) \
						   $(LIBAVC1394_LIBS) $(GL_LIBS) $(GLU_LIBS) $(src)/util/libUtil.a $(src)/gutil/libGUtil.a 

libmsgthreads_la_CPPFLAGS = -I$(PYTHON_INCLUDE_DIR) $(AM_CPPFLAGS)

#Oinstall-libLTLIBRARIES: $(lib_LTLIBRARIES)
#   	@$(NORMAL_INSTALL)
#   	test -z "$(libdir)" || $(mkdir_p) "$(DESTDIR)$(libdir)"

install-libLTLIBRARIES: $(lib_LTLIBRARIES)
	@list='$(lib_LTLIBRARIES)'; for p in $$list; do \
	  if test -f $$p; then \
	    f=$(am__strip_dir) \
	    echo " $(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) '$$p' '$(pyexecdir)/$$f'"; \
	    $(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) "$$p" "$(pyexecdir)/$$f"; \
	  else :; fi; \
	 done

uninstall-libLTLIBRARIES:
	@$(NORMAL_UNINSTALL)
	@list='$(lib_LTLIBRARIES)'; for p in $$list; do \
	  p=$(am__strip_dir) \
	  echo " $(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=uninstall rm -f '$(pyexecdir)/$$p'"; \
	  $(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=uninstall rm -f "$(pyexecdir)/$$p"; \
	done

