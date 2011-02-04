###############################################################################
# $Id: Makefile,v 1.18 2010/02/11 12:22:01 faust Exp $
###############################################################################

include ../../Makefile.conf

PROG = rscriptd

SRCS = ./main.cpp \
       ./listener.cpp \
       ./pidfile.cpp

STGLIBS =  -lstg_logger \
           -lstg_common \
	   -lstg_crypto \
           -lscript_executer \
           -lconffiles

LIBS += $(LIB_THREAD)

#ifeq ($(OS),linux)
#LIBS += -ldl
#else
#LIBS += -lc
#endif

SEARCH_DIRS = -I $(DIR_INCLUDE)

OBJS = $(notdir $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS))))

CXXFLAGS += -Wall
LDFLAGS += -Wl,-E -L$(DIR_LIB) -Wl,-rpath,$(PREFIX)/usr/lib/stg -Wl,-rpath-link,$(DIR_LIB)

vpath %.so $(DIR_LIB)

.PHONY: all clean distclean libs install uninstall
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

install: install-bin install-data

install-bin:
	install -m $(BIN_MODE) -o $(OWNER) -s $(PROG) $(PREFIX)/usr/sbin/$(PROG)
	$(MAKE) -C $(DIR_LIBSRC) install

install-data:
	# Install etc
	mkdir -m $(DIR_MODE) -p $(PREFIX)/etc/stargazer
	install -m $(DATA_MODE) -o $(OWNER) ./rscriptd.conf $(PREFIX)/etc/stargazer/rscriptd.conf

uninstall: uninstall-bin uninstall-data

uninstall-bin:
	rm -f $(PREFIX)/usr/sbin/$(PROG)

uninstall-data:
	# Uninstall etc
	rm -f $(PREFIX)/etc/stragazer/rscriptd.conf


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


