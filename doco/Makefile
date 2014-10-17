# Makefile for the pplMe documentation.


# Environment.
SHELL = /bin/bash


# Various variables.
doxygen_config = Doxyfile


.PHONY:	all
all:	doco


.PHONY: doco
doco:	src-doco


.PHONY:
src-doco:
	$(DOXYGEN) $(doxygen_config)


.PHONY:	clean
clean:
	-rm -rf html
