###############################################################################
# $Id: Makefile,v 1.5 2008/12/04 17:13:14 faust Exp $
###############################################################################

LIB_NAME = stgpinger

SRCS = pinger.cpp

INCS = pinger.h

LIBS = $(LIB_THREAD)

LIB_INCS = -I ../common.lib/include

include ../Makefile.in

test: all
	g++ -c test.cpp
	g++ -o test test.o ./libstgpinger.a -lpthread
