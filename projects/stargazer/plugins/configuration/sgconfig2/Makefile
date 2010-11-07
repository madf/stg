###############################################################################
# $Id: Makefile,v 1.9 2008/12/04 17:09:40 faust Exp $
###############################################################################

include ../../../../../Makefile.conf

PROG = mod_conf_sg2.so

SRCS = ./stgconfig.cpp \
       ./rsconf.cpp \
       ./configproto.cpp \
       ./parser.cpp \
       ./parser_tariff.cpp \
       ./parser_admin.cpp

LIBS += -lexpat \
	$(LIB_THREAD)

STGLIBS = -lstg_common -lstg_logger

include ../../Makefile.in

