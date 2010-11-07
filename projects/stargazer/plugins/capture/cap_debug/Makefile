###############################################################################
# $Id: Makefile,v 1.10 2008/12/04 17:03:25 faust Exp $
###############################################################################

include ../../../../../Makefile.conf

PROG = mod_cap_debug.so

SRCS = ./debug_cap.cpp \
       ./checksum.c \
       ./icmp.c \
       ./ip.c \
       ./misc.c \
       ./packet.c \
       ./socket.c \
       ./tcp.c \
       ./udp.c

STGLIBS = -lstg_common

LIBS += $(LIB_THREAD)

include ../../Makefile.in

