# Makefile for the pplMe projectlette's codes.


# The libraries that consitute pplMe.
libdirs = $(wildcard lib*)


# Default to building just the libraries (for now).
all:	$(libdirs)


# How to build pplMe.
.PHONY:	$(libdirs)
$(libdirs):	
	cd $@ && make $@


# How to clean up.
libdirs_clean = $(patsubst %,%_clean,$(libdirs))
.PHONY:	$(libdirs_clean)
$(libdirs_clean):
	cd $(patsubst %_clean,%,$@) && make clean

.PHONY:	clean
clean:	$(libdirs_clean)


# How to test.
libdirs_test = $(patsubst %,%_test,$(libdirs))
.PHONY:	$(libdirs_test)
$(libdirs_test):
	cd $(patsubst %_test,%,$@) && make test

.PHONY:	test
test:	$(libdirs_test)
