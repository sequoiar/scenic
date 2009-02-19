all:
	@echo Do some code validation in $(PWD)
	find . -name \*.py -exec py_compilefiles {} \;

clean:
	@echo Do some cleanup in $(PWD) 
	find . -name \*.pyc -exec rm {} \;

check: test_com_chan.py test_addressbook.py \
		test_common.py test_devices.py test_i18n.py \
		test_log.py test_network.py test_observer.py \
		solo_test_telnet_settings.py 

%.py: 
	@echo ==================== test/$@ ======================
	trial test/$@


html:
	@echo Do some html in $(PWD)

install:
	@echo Install $(PWD)

# in case target exits in directory
# u want to 
.PHONY: clean test 
