SUBDIRS = doc scenic scripts rtpmidi

TESTS_ENVIRONMENT = LANG=en_CA.UTF-8 PATH=../../src/main:$(PWD)/scripts:$(PATH) PYTHONPATH=../../py:$(PYTHONPATH) trial
        
TESTS = \
	scenic/test/test_cameras.py \
	scenic/test/test_communication.py \
	scenic/test/test_gstreamer.py \
	scenic/test/test_midi.py \
	scenic/test/test_ports.py \
	scenic/test/test_signals.py
	scenic/test/test_x11.py

desktopdir = $(datadir)/applications
desktop_DATA = scenic.desktop

EXTRA_DIST = $(desktop_DATA) scenic.svg

clean-local:
	rm -rf _trial_temp

pychecker:
	rm -f pychecker.log
	for f in `find ./ -type f -name \*.py`; do pychecker --limit 100 $$f >> pychecker.log; done; echo "Wrote pychecker output to pychecker.log"
