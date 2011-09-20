###############################################################################
# $Id: Makefile,v 1.18 2010/02/11 12:22:01 faust Exp $
###############################################################################

include ../../Makefile.conf

PROG = rscriptd

SRCS = ./main.cpp \
       ./listener.cpp \
       ./pidfile.cpp

STGLIBS =  logger \
	   locker \
           common \
	   crypto \
           scriptexecuter \
           conffiles

ifneq ($(OS),linux)
LIBS += -liconv
endif

STGLIBS_INCS = $(addprefix -I ../../stglibs/,$(addsuffix .lib/include,$(STGLIBS)))
STGLIBS_LIBS = $(addprefix -L ../../stglibs/,$(addsuffix .lib,$(STGLIBS)))

LIBS += $(LIB_THREAD) $(addprefix -lstg,$(STGLIBS))

SEARCH_DIRS = -I ../../include

OBJS = $(notdir $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS))))

CXXFLAGS += $(DEFS) $(STGLIBS_INCS) $(SEARCH_DIRS)
CFLAGS += $(DEFS) $(STGLIBS_INCS) $(SEARCH_DIRS)
LDFLAGS += -Wl,-E $(STGLIBS_LIBS)

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
	$(MAKE) -C $(DIR_LIBSRC)
	@>deps ;\
	for file in $(SRCS); do\
	  echo "`$(CC) $(CXXFLAGS) -MM $$file` Makefile" >> deps ;\
	  echo -e '\t$$(CC) -c $$< $(CXXFLAGS)' >> deps ;\
	done


