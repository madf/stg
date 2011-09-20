###############################################################################
# $Id: Makefile,v 1.11 2010/03/04 10:47:45 faust Exp $
###############################################################################

include ../../../../../Makefile.conf

PROG = mod_conf_sg.so

SRCS = ./stgconfig.cpp \
       ./rsconf.cpp \
       ./configproto.cpp \
       ./parser.cpp \
       ./parser_tariff.cpp \
       ./parser_admin.cpp

LIBS += -lexpat \
	$(LIB_THREAD)

STGLIBS = common \
	  locker \
	  logger \
	  crypto \
	  scriptexecuter

include ../../Makefile.in

