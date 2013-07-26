###############################################################################
# $Id: Makefile,v 1.3 2008/05/11 09:30:43 nobunaga Exp $
###############################################################################

include ../../Makefile.conf

PROG = sgconf_xml

SRCS = ./main.cpp \
       ./parser.cpp

STGLIBS = conffiles \
          srvconf \
          crypto \
          common

STGLIBS_INCS = $(addprefix -I ../../stglibs/,$(addsuffix .lib/include,$(STGLIBS)))
STGLIBS_LIBS = $(addprefix -L ../../stglibs/,$(addsuffix .lib,$(STGLIBS)))

LIBS += $(addprefix -lstg,$(STGLIBS)) -lexpat $(LIB_THREAD)

ifeq ($(OS),linux)
else
LIBS += -lc \
	-liconv
endif

SEARCH_DIRS = -I ../../include

ifeq ($(OS),bsd)
SEARCH_DIRS += -I/usr/local/include
CXXFLAGS += -DHAVE_DECL_GETOPT=1
endif

ifeq ($(OS),bsd5)
SEARCH_DIRS += -I/usr/local/include
CXXFLAGS += -DHAVE_DECL_GETOPT=1
endif

OBJS = $(notdir $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS))))

CXXFLAGS += $(DEFS) $(STGLIBS_INCS) $(SEARCH_DIRS)
CFLAGS += $(DEFS) $(STGLIBS_INCS) $(SEARCH_DIRS)
LDFLAGS += -Wl,-E $(STGLIBS_LIBS)

.PHONY: all clean distclean libs install uninstall install-bin install-data uninstall-bin uninstall-data
all: libs $(PROG) ../../Makefile.conf

libs:
	$(MAKE) -C $(DIR_LIBSRC)

$(PROG): $(OBJS)
	$(CXX) $^ $(LIBS) $(LDFLAGS) -o $(PROG)

clean:
	rm -f deps $(PROG) *.o tags *.*~ .OS
	rm -f .OS
	rm -f .store
	rm -f .db.sql
	rm -f core*
	$(MAKE) -C $(DIR_LIBSRC) clean

distclean: clean
	rm -f ../../Makefile.conf

install: install-bin install-data

install-bin:
ifeq ($(DEBUG), yes)
	install -D -m $(BIN_MODE) -o $(OWNER) $(PROG) $(PREFIX)/usr/bin/$(PROG)
else
	install -D -m $(BIN_MODE) -o $(OWNER) -s $(PROG) $(PREFIX)/usr/bin/$(PROG)
endif
	$(MAKE) -C $(DIR_LIBSRC) install

uninstall: uninstall-bin uninstall-data

uninstall-bin:
	rm -f $(PREFIX)/usr/sbin/$(PROG)


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
