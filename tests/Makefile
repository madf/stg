CXXFLAGS+=-g3 -Wall -W -Wextra -DLINUX -I../include -I../projects/stargazer -I../stglibs/stgconffiles.lib -I.
LIBS=-lpthread
PROG=tests

SOURCES=main.cpp \
	test_raw_ip.cpp \
	test_admin_conf.cpp \
	test_tariff.cpp \
	test_conffiles.cpp \
	../projects/stargazer/tariff_impl.cpp \
	../stglibs/conffiles.lib/conffiles.cpp \
	../stglibs/common.lib/common.cpp

all: $(PROG) 

$(PROG): $(subst .cpp,.o,$(SOURCES))
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@

clean:
	rm -f *.o *.d $(PROG)

ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(subst .cpp,.d,$(SOURCES))
endif
endif

%.d: %.cpp
	@$(CC) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\).o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
