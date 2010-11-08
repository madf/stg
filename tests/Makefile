CXXFLAGS+=-g3 -Wall -W -pedantic -DLINUX -I../include -I../projects/stargazer
LIBS=-lpthread
PROG=tests

SOURCES=main.cpp \
	test_raw_ip.cpp \
	test_admin_conf.cpp \
	test_tariff.cpp \
	../projects/stargazer/tariff.cpp

all: $(PROG) 

$(PROG): $(subst .cpp,.o,$(SOURCES))
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@

clean:
	rm -f *.o *.d

ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(subst .cpp,.d,$(SOURCES))
endif
endif

%.d: %.cpp
	@$(CC) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\).o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
