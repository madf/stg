include ../../../../../Makefile.conf

LIBS += $(LIB_THREAD)

PROG = mod_smux.so

SRCS =  smux.cpp \
	sensors.cpp \
	tables.cpp \
	handlers.cpp \
	utils.cpp \
	types.cpp

STGLIBS = common \
	  smux \
	  logger \
	  scriptexecuter

CFLAGS += -I ../../../../../stglibs/smux.lib/include/stg
CXXFLAGS += -I ../../../../../stglibs/smux.lib/include/stg

include ../../Makefile.in
