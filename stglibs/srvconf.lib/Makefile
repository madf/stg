###############################################################################
# $Id: Makefile,v 1.9 2010/08/18 07:47:03 faust Exp $
###############################################################################

LIB_NAME = stgsrvconf

STGLIBS = -lstgcommon \
          -lstgcrypto
LIBS = -lexpat

SRCS = 	netunit.cpp \
        parser.cpp \
        servconf.cpp

INCS = servconf.h \
       netunit.h

LIB_INCS = -I ../common.lib/include \
	   -I ../crypto.lib/include

include ../Makefile.in
