###############################################################################
# $Id: Makefile,v 1.3 2010/04/26 12:44:42 faust Exp $
###############################################################################

include ../../../../../Makefile.conf

LIBS += $(LIB_THREAD)

PROG = mod_radius.so

SRCS = radius.cpp \
       config.cpp

STGLIBS = common \
	  crypto \
	  logger \
	  scriptexecuter

include ../../Makefile.in

