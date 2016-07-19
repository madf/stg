###############################################################################
# $Id: Makefile,v 1.9 2008/12/04 17:15:53 faust Exp $
###############################################################################

include ../../../../../Makefile.conf

DEFS += -DIBPP_LINUX

PROG = mod_store_firebird.so

SRCS =	./firebird_store.cpp \
	./firebird_store_admins.cpp \
	./firebird_store_corporations.cpp \
	./firebird_store_messages.cpp \
	./firebird_store_services.cpp \
	./firebird_store_tariffs.cpp \
	./firebird_store_users.cpp \
	./firebird_store_utils.cpp

STGLIBS = ibpp \
	  common \
	  logger \
	  crypto

CXXFLAGS +=  $(FB_CFLAGS)
LDFLAGS += $(FB_LDFLAGS)

include ../../Makefile.in

