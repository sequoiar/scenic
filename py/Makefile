all:
	@echo Do some code validation in $(PWD)

clean:
	@echo Do some cleanup in $(PWD) 

check:
	@echo Do tests in $(PWD)
	rm test/*.pyc
	trial test/solo_test_telnet_settings.py
	trial test
	
html:
	@echo Do some html in $(PWD)

install:
	@echo Install $(PWD)

# in case target exits in directory
# u want to 
.PHONY: clean test 
