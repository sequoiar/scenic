include $(top_srcdir)/src/globals.mak

lib_LTLIBRARIES = milhouse.la

milhouse_la_SOURCES = exports.cpp pyMsgThread.cpp
milhouse_la_LDFLAGS = -version-info 1:1:1 -module

# add your libs i.e. ../hello/libhello.la  
milhouse_la_LIBADD =  -l$(BOOST_PYTHON_LIB) $(GLIB_LIBS) \
						   $(ALL_GST_LIBS) $(JACK_LIBS) $(src)/tcp/libtcp.a \
						   $(src)/gst/control/libgstthread.a \
						   $(src)/gst/engine/libgst.a $(GTK_LIBS) \
						   $(LIBAVC1394_LIBS) $(GL_LIBS) $(GLU_LIBS) $(src)/util/libutil.a \
						   $(src)/gutil/libgutil.a 

milhouse_la_CPPFLAGS = -I$(PYTHON_INCLUDE_DIR) $(AM_CPPFLAGS)

#Oinstall-libLTLIBRARIES: $(lib_LTLIBRARIES)
#   	@$(NORMAL_INSTALL)
#   	test -z "$(libdir)" || $(mkdir_p) "$(DESTDIR)$(libdir)"

install-libLTLIBRARIES: $(lib_LTLIBRARIES)
	@list='$(lib_LTLIBRARIES)'; for p in $$list; do \
	  if test -f $$p; then \
	  	if  ! \[ -d '$(DESTDIR)$(pyexecdir)' \]; then \
			echo "mkdir -p '$(DESTDIR)$(pyexecdir)'"; \
			mkdir -p "$(DESTDIR)$(pyexecdir)"; \
		fi; \
	    f=$(am__strip_dir) \
	    echo " $(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) '$$p' '$(DESTDIR)$(pyexecdir)/$$f'"; \
	    $(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) "$$p" "$(DESTDIR)$(pyexecdir)/$$f"; \
	  else :; fi; \
	 done

uninstall-libLTLIBRARIES:
	@$(NORMAL_UNINSTALL)
	@list='$(lib_LTLIBRARIES)'; for p in $$list; do \
	  p=$(am__strip_dir) \
	  echo " $(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=uninstall rm -f '$(DESTDIR)$(pyexecdir)/$$p'"; \
	  $(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=uninstall rm -f "$(DESTDIR)$(pyexecdir)/$$p"; \
	done

