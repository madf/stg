###############################################################################
# $Id: Makefile,v 1.3 2008/05/11 09:30:43 nobunaga Exp $
###############################################################################

include ../../Makefile.conf

PROG = sgconf

SRCS = ./main.cpp \
       ./parser.cpp

LIBS =  -lconffiles \
        -lstg_crypto \
        -lstg_common \
        -lsrvconf

ifeq ($(OS),linux)
LIBS += -lexpat \
        -lpthread \
        -ldl
endif

ifeq ($(OS),bsd)
LIBS += -lexpat \
        -lc_r \
        -lc
endif

ifeq ($(OS),bsd5)
LIBS += -lexpat \
        -lc_r \
        -lc
endif

SEARCH_DIRS = -I $(DIR_INCLUDE)

OBJS = $(notdir $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS))))

CC = g++

CFLAGS += -Wall
LDFLAGS += -Wl,-E -L$(DIR_LIB) -Wl,-rpath,$(PREFIX)/usr/lib/stg

vpath %.so $(DIR_LIB)

.PHONY: all clean distclean libs install uninstall install-bin install-data uninstall-bin uninstall-data
all: libs $(PROG) ../../Makefile.conf

libs:
	$(MAKE) -C $(DIR_LIBSRC)

$(PROG): $(OBJS) $(LIBS)
	$(CC) $^ $(LDFLAGS) -o $(PROG)

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
	install -m $(BIN_MODE) -o $(OWNER) -s $(PROG) $(PREFIX)/usr/sbin/$(PROG)
	$(MAKE) -C $(DIR_LIBSRC) install

install-data:
	# Install etc
	install -m $(DATA_MODE) -o $(OWNER) -D $(ETC_DIR)/sgconf.conf $(PREFIX)/etc/stargazer/sgconf.conf

uninstall: uninstall-bin uninstall-data

uninstall-bin:
	rm -f $(PREFIX)/usr/sbin/$(PROG)

uninstall-data:
	# Uninstall etc
	rm -f $(PREFIX)/etc/stargazer/sgconf.conf


ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),uninstall)
include deps
endif
endif
endif

deps:	$(SRCS) ../../Makefile.conf
	$(MAKE) -C $(DIR_LIBSRC) includes
	@>deps ;\
	for file in $(SRCS); do\
	  echo "`$(CC) $(CFLAGS) $(SEARCH_DIRS) -MM $$file` Makefile" >> deps ;\
	  echo -e '\t$$(CC) -c $$< $(CFLAGS) $(SEARCH_DIRS) $(DEFS)' >> deps ;\
	done


