all:
	@echo Do some code validation in $(PWD)

clean:
	@echo Do some cleanup in $(PWD) 

#test_addressbook.py 
check: test_com_chan.py \
		test_common.py test_devices.py test_i18n.py \
		test_log.py test_network.py test_observer.py \
		solo_test_telnet_settings.py 

%.py: 
	trial test/$@


html:
	@echo Do some html in $(PWD)

install:
	@echo Install $(PWD)

# in case target exits in directory
# u want to 
.PHONY: clean test 
