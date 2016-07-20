###############################################################################
# $Id: Makefile,v 1.6 2010/03/25 10:35:55 faust Exp $
###############################################################################

include ../../../../../Makefile.conf

PROG = mod_store_mysql.so

SRCS = ./mysql_store.cpp

STGLIBS = common \
	  crypto \
	  logger

CXXFLAGS += $(MYSQL_CFLAGS)
LIBS += $(MYSQL_LDFLAGS)

include ../../Makefile.in

