###############################################################################
# $Id: Makefile,v 1.1 2008/07/05 12:35:53 faust Exp $
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

SRCS = ./userstat.cpp

LIBS += -lstg_common \
	-lstg_crypto

include ../../Makefile.in

