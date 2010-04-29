man_MANS = milhouse.1 scenic.1 midistream.1 firereset.1
EXTRA_DIST = $(man_MANS)
MAINTAINERCLEANFILES = $(man_MANS)

# The man pages depend on the --help strings and the version number.
common_mandeps = $(top_srcdir)/configure.ac

# Depend on the source, not the binary; we don't need to regenerate the
# binary when any source file changes, only the main one.
# Use -o so that the `missing' program can infer the output file.
milhouse.1: $(top_srcdir)/src/main/milhouse.cpp $(top_srcdir)/src/util/programOptions.cpp $(common_mandeps) $(top_srcdir)/man/milhouse-man.txt
	$(HELP2MAN) --include=milhouse-man.txt --name="Milhouse is a Gstreamer application for audio and video over RTP" --no-info $(top_builddir)/src/main/milhouse -o $@

scenic.1: $(top_srcdir)/py/scenic/runner.py $(top_srcdir)/py/scripts/scenic.in  $(top_srcdir)/py/scenic/configure.py.in $(common_mandeps) $(top_srcdir)/man/scenic-man.txt
	$(HELP2MAN) --include=scenic-man.txt --name="Scenic Audio/Video Streaming Software for GNU/Linux" --no-info $(top_builddir)/py/scripts/scenic -o $@

midistream.1: $(top_srcdir)/py/scripts/midistream.in $(top_srcdir)/py/rtpmidi/runner.py $(common_mandeps) $(top_srcdir)/man/midistream-man.txt
	$(HELP2MAN) --include=midistream-man.txt --name="MIDI RTP Streaming Software" --no-info $(top_builddir)/py/scripts/midistream -o $@

firereset.1: $(top_srcdir)/src/firereset/firereset.c $(top_srcdir)/man/firereset-man.txt
	$(HELP2MAN) --include=firereset-man.txt --name="Firereset resets a ieee1394 bus to help in case of problems" --no-info $(top_builddir)/src/firereset/firereset -o $@
