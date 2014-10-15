# Makefile that does the bulk of the business of building pplMe.


# Rely upon the fact that this makefile is in the root src directory.
PPLME_SRC_DIR := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))


# Various libraries that we rely upon.
googletest = $(THIRDPARTY)/googletest


# Classic flags and whatnot for the builtin rules.
CPPFLAGS = -I$(googletest)/include -I$(PPLME_SRC_DIR)
CXXFLAGS = -m64 -std=c++11 -Wall -Wextra -Werror -pedantic-errors -g -O2
LDFLAGS = -g -O2
LDLIBS = -lpthread $(googletest)/make/gtest_main.a


# Stuffs related to the library at hand.
src = $(wildcard *.cc)
objs = $(src:.cc=.o)
lib = $(patsubst lib%,lib%.a,$(component))


# Build that library!
$(lib):	$(objs)
	$(AR) $(ARFLAGS) $@ $^


# Stuffs related to testing the library at hand.
tests_dir = $(component)_tests
tests_src = $(wildcard $(tests_dir)/*.cc)
tests_objs = $(tests_src:.cc=.o)
tests_bin = $(tests_dir)/$(component)_tests


# Build those tests!
$(tests_bin):	$(lib) $(tests_objs)
		$(CXX) $(LDFLAGS) $(LDLIBS) $^ $(lib) -o $@


# Run those tests!
.PHONY:	test
test:	$(tests_bin)
	$(tests_bin)


# Clean all the things!
.PHONY:	clean
clean:
	-rm $(tests_bin) $(tests_objs) $(lib) $(objs) 2>/dev/null
