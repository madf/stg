###############################################################################
# $Id: Makefile,v 1.9 2010/08/18 07:47:03 faust Exp $
###############################################################################

LIB_NAME = stgsrvconf

STGLIBS = -lstgcommon \
          -lstgcrypto
LIBS = -lexpat

SRCS =  netunit.cpp \
        parser.cpp \
        parser_auth_by.cpp \
        parser_server_info.cpp \
        parser_check_user.cpp \
        parser_get_user.cpp \
        servconf.cpp

INCS = servconf.h \
       netunit.h

LIB_INCS = -I ../common.lib/include \
           -I ../crypto.lib/include

include ../Makefile.in
