###############################################################################
# $Id: Makefile,v 1.11 2008/12/04 17:21:14 faust Exp $
###############################################################################

include ../../../../../Makefile.conf

LIBS += $(LIB_THREAD)

PROG = mod_ping.so

SRCS = ./ping.cpp

STGLIBS = pinger \
	  common \
	  locker \
	  logger \
	  scriptexecuter

include ../../Makefile.in

