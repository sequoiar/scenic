include $(top_srcdir)/globals.mak 

TESTS_ENVIRONMENT = LANG=en_CA.UTF-8 trial
        
all: miville.webapp

miville.webapp: webapp.ini
	@zip miville.webapp webapp.ini 
	

install-exec-hook: 
		install -m 644 -D prism-miville $(DESTDIR)$(bindir)/prism-miville
		install -m 644 -D miville.webapp $(DESTDIR)$(prefix)/share/prism/apps/miville.webapp

uninstall-hook:
		rm -f $(DESTDIR)$(bindir)/prism-miville
		rm -f $(DESTDIR)$(prefix)/share/prism/apps/miville.webapp

clean:
		rm -f miville.webapp
