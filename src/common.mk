# Makefile that does the bulk of the business of building pplMe.


# Environment.
SHELL = /bin/bash


# Rely upon the fact that this makefile is in the root src directory.
PPLME_SRC_DIR := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))


# Various libraries that we rely upon.
googletest = $(THIRDPARTY)/gtest-1.7.0
boost = $(THIRDPARTY)/boost_1_56_0
protocol_buffers = $(THIRDPARTY)/protobuf-2.6.0
glog = $(THIRDPARTY)/glog-0.3.3
gflags = $(THIRDPARTY)/gflags-2.1.1


# Classic flags and whatnot for the builtin rules.
CPPFLAGS =  \
	-I$(googletest)/include  \
	-I$(boost)  \
        -isystem $(protocol_buffers)/src  \
	-I$(glog)/src  \
	-I$(gflags)/include  \
	-I$(PPLME_SRC_DIR)
CXXFLAGS = -m64 -std=c++11 -Wall -Wextra -Werror -pedantic-errors -g -O2
LDFLAGS = -g -O2  \
	-L$(boost)/stage/lib  \
	-L$(protocol_buffers)/src/.libs  \
	-Wl,-rpath $(realpath $(protocol_buffers)/src/.libs)  \
	-L$(glog)/.libs  \
	-L$(gflags)/lib  \
	-L$(googletest)/lib/.libs  \
	-Wl,-rpath $(realpath $(googletest)/lib/.libs)
LDLIBS = -lboost_system -lprotobuf -lglog -lgflags -lpthread
GTEST_LDLIBS = -lgtest -lgtest_main


# Stuffs related to the library/binary at hand.
src = $(wildcard *.cc) $(wildcard detail/*.cc)
protos = $(wildcard *.proto)
objs = $(protos:.proto=.pb.o) $(src:.cc=.o)


ifneq ($(filter lib%,$(component)),)
# Build that library!
lib = $(patsubst lib%,lib%.a,$(component))
.PHONY:		$(component)
$(component):	$(lib)
$(lib):		$(objs)
		$(AR) $(ARFLAGS) $@ $^
else
# Build that binary!
$(component):	$(objs) $(ALL_PPLME_LIBS)
		$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@
endif


# Stuffs related to testing the library at hand.
tests_dir = $(component)_tests
tests_src = $(wildcard $(tests_dir)/*.cc)
tests_objs = $(tests_src:.cc=.o)
tests_bin = $(tests_dir)/$(component)_tests


# Build those tests!
$(tests_bin):	$(tests_objs) $(lib)
		$(CXX) $(LDFLAGS) $(LDLIBS) $^ $(ALL_PPLME_LIBS) -o $@  \
			$(GTEST_LDLIBS) $(LDLIBS)


# Run those tests!
.PHONY:	test
test:	$(tests_bin)
	$(tests_bin)


# Clean all the things!
.PHONY:	clean
clean:
	-rm $(tests_bin) $(tests_objs) $(lib) $(component) $(objs) 2>/dev/null


# How to build .proto.
PROTOC = $(protocol_buffers)/src/protoc
%.pb.cc:	%.proto
		$(PROTOC) -I=. --cpp_out=. $<
%.pb.h:		%.proto
		$(PROTOC) -I=. --cpp_out=. $<
