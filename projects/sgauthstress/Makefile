include ../../Makefile.conf

PROG = sgauthstress

SRCS = main.cpp \
       settings.cpp \
       store_loader.cpp \
       user.cpp \
       proto.cpp

STGLIBS = dotconfpp \
	  locker \
	  crypto \
	  common

STGLIBS_INCS = $(addprefix -I ../../stglibs/,$(addsuffix .lib/include,$(STGLIBS)))
STGLIBS_LIBS = $(addprefix -L ../../stglibs/,$(addsuffix .lib,$(STGLIBS)))

LIBS += $(addprefix -lstg,$(STGLIBS)) $(LIB_THREAD)

ifeq ($(OS),linux)
LIBS += -ldl
else
LIBS += -lintl \
        -lc \
	-liconv
endif

SEARCH_DIRS = -I ../../include

OBJS = $(notdir $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS))))

CXXFLAGS += $(DEFS) $(STGLIBS_INCS) $(SEARCH_DIRS)
CFLAGS += $(DEFS) $(STGLIBS_INCS) $(SEARCH_DIRS)
LDFLAGS += -Wl,-E $(STGLIBS_LIBS)

.PHONY: all clean distclean libs plugins
all: libs plugins $(PROG) ../../Makefile.conf

libs:
	$(MAKE) -C $(DIR_LIBSRC)

plugins: libs 
	$(MAKE) -C $(DIR_PLUGINS)

$(PROG): $(OBJS)
	$(CXX) $^ $(LDFLAGS) $(LIBS) -o $(PROG)

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
