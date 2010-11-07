###############################################################################
# $Id: Makefile,v 1.9 2010/08/18 07:47:03 faust Exp $
###############################################################################

LIB_NAME = srvconf
PROG = lib$(LIB_NAME)

STGLIBS = -lstg_common \
          -lstg_crypto
LIBS = -lexpat

SRCS = 	netunit.cpp \
        parser.cpp \
        servconf.cpp

INCS = servconf.h \
       netunit.h

include ../Makefile.in
