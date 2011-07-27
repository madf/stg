###############################################################################
# $Id: Makefile,v 1.5 2009/03/03 15:49:34 faust Exp $
###############################################################################

include ../../Makefile.conf

LIB_NAME = rlm_stg

PROG = $(LIB_NAME).so

SRCS = ./rlm_stg.cpp \
       ./stg_client.cpp

STGLIBS = -lstgcommon \
          -lstgcrypto

LIBS += $(LIB_THREAD)

ifeq ($(OS),linux)
LIBS += -ldl
else
LIBS += -lintl \
        -lc
endif

SEARCH_DIRS = -I $(DIR_INCLUDE)

OBJS = $(notdir $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS))))

CXXFLAGS += -Wall -fPIC -I./ 
LDFLAGS += -shared -L$(DIR_LIB) -Wl,-rpath,$(PREFIX)/usr/lib/stg

vpath %.a $(DIR_LIB)

.PHONY: all clean distclean libs install uninstall install-bin uninstall-bin
all: libs $(PROG) ../../Makefile.conf

libs:
	$(MAKE) -C $(DIR_LIBSRC)

$(PROG): $(OBJS) $(STGLIBS) 
	$(CXX) $^ $(LDFLAGS) -o $(PROG) $(LIBS)

clean:
	rm -f deps $(PROG) *.o tags *.*~ .OS
	rm -f .OS
	rm -f core*
	$(MAKE) -C $(DIR_LIBSRC) clean

distclean: clean
	rm -f ../../Makefile.conf

install: install-bin

install-bin:
	install -m $(BIN_MODE) -o $(OWNER) -s $(PROG) $(PREFIX)/usr/lib/$(PROG)
	$(MAKE) -C $(DIR_LIBSRC) install

uninstall: uninstall-bin

uninstall-bin:
	rm -f $(PREFIX)/usr/lib/$(PROG)

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


