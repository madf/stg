###############################################################################
# $Id: Makefile,v 1.5 2009/03/03 15:49:34 faust Exp $
###############################################################################

include ../../Makefile.conf

LIB_NAME = rlm_stg

PROG = $(LIB_NAME).so

SRCS = ./rlm_stg.c \
       ./iface.cpp \
       ./stg_client.cpp

STGLIBS = crypto \
          common

STGLIBS_INCS = $(addprefix -I ../../stglibs/,$(addsuffix .lib/include,$(STGLIBS)))
STGLIBS_LIBS = $(addprefix -L ../../stglibs/,$(addsuffix .lib,$(STGLIBS)))

LIBS += $(addprefix -lstg,$(STGLIBS)) $(LIB_THREAD) $(LIBICONV)

ifeq ($(OS),linux)
LIBS += -ldl
else
LIBS += -lintl \
        -lc
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
ifeq ($(DEBUG), yes)
ifeq ($(OS), linux)
	mkdir -m $(DIR_MODE) -p $(PREFIX)/usr/lib/freeradius
	install -m $(BIN_MODE) -o $(OWNER) $(PROG) $(PREFIX)/usr/lib/freeradius/$(PROG)
else
	mkdir -m $(DIR_MODE) -p $(PREFIX)/usr/lib
	install -m $(BIN_MODE) -o $(OWNER) $(PROG) $(PREFIX)/usr/lib/$(PROG)
endif
else
	mkdir -m $(DIR_MODE) -p $(PREFIX)/usr/lib/freeradius
	install -m $(BIN_MODE) -o $(OWNER) -s $(PROG) $(PREFIX)/usr/lib/freeradius/$(PROG)
ifeq ($(OS), linux)
	mkdir -m $(DIR_MODE) -p $(PREFIX)/usr/lib/freeradius
	install -m $(BIN_MODE) -o $(OWNER) -s $(PROG) $(PREFIX)/usr/lib/freeradius/$(PROG)
else
	mkdir -m $(DIR_MODE) -p $(PREFIX)/usr/lib
	install -m $(BIN_MODE) -o $(OWNER) -s $(PROG) $(PREFIX)/usr/lib/$(PROG)
endif
endif
	$(MAKE) -C $(DIR_LIBSRC) install

uninstall: uninstall-bin

uninstall-bin:
ifeq ($(OS), linux)
	rm -f $(PREFIX)/usr/lib/freeradius/$(PROG)
else
	rm -f $(PREFIX)/usr/lib/$(PROG)
endif

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
	  echo "$$file" | grep ".c$$" > /dev/null;\
	  if [ $$? -eq 0 ];\
	  then\
	    echo "`$(CC) $(CFLAGS) -MM $$file` Makefile" >> deps ;\
	    printf '%b\n' '\t$$(CC) $(CFLAGS) -c $$<' >> deps ;\
	  else\
	    echo "`$(CXX) $(CXXFLAGS) -MM $$file` Makefile" >> deps ;\
	    printf '%b\n' '\t$$(CXX) $(CXXFLAGS) -c $$<' >> deps ;\
	  fi;\
	done
