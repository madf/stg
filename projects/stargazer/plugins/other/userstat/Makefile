###############################################################################
# $Id: Makefile,v 1.4 2007/09/26 14:07:28 faust Exp $
###############################################################################

include ../../../../../Makefile.conf

LIBS += -lexpat

ifeq ($(OS),linux)
LIBS += -lpthread
endif

ifeq ($(OS),bsd)
LIBS += -lc_r
endif

ifeq ($(OS),bsd5)
LIBS += -lc_r
endif

PROG = mod_userstat.so

SRCS = ./userstat.cpp \
       ./datathread.cpp

LIBS += -lstgcommon \
	-lstgcrypto

include ../../Makefile.in

