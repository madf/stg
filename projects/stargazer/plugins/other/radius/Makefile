###############################################################################
# $Id: Makefile,v 1.3 2010/04/26 12:44:42 faust Exp $
###############################################################################

include ../../../../../Makefile.conf

LIBS += $(LIB_THREAD) -lyajl

PROG = mod_radius.so

SRCS = radius.cpp \
       config.cpp \
       conn.cpp

STGLIBS = common \
	  crypto \
	  logger \
	  scriptexecuter \
	  json

include ../../Makefile.in

