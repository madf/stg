###############################################################################
# $Id: Makefile,v 1.9 2008/12/04 17:09:40 faust Exp $
###############################################################################

include ../../../../../Makefile.conf

PROG = mod_conf_sg-ng.so

SRCS = ./stgconfig.cpp \
       ./creator.cpp \
       ./main_thread.cpp \
       ./config_thread.cpp \
       ./root_parser.cpp \
       ./parser_info.cpp \
       ./parser_getuser.cpp \
       ./parser_getusers.cpp

LIBS += -lexpat \
	-lboost_thread-mt

STGLIBS = -lstg_common -lstg_logger -lcrypto

include ../../Makefile.in
