# Makefile for the pplMe projectlette's codes.


# The libraries that constitute pplMe.
libdirs = $(wildcard lib*)


# The binaries that constitude pplMe.
binaries = pplmed pplmec


# Default to building everything.
libsnbins = $(libdirs) $(binaries)
all:	$(libsnbins)


# How to build pplMe.
.PHONY:	$(libsnbins)
$(libsnbins):
	cd $@ && make $@


# How to clean up.
libsnbins_clean = $(patsubst %,%_clean,$(libsnbins))
.PHONY:	$(libsnbins_clean)
$(libsnbins_clean):
	cd $(patsubst %_clean,%,$@) && make clean

.PHONY:	clean
clean:	$(libsnbins_clean)


# How to test.
libdirs_test = $(patsubst %,%_test,$(libdirs))
.PHONY:	$(libdirs_test)
$(libdirs_test):
	cd $(patsubst %_test,%,$@) && make test

.PHONY:	test
test:	$(libdirs) $(libdirs_test)


# This is a bit of a hack; I was so proud of it, but I think my goal of having
# each library/binary siloed in its own directory with its own makefile (albeit
# reusing common.mak) may have been folly.  If only I had properly understood
# <http://aegis.sourceforge.net/auug97.pdf> ["Recursive Make Considered
# Harmful"].
# Requiescat in pace, Peter Miller.
export ALL_PPLME_LIBS = $(foreach lib,$(libdirs),../$(lib)/$(lib).a)
