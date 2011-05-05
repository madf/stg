include ../../Makefile.conf

PROG = sgauthstress

SRCS = main.cpp \
       settings.cpp \
       store_loader.cpp \
       user.cpp \
       proto.cpp

STGLIBS =  -lstgcrypto \
           -lstgcommon \
           -ldotconfpp \
           -lstgia

LIBS += $(LIB_THREAD)

ifeq ($(OS),linux)
LIBS += -ldl
else
LIBS += -lintl \
        -lc
endif

SEARCH_DIRS = -I $(DIR_INCLUDE)

OBJS = $(notdir $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS))))

CXXFLAGS += -Wall
LDFLAGS += -Wl,-E -L$(DIR_LIB) -Wl,-rpath,$(PREFIX)/usr/lib/stg -Wl,-rpath-link,$(DIR_LIB)

vpath %.so $(DIR_LIB)

.PHONY: all clean distclean libs plugins
all: libs plugins $(PROG) ../../Makefile.conf

libs:
	$(MAKE) -C $(DIR_LIBSRC)

plugins: libs 
	$(MAKE) -C $(DIR_PLUGINS)

$(PROG): $(OBJS) libs
	$(CXX) $(OBJS) $(LDFLAGS) $(LIBS) $(STGLIBS) -o $(PROG)

clean:
	rm -f deps $(PROG) *.o *.*~
	$(MAKE) -C $(DIR_LIBSRC) clean
	$(MAKE) -C $(DIR_PLUGINS) clean

distclean: clean
	rm -f ../../Makefile.conf

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
	  echo "`$(CC) $(CXXFLAGS) $(SEARCH_DIRS) -MM $$file` Makefile ../../Makefile.conf" >> deps ;\
	  echo -e '\t$$(CC) -c $$< $(CXXFLAGS) $(SEARCH_DIRS) $(DEFS)' >> deps ;\
	done
