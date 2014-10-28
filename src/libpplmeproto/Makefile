# Makefile for pplMe's libpplmeproto.

component = libpplmeproto

include ../common.mk


# Because the make rules automatically go from .proto -> .pb.o, we need
# to provide some hints to make sure they're processed in a workable order.
person.pb.o:	date.pb.h geo_position.pb.h uuid.pb.h
