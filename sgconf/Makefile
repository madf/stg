###############################################################################
# $Id: Makefile,v 1.21 2010/02/11 12:34:14 faust Exp $
###############################################################################

include ../../Makefile.conf

PROG = sgconf

SRCS = ./main.cpp \
       ./options.cpp \
       ./api_action.cpp \
       ./actions.cpp \
       ./admins.cpp \
       ./tariffs.cpp \
       ./users.cpp \
       ./services.cpp \
       ./corps.cpp \
       ./info.cpp \
       ./xml.cpp

STGLIBS = srvconf \
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
LDFLAGS += $(STGLIBS_LIBS)

ifneq ($(OS),darwin)
LDFLAGS += -Wl,-E
endif

.PHONY: all clean distclean libs install uninstall install-bin install-data uninstall-bin uninstall-data
all: libs $(PROG) ../../Makefile.conf

libs:
	$(MAKE) -C $(DIR_LIBSRC)

$(PROG): $(OBJS)
	$(CXX) $^ $(LDFLAGS) $(LIBS) -o $(PROG)

clean:
	rm -f deps $(PROG) *.o
	$(MAKE) -C $(DIR_LIBSRC) clean

distclean: clean
	rm -f ../../Makefile.conf

install: install-bin

install-bin:
ifeq ($(DEBUG), yes)
	mkdir -m $(DIR_MODE) -p $(PREFIX)/usr/sbin
	install -m $(BIN_MODE) -o $(OWNER) $(PROG) $(PREFIX)/usr/sbin/$(PROG)
else
	mkdir -m $(DIR_MODE) -p $(PREFIX)/usr/sbin
	install -m $(BIN_MODE) -o $(OWNER) -s $(PROG) $(PREFIX)/usr/sbin/$(PROG)
endif
	$(MAKE) -C $(DIR_LIBSRC) install

uninstall: uninstall-bin

uninstall-bin:
	rm -f $(PREFIX)/usr/sbin/$(PROG)


ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),uninstall)
-include deps
endif
endif
endif

deps: $(SRCS) ../../Makefile.conf
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
