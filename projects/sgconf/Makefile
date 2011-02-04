###############################################################################
# $Id: Makefile,v 1.21 2010/02/11 12:34:14 faust Exp $
###############################################################################

include ../../Makefile.conf

PROG = sgconf

SRCS = ./main.cpp \
       ./common_sg.cpp

STGLIBS =  -lconffiles \
           -lstg_common \
	   -lstg_crypto \
           -lsrvconf

LIBS += -lexpat \
	$(LIB_THREAD)

ifeq ($(OS),linux)
LIBS += -ldl
else
LIBS += -lc \
	-liconv
endif

SEARCH_DIRS = -I $(DIR_INCLUDE)

ifeq ($(OS),bsd)
SEARCH_DIRS += -I/usr/local/include
CXXFLAGS += -DHAVE_DECL_GETOPT=1
endif

ifeq ($(OS),bsd5)
SEARCH_DIRS += -I/usr/local/include
CXXFLAGS += -DHAVE_DECL_GETOPT=1
endif

OBJS = $(notdir $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS))))

CXXFLAGS += -Wall
LDFLAGS += -Wl,-E -L$(DIR_LIB) -Wl,-rpath,$(PREFIX)/usr/lib/stg -Wl,-rpath-link,$(DIR_LIB)

vpath %.so $(DIR_LIB)

.PHONY: all clean distclean libs install uninstall install-bin install-data uninstall-bin uninstall-data
all: libs $(PROG) ../../Makefile.conf

libs:
	$(MAKE) -C $(DIR_LIBSRC)

$(PROG): $(OBJS) $(STGLIBS)
	$(CC) $^ $(LDFLAGS) $(LIBS) -o $(PROG)

clean:
	rm -f deps $(PROG) *.o tags *.*~ .OS
	rm -f .OS
	rm -f .store
	rm -f .db.sql
	rm -f core*
	$(MAKE) -C $(DIR_LIBSRC) clean

distclean: clean
	rm -f ../../Makefile.conf

install: install-bin

install-bin:
	install -m $(BIN_MODE) -o $(OWNER) -s $(PROG) $(PREFIX)/usr/sbin/$(PROG)
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
	$(MAKE) -C $(DIR_LIBSRC) includes
	@>deps ;\
	for file in $(SRCS); do\
	  echo "`$(CC) $(CXXFLAGS) $(SEARCH_DIRS) -MM $$file` Makefile" >> deps ;\
	  echo -e '\t$$(CC) -c $$< $(CXXFLAGS) $(SEARCH_DIRS) $(DEFS)' >> deps ;\
	done


