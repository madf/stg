include ../../../../../Makefile.conf

LIBS += $(LIB_THREAD)

PROG = mod_smux.so

SRCS =  smux.cpp \
	sensors.cpp \
	handlers.cpp \
	utils.cpp \
	types.cpp

STGLIBS = -lstgcommon -lstgsmux

CXXFLAGS += -Iasn1

include ../../Makefile.in

.PHONY: asn1

asn1:
	$(MAKE) -C asn1 asn1
