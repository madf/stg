###############################################################################
# $Id: Makefile,v 1.10 2009/08/03 10:25:40 faust Exp $
###############################################################################

include ../../../../../Makefile.conf

PROG = mod_conf_rpc.so

SRCS = ./rpcconfig.cpp \
       ./user_helper.cpp \
       ./tariff_helper.cpp \
       ./info_methods.cpp \
       ./users_methods.cpp \
       ./tariffs_methods.cpp \
       ./admins_methods.cpp \
       ./messages_methods.cpp

LIBS += $(XMLRPC_LDFLAGS) \
	$(LIB_THREAD)

ifneq ($(OS),linux)
LIBS += -liconv
endif

STGLIBS = common \
	  logger \
	  scriptexecuter

include ../../Makefile.in

