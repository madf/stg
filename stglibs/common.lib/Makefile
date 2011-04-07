###############################################################################
# $Id: Makefile,v 1.9 2010/01/21 13:02:12 faust Exp $
###############################################################################

include ../../Makefile.conf

LIB_NAME = stg_common
PROG = lib$(LIB_NAME)

SRCS = common.cpp \
	stg_strptime.cpp

INCS = common.h

ifneq ($(OS),linux)
LIBS += -liconv
endif

include ../Makefile.in
