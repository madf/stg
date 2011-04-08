###############################################################################
# $Id: Makefile,v 1.18 2009/08/06 12:50:55 faust Exp $
###############################################################################

include ../../Makefile.conf

PROG = sgauth

SRCS = ./main.cpp \
       ./settings_impl.cpp \
       ./web.cpp

STGLIBS =  -lstgconffiles \
           -lstgcrypto \
           -lstgcommon \
           -lstgia

LIBS += $(LIB_THREAD)

ifeq ($(OS),linux)
else
LIBS += -lintl \
        -lc
endif

SEARCH_DIRS = -I $(DIR_INCLUDE)

OBJS = $(notdir $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS))))

CXXFLAGS += -Wall
LDFLAGS += -Wl,-E -L$(DIR_LIB) -Wl,-rpath,$(PREFIX)/usr/lib/stg -Wl,-rpath-link,$(DIR_LIB)

vpath %.so $(DIR_LIB)

.PHONY: all clean distclean libs install uninstall install-bin install-data uninstall-bin uninstall-data
all: libs $(PROG) ../../Makefile.conf

libs:
	$(MAKE) -C $(DIR_LIBSRC)

$(PROG): $(OBJS) $(STGLIBS) 
	$(CC) $^ $(LDFLAGS) -o $(PROG) $(LIBS)

clean:
	rm -f deps $(PROG) *.o tags *.*~ .OS
	rm -f .OS
	rm -f .store
	rm -f .db.sql
	rm -f core*
	rm -f css.h
	$(MAKE) -C $(DIR_LIBSRC) clean

distclean: clean
	rm -f ../../Makefile.conf

install: install-bin install-data

install-bin:
	install -m $(BIN_MODE) -o $(OWNER) -s $(PROG) $(PREFIX)/usr/sbin/$(PROG)
	$(MAKE) -C $(DIR_LIBSRC) install

install-data:
	# Install etc
	mkdir -m $(DIR_MODE) -p $(PREFIX)/etc
	install -m $(DATA_MODE) -o $(OWNER) ./sgauth.conf $(PREFIX)/etc/sgauth.conf

uninstall: uninstall-bin uninstall-data

uninstall-bin:
	rm -f $(PREFIX)/usr/sbin/$(PROG)

uninstall-data:
	# Uninstall etc
	rm -f $(PREFIX)/etc/sgauth.conf


ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),uninstall)
-include deps
endif
endif
endif

deps:	$(SRCS) ../../Makefile.conf sgauth.css
	$(MAKE) -C $(DIR_LIBSRC) includes
	@>deps ;\
	./make_css.sh
	for file in $(SRCS); do\
	  echo "`$(CC) $(CXXFLAGS) $(SEARCH_DIRS) -MM $$file` Makefile" >> deps ;\
	  echo -e '\t$$(CC) -c $$< $(CXXFLAGS) $(SEARCH_DIRS) $(DEFS)' >> deps ;\
	done
