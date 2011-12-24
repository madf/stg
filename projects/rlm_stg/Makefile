###############################################################################
# $Id: Makefile,v 1.5 2009/03/03 15:49:34 faust Exp $
###############################################################################

include ../../Makefile.conf

LIB_NAME = rlm_stg

PROG = $(LIB_NAME).so

SRCS = ./rlm_stg.cpp \
       ./stg_client.cpp

STGLIBS = crypto \
	  common
          
STGLIBS_INCS = $(addprefix -I ../../stglibs/,$(addsuffix .lib/include,$(STGLIBS)))
STGLIBS_LIBS = $(addprefix -L ../../stglibs/,$(addsuffix .lib,$(STGLIBS)))

LIBS += $(addprefix -lstg,$(STGLIBS)) $(LIB_THREAD)

ifeq ($(OS),linux)
LIBS += -ldl
else
LIBS += -lintl \
        -lc \
	-liconv
endif

SEARCH_DIRS = -I ../../include

OBJS = $(notdir $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS))))

CXXFLAGS += -fPIC $(DEFS) $(STGLIBS_INCS) $(SEARCH_DIRS)
CFLAGS += $(DEFS) $(STGLIBS_INCS) $(SEARCH_DIRS)
LDFLAGS += -shared -Wl,-rpath,$(PREFIX)/usr/lib/stg -Wl,-E $(STGLIBS_LIBS)

.PHONY: all clean distclean libs install uninstall install-bin uninstall-bin
all: libs $(PROG) ../../Makefile.conf

libs:
	$(MAKE) -C $(DIR_LIBSRC)

$(PROG): $(OBJS)
	$(CXX) $^ $(LDFLAGS) -o $(PROG) $(LIBS)

clean:
	rm -f deps $(PROG) *.o tags *.*~ .OS
	rm -f .OS
	rm -f core*
	$(MAKE) -C $(DIR_LIBSRC) clean

distclean: clean
	rm -f ../../Makefile.conf

install: install-bin

install-bin:
	install -m $(BIN_MODE) -o $(OWNER) -s $(PROG) $(PREFIX)/usr/lib/$(PROG)
	$(MAKE) -C $(DIR_LIBSRC) install

uninstall: uninstall-bin

uninstall-bin:
	rm -f $(PREFIX)/usr/lib/$(PROG)

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
	  printf '%b\n' '\t$$(CC) -c $$< $(CXXFLAGS)' >> deps ;\
	done
