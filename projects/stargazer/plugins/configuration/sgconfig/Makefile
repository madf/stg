###############################################################################
# $Id: Makefile,v 1.11 2010/03/04 10:47:45 faust Exp $
###############################################################################

include ../../../../../Makefile.conf

PROG = mod_conf_sg.so

SRCS = ./stgconfig.cpp \
       ./conn.cpp \
       ./configproto.cpp \
       ./parser.cpp \
       ./parser_tariffs.cpp \
       ./parser_admins.cpp \
       ./parser_users.cpp \
       ./parser_message.cpp \
       ./parser_auth_by.cpp \
       ./parser_server_info.cpp

LIBS += -lexpat \
	$(LIB_THREAD)

STGLIBS = common \
	  logger \
	  crypto \
	  scriptexecuter

include ../../Makefile.in

