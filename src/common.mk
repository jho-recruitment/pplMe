# Makefile that does the bulk of the business of building pplMe.


# Environment.
SHELL = /bin/bash


# Rely upon the fact that this makefile is in the root src directory.
PPLME_SRC_DIR := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))


# Various libraries that we rely upon.
googletest = $(THIRDPARTY)/googletest
boost = $(THIRDPARTY)/boost_1_56_0


# Classic flags and whatnot for the builtin rules.
CPPFLAGS = -I$(googletest)/include -I$(boost) -I$(PPLME_SRC_DIR)
CXXFLAGS = -m64 -std=c++11 -Wall -Wextra -Werror -pedantic-errors -g -O2
LDFLAGS = -g -O2
LDLIBS = -lpthread
GTEST_LDLIBS = $(googletest)/make/gtest_main.a


# Stuffs related to the library/binary at hand.
src = $(wildcard *.cc)
objs = $(src:.cc=.o)


ifneq ($(filter lib%,$(component)),)
# Build that library!
lib = $(patsubst lib%,lib%.a,$(component))
.PHONY:		$(component)
$(component):	$(lib)
$(lib):		$(objs)
		$(AR) $(ARFLAGS) $@ $^
else
# Build that binary!
$(component):	$(objs)
		$(CXX) $(LDFLAGS) $(LDLIBS) $^ $(ALL_PPLME_LIBS) -o $@
endif


# Stuffs related to testing the library at hand.
tests_dir = $(component)_tests
tests_src = $(wildcard $(tests_dir)/*.cc)
tests_objs = $(tests_src:.cc=.o)
tests_bin = $(tests_dir)/$(component)_tests


# Build those tests!
$(tests_bin):	$(tests_objs) $(lib)
		$(CXX) $(LDFLAGS) $(LDLIBS) $(GTEST_LDLIBS) \
			$^ $(ALL_PPLME_LIBS) -o $@


# Run those tests!
.PHONY:	test
test:	$(tests_bin)
	$(tests_bin)


# Clean all the things!
.PHONY:	clean
clean:
	-rm $(tests_bin) $(tests_objs) $(lib) $(component) $(objs) 2>/dev/null
