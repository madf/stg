include ../../../../../Makefile.conf

PROG = mod_cap_nfqueue.so

SRCS = ./nfqueue.cpp

LIBS += $(NFQ_LDFLAGS) $(NETLINK_LDFLAGS) $(LIB_THREAD)

STGLIBS = common \
          logger

include ../../Makefile.in

