###############################################################################
# $Id: Makefile,v 1.18 2010/02/11 12:22:01 faust Exp $
###############################################################################

include ../../Makefile.conf

PROG = rscriptd

SRCS = ./main.cpp \
       ./listener.cpp \
       ./pidfile.cpp

STGLIBS = scriptexecuter \
          conffiles \
	  logger \
	  crypto \
          common

STGLIBS_INCS = $(addprefix -I ../../stglibs/,$(addsuffix .lib/include,$(STGLIBS)))
STGLIBS_LIBS = $(addprefix -L ../../stglibs/,$(addsuffix .lib,$(STGLIBS)))

LIBS += $(addprefix -lstg,$(STGLIBS)) $(LIB_THREAD)

ifneq ($(OS),linux)
LIBS += -liconv
endif

SEARCH_DIRS = -I ../../include

OBJS = $(notdir $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS))))

CXXFLAGS += $(DEFS) $(STGLIBS_INCS) $(SEARCH_DIRS)
CFLAGS += $(DEFS) $(STGLIBS_INCS) $(SEARCH_DIRS)

ifneq ($(OS),darwin)
LDFLAGS += -Wl,-E
endif

LDFLAGS += $(STGLIBS_LIBS)

.PHONY: all clean distclean libs install uninstall
all: libs $(PROG) ../../Makefile.conf

libs:
	$(MAKE) -C $(DIR_LIBSRC)

$(PROG): $(OBJS)
	$(CXX) $^ $(LDFLAGS) $(LIBS) -o $(PROG)

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
	install -D -m $(BIN_MODE) -o $(OWNER) $(PROG) $(PREFIX)/usr/sbin/$(PROG)
else
	install -D -m $(BIN_MODE) -o $(OWNER) -s $(PROG) $(PREFIX)/usr/sbin/$(PROG)
endif
	$(MAKE) -C $(DIR_LIBSRC) install

install-data:
	# Install etc
	mkdir -m $(DIR_MODE) -p $(PREFIX)/etc/rscriptd
	install -m $(DATA_MODE) -o $(OWNER) ./rscriptd.conf $(PREFIX)/etc/rscriptd/rscriptd.conf
ifeq ($(OS), linux)
	install -D -m $(BIN_MODE) -o $(OWNER) ../stargazer/inst/linux/etc/stargazer/OnConnect $(PREFIX)/etc/rscriptd/
	install -D -m $(BIN_MODE) -o $(OWNER) ../stargazer/inst/linux/etc/stargazer/OnDisconnect $(PREFIX)/etc/rscriptd/
else
	install -D -m $(BIN_MODE) -o $(OWNER) ../stargazer/inst/freebsd/etc/stargazer/OnConnect $(PREFIX)/etc/rscriptd/
	install -D -m $(BIN_MODE) -o $(OWNER) ../stargazer/inst/freebsd/etc/stargazer/OnDisconnect $(PREFIX)/etc/rscriptd/
endif

uninstall: uninstall-bin uninstall-data

uninstall-bin:
	rm -f $(PREFIX)/usr/sbin/$(PROG)

uninstall-data:
	# Uninstall etc
	rm -f $(PREFIX)/etc/rscriptd/rscriptd.conf
	rm -f $(PREFIX)/etc/rscriptd/OnConnect
	rm -f $(PREFIX)/etc/rscriptd/OnDisconnect

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
