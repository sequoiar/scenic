dist_man_MANS = milhouse.1 scenic.1 midistream.1 firereset.1 jack-info.1 dc-ctl.1
MAN_TEXT_FILES=firereset-man.txt jack-info-man.txt midistream-man.txt milhouse-man.txt scenic-man.txt dc-ctl-man.txt
EXTRA_DIST = $(dist_man_MANS) $(MAN_TEXT_FILES)
MAINTAINERCLEANFILES = $(dist_man_MANS)

# The man pages depend on the --help strings and the version number.
common_mandeps = $(top_srcdir)/configure.ac

# Depend on the source, not the binary; we don't need to regenerate the
# binary when any source file changes, only the main one.
# Use -o so that the `missing' program can infer the output file.
milhouse.1: $(top_srcdir)/src/main/milhouse.cpp $(top_srcdir)/src/main/program_options.cpp $(common_mandeps) $(top_srcdir)/man/milhouse-man.txt
	$(HELP2MAN) --include=$(top_srcdir)/man/milhouse-man.txt --name="Streams audio and video with GStreamer over RTP" --no-info $(top_builddir)/src/main/milhouse -o $@

scenic.1: $(top_srcdir)/py/scenic/runner.py $(top_srcdir)/py/scripts/scenic.in  $(top_srcdir)/py/scenic/configure.py.in $(common_mandeps) $(top_srcdir)/man/scenic-man.txt
	$(HELP2MAN) --include=$(top_srcdir)/man/scenic-man.txt --name="Telepresence application for live performance and installations" --no-info $(top_builddir)/py/scripts/scenic -o $@

midistream.1: $(top_srcdir)/py/scripts/midistream.in $(top_srcdir)/py/rtpmidi/runner.py $(common_mandeps) $(top_srcdir)/man/midistream-man.txt
	$(HELP2MAN) --include=$(top_srcdir)/man/midistream-man.txt --name="Streams MIDI notes and controls over RTP" --no-info $(top_builddir)/py/scripts/midistream -o $@

firereset.1: $(top_srcdir)/utils/firereset/firereset.c $(top_srcdir)/man/firereset-man.txt
	$(HELP2MAN) --include=$(top_srcdir)/man/firereset-man.txt --name="Resets a IEEE 1394 bus to help in case of problems" --no-info $(top_builddir)/utils/firereset/firereset -o $@

jack-info.1: $(top_srcdir)/utils/jack-info/jack_info.c $(top_srcdir)/man/jack-info-man.txt
	$(HELP2MAN) --include=$(top_srcdir)/man/jack-info-man.txt --name="Prints informations about running JACK servers" --no-info $(top_builddir)/utils/jack-info/jack-info -o $@

dc-ctl.1: $(top_srcdir)/utils/dc-ctl/dc-ctl.cpp  $(top_srcdir)/man/dc-ctl-man.txt
	$(HELP2MAN) --include=$(top_srcdir)/man/dc-ctl-man.txt --name="Image control for IIDC IEEE 1394 cameras" --no-info $(top_builddir)/utils/dc-ctl/dc-ctl -o $@

