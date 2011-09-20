###############################################################################
# $Id: Makefile,v 1.12 2009/03/03 15:49:34 faust Exp $
###############################################################################

include ../../Makefile.conf

PROG = convertor

SRCS = ./main.cpp \
       ./settings_impl.cpp

STGLIBS =  logger \
           common \
           dotconfpp \
	   crypto \
	   conffiles

STGLIBS_INCS = $(addprefix -I ../../stglibs/,$(addsuffix .lib/include,$(STGLIBS)))
STGLIBS_LIBS = $(addprefix -L ../../stglibs/,$(addsuffix .lib,$(STGLIBS)))

LIBS += $(LIB_THREAD) $(addprefix -lstg,$(STGLIBS))

ifeq ($(OS),linux)
LIBS += -ldl
else
LIBS += -lc -liconv
endif

SEARCH_DIRS = -I ../../include

OBJS = $(notdir $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS))))

CXXFLAGS += $(DEFS) $(STGLIBS_INCS) $(SEARCH_DIRS)
CFLAGS += $(DEFS) $(STGLIBS_INCS) $(SEARCH_DIRS)
LDFLAGS += -Wl,-E $(STGLIBS_LIBS)

.PHONY: all clean distclean libs plugins install uninstall
all: libs plugins $(PROG) ../../Makefile.conf

libs:
	$(MAKE) -C $(DIR_LIBSRC)

plugins: libs 
	$(MAKE) -C $(DIR_PLUGINS)

$(PROG): $(OBJS)
	$(CXX) $^ $(LDFLAGS) $(LIBS) -o $(PROG)

clean:
	rm -f deps $(PROG) *.o tags *.*~ .OS
	rm -f .OS
	rm -f .store
	rm -f .db.sql
	rm -f core*
	$(MAKE) -C $(DIR_LIBSRC) clean
	$(MAKE) -C $(DIR_PLUGINS) clean

distclean: clean
	rm -f ../../Makefile.conf

ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),uninstall)
-include deps
endif
endif
endif

deps:	$(SRCS) ../../Makefile.conf
	$(MAKE) -C $(DIR_LIBSRC)
	@>deps ;\
	for file in $(SRCS); do\
	  echo "`$(CC) $(CXXFLAGS) -MM $$file` Makefile" >> deps ;\
	  echo -e '\t$$(CC) -c $$< $(CXXFLAGS)' >> deps ;\
	done


