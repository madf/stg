###############################################################################
# $Id: Makefile,v 1.9 2010/08/18 07:47:03 faust Exp $
###############################################################################

LIB_NAME = stgsrvconf

STGLIBS = -lstgcommon \
          -lstgcrypto
LIBS = -lexpat

SRCS =  parsers/property.cpp \
        parsers/auth_by.cpp \
        parsers/server_info.cpp \
        parsers/check_user.cpp \
        parsers/get_user.cpp \
        parsers/get_users.cpp \
        parsers/chg_user.cpp \
        parsers/send_message.cpp \
        netunit.cpp \
        servconf.cpp


INCS = servconf.h \
       netunit.h

LIB_INCS = -I ../common.lib/include \
           -I ../crypto.lib/include

include ../Makefile.in
