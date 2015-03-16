###############################################################################
# $Id: Makefile,v 1.12 2009/03/03 15:49:34 faust Exp $
###############################################################################

include ../../Makefile.conf

PROG = sgconv

SRCS = ./main.cpp \
       ./settings_impl.cpp

STGLIBS = dotconfpp \
          conffiles \
          logger \
          crypto \
          common

STGLIBS_INCS = $(addprefix -I ../../stglibs/,$(addsuffix .lib/include,$(STGLIBS)))
STGLIBS_LIBS = $(addprefix -L ../../stglibs/,$(addsuffix .lib,$(STGLIBS)))

LIBS += $(addprefix -lstg,$(STGLIBS)) $(LIB_THREAD)

ifeq ($(OS),linux)
LIBS += -ldl
else
LIBS += -lc -liconv
endif

SEARCH_DIRS = -I ../../include

OBJS = $(notdir $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS))))

CXXFLAGS += $(DEFS) $(STGLIBS_INCS) $(SEARCH_DIRS)
CFLAGS += $(DEFS) $(STGLIBS_INCS) $(SEARCH_DIRS)

ifneq ($(OS),darwin)
LDFLAGS += -Wl,-E
endif

LDFLAGS += $(STGLIBS_LIBS)

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

install: install-bin

install-bin:
ifeq ($(DEBUG), yes)
	mkdir -m $(DIR_MODE) -p $(PREFIX)/usr/bin
	install -m $(BIN_MODE) -o $(OWNER) $(PROG) $(PREFIX)/usr/bin/$(PROG)
else
	mkdir -m $(DIR_MODE) -p $(PREFIX)/usr/bin
	install -m $(BIN_MODE) -o $(OWNER) -s $(PROG) $(PREFIX)/usr/bin/$(PROG)
endif
	$(MAKE) -C $(DIR_PLUGINS) install

uninstall: uninstall-bin

uninstall-bin:
	rm -f $(PREFIX)/usr/bin/$(PROG)

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
