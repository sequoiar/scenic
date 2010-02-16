man_MANS = milhouse.1 scenic.1 midistream.1
EXTRA_DIST = $(man_MANS)
MAINTAINERCLEANFILES = $(man_MANS)

# The man pages depend on the --help strings and the version number.
common_mandeps = $(top_srcdir)/configure.ac

# Depend on the source, not the binary; we don't need to regenerate the
# binary when any source file changes, only the main one.
# Use -o so that the `missing' program can infer the output file.
milhouse.1: $(top_srcdir)/src/main/milhouse.cpp $(top_srcdir)/src/util/programOptions.cpp $(common_mandeps)
	$(HELP2MAN) --include=milhouse-man.txt --name="Milhouse is a Gstreamer application for audio and video over RTP" --no-info ../src/main/milhouse -o $@

scenic.1: $(top_srcdir)/py/scenic/runner.py $(common_mandeps)
	$(HELP2MAN) --include=scenic-man.txt --name="Scenic Audio/Video Streaming Software for GNU/Linux" --no-info ../py/scripts/scenic -o $@

midistream.1: $(top_srcdir)/py/scripts/midistream.in $(common_mandeps)
	PYTHONPATH=../py:$(PYTHONPATH) $(HELP2MAN) --name="MIDI RTP Streaming Software" --no-info ../py/scripts/midistream -o $@
