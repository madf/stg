###############################################################################
# $Id: Makefile,v 1.9 2010/01/21 13:02:12 faust Exp $
###############################################################################

include ../../Makefile.conf

LIB_NAME = stgcommon

SRCS = common.cpp \
	strptime.cpp

INCS = common.h

ifneq ($(OS),linux)
LIBS += -liconv
endif

include ../Makefile.in
