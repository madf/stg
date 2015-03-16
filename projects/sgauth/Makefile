###############################################################################
# $Id: Makefile,v 1.18 2009/08/06 12:50:55 faust Exp $
###############################################################################

include ../../Makefile.conf

PROG = sgauth

SRCS = ./main.cpp \
       ./settings_impl.cpp \
       ./web.cpp

STGLIBS =  conffiles \
           ia \
           crypto \
           common

STGLIBS_INCS = $(addprefix -I ../../stglibs/,$(addsuffix .lib/include,$(STGLIBS)))
STGLIBS_LIBS = $(addprefix -L ../../stglibs/,$(addsuffix .lib,$(STGLIBS)))

LIBS += $(addprefix -lstg,$(STGLIBS)) $(LIB_THREAD)

ifeq ($(OS),linux)
else
LIBS += -lintl \
        -lc \
        -liconv
endif

SEARCH_DIRS = -I ../../include

OBJS = $(notdir $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS))))

CXXFLAGS += $(DEFS) $(STGLIBS_INCS) $(SEARCH_DIRS)
CFLAGS += $(DEFS) $(STGLIBS_INCS) $(SEARCH_DIRS)
LDFLAGS += $(STGLIBS_LIBS)

.PHONY: all clean distclean libs install uninstall install-bin install-data uninstall-bin uninstall-data
all: libs $(PROG) ../../Makefile.conf

libs:
	$(MAKE) -C $(DIR_LIBSRC)

$(PROG): $(OBJS)
	$(CXX) $^ $(LDFLAGS) -o $(PROG) $(LIBS)

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
ifeq ($(DEBUG), yes)
	mkdir -m $(DIR_MODE) -p $(PREFIX)/usr/sbin
	install -m $(BIN_MODE) -o $(OWNER) $(PROG) $(PREFIX)/usr/sbin/$(PROG)
else
	mkdir -m $(DIR_MODE) -p $(PREFIX)/usr/sbin
	install -m $(BIN_MODE) -o $(OWNER) -s $(PROG) $(PREFIX)/usr/sbin/$(PROG)
endif
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
	$(MAKE) -C $(DIR_LIBSRC)
	@>deps ;\
	./make_css.sh ;\
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
