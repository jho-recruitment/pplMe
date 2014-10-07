# Makefile for the pplMe projectlette.


# Environment
SHELL = /bin/bash


# Build the Gubbins
.PHONY:	all
all:	pplMe


.PHONY:	3rdParty
3rdParty:
	cd 3rdParty && $(MAKE)


.PHONY:	pplMe
pplMe:	3rdParty
	cd src && $(MAKE)


# Clean Up
.PHONY:	clean
clean:
	cd src && $(MAKE) clean
	cd 3rdParty && $(MAKE) clean
