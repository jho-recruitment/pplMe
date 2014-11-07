# Makefile for the libraries that pplMe depends upon.


# Environment.
SHELL = /bin/bash


# Stuff that you might want to override.
CMAKE = cmake
TAR = tar
UNZIP = unzip
WGET = wget
WGETFLAGS = -q


# All the 3rd Party libraries that pplMe currently requires.
libs = googletest boost protobuf gflags glog


# We need everything, obvs.
.PHONY:	all
all:	$(libs)


# Google Test.
googletest_dist = https://googletest.googlecode.com/files/gtest-1.7.0.zip
googletest_dir = gtest-1.7.0
googletest:
	cd $(googletest_dir) &&  \
		./configure &&  \
		$(MAKE)

# Boost.
boost_dist = http://downloads.sourceforge.net/project/boost/boost/1.56.0/boost_1_56_0.tar.gz
boost_dir = boost_1_56_0
boost:
	cd $(boost_dir) &&  \
		./bootstrap.sh &&  \
		./b2 --with-system link=static


# Protocol Buffers.
protobuf_dist = https://github.com/google/protobuf/releases/download/v2.6.0/protobuf-2.6.0.tar.gz
protobuf_dir = protobuf-2.6.0
protobuf:
	cd $(protobuf_dir) &&  \
		./configure &&  \
		$(MAKE)


# gflags
gflags_dist = https://github.com/schuhschuh/gflags/archive/v2.1.1.tar.gz
gflags_dir = gflags-2.1.1
gflags:
	cd $(gflags_dir) &&  \
		$(CMAKE) -DGFLAGS_NAMESPACE=google -G "Unix Makefiles" &&  \
		$(MAKE)


# Google glog.
glog_dist = https://google-glog.googlecode.com/files/glog-0.3.3.tar.gz
glog_dir = glog-0.3.3
glog:
	cd $(glog_dir) &&  \
		./configure  \
			--with-gflags=../$(gflags_dir) --disable-shared &&  \
		$(MAKE)


# GNU make magic for eliminating some of the repetepetetiveness.
define lib_rules
$(1):		$$($(1)_dir)

ifneq ($(filter %.zip,$($(1)_dist)),)
$(1)_archive := $($(1)_dir).zip

$$($(1)_dir):	$$($(1)_archive)
		$(UNZIP) $$<
endif
ifneq ($(filter %.tar.gz,$($(1)_dist)),)
$(1)_archive := $($(1)_dir).tar.gz

$$($(1)_dir):	$$($(1)_archive)
		$(TAR) xzf $$<
endif

$$($(1)_archive):
		$(WGET) $(WGETFLAGS) $$($(1)_dist) -O $$@

.PHONY:		clean-$(1)
clean-$(1):
		-rm -rf $$($(1)_dir)
		-rm $$($(1)_archive)
endef
$(foreach lib,$(libs),$(eval $(call lib_rules,$(lib))))


# Don't call this target, you'll probably regret it.
.PHONY:	clean
clean:	$(foreach lib,$(libs),clean-$(lib))
