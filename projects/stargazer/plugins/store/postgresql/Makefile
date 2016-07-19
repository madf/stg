###############################################################################
# $Id: Makefile,v 1.4 2010/04/26 12:44:42 faust Exp $
###############################################################################

include ../../../../../Makefile.conf

PROG = mod_store_postgresql.so

SRCS =	./postgresql_store.cpp \
	./postgresql_store_admins.cpp \
	./postgresql_store_corporations.cpp \
	./postgresql_store_messages.cpp \
	./postgresql_store_services.cpp \
	./postgresql_store_tariffs.cpp \
	./postgresql_store_users.cpp \
	./postgresql_store_utils.cpp

STGLIBS = common \
	  crypto \
	  logger

CXXFLAGS +=  $(PG_CFLAGS)
LDFLAGS += $(PG_LDFLAGS)

LIBS += -lpq

include ../../Makefile.in

