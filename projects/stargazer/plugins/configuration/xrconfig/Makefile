###############################################################################
# $Id: Makefile,v 1.3 2009/03/03 15:49:35 faust Exp $
###############################################################################
DEFS = -DLINUX

ifeq ($(OS),bsd)
DEFS = -DFREE_BSD
endif

ifeq ($(OS),bsd5)
DEFS = -DFREE_BSD5
endif


DIR_INCLUDE = ../../../../../include
DIR_LIB = ../../../../../lib

PROG = mod_conf_xr.so

SRCS = ./xrconfig.cpp


LIBS = $(LIB_THREAD)

SEARCH_DIRS = -I $(DIR_INCLUDE)

OBJS = $(notdir $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS))))

CXXFLAGS = -g3 -Wall -fPIC
LDFLAGS = -g3 -shared


vpath %.a $(DIR_LIB)


all: $(PROG)

$(PROG): $(OBJS) $(LIBS)
	$(CC) $^ $(LDFLAGS) -o $(PROG)

clean:
	rm -f deps $(PROG) *.o tags *.*~


install:
	echo TODO

uninstall:
	echo TODO

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),uninstall)
include deps
endif
endif

deps:	$(SRCS)
	@>deps ;\
	for file in $(SRCS); do\
	  echo "`$(CC) $(CXXFLAGS) $(DEFS) $(SEARCH_DIRS) -MM $$file` Makefile" >> deps ;\
	  echo -e '\t$$(CC) -c $$< $(CXXFLAGS) $(SEARCH_DIRS) $(DEFS)' >> deps ;\
	done


