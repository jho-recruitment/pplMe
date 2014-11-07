# Makefile for the pplMe projectlette.


# Environment.
SHELL = /bin/bash


# Stuff that you might want to override.
export DOXYGEN = doxygen


# Build the Gubbins.
.PHONY:	all
all:	pplMe

.PHONY:	3rdParty
3rdParty:
	cd 3rdParty && $(MAKE)

.PHONY:	pplMe
pplMe:
	cd src && $(MAKE) THIRDPARTY=$(realpath 3rdParty)


# Test the Gubbins.
.PHONY:	test
test:
	cd src && $(MAKE) THIRDPARTY=$(realpath 3rdParty) test


# Render the documentation.
.PHONY:	doco
doco:
	cd doco && $(MAKE) doco


# Clean Up.
.PHONY:	clean
clean:
	cd src && $(MAKE) clean
.PHONY:	megaclean
megaclean:
	cd doco && $(MAKE) clean
	cd 3rdParty && $(MAKE) clean
