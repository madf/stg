include make.conf

CFLAGS += -g3 -W -Wall -pedantic
CFLAGS += $(DEFINES) -D_BSD_SOURCE

CXXFLAGS += $(CFLAGS)

SOURCES=logger.cpp lock.cpp traffcounter.cpp rules.cpp utils.cpp
RULES_TESTER_SOURCES=logger.cpp rules.cpp utils.cpp rules_tester.cpp
RULES_FINDER_TESTER_SOURCES=logger.cpp lock.cpp rules.cpp rules_finder.cpp utils.cpp rf_tester.cpp
TC_TESTER_SOURCES=logger.cpp rules.cpp rules_finder.cpp utils.cpp traffcounter.cpp lock.cpp tc_tester.cpp
LIBS=-lpthread
PROG=st_core

.PHONY: all tests clean

#all: $(PROG)
all: tests

$(PROG): $(subst .cpp,.o,$(SOURCES))
	$(CXX) $^ $(LDFLAGS) $(LIBS) -o $@

tests: rules_tester rf_tester tc_tester

rules_tester: $(subst .cpp,.o,$(RULES_TESTER_SOURCES))
	$(CXX) $^ $(LDFLAGS) -o $@

rf_tester: $(subst .cpp,.o,$(RULES_FINDER_TESTER_SOURCES))
	$(CXX) $^ $(LDFLAGS) -o $@

tc_tester: $(subst .cpp,.o,$(TC_TESTER_SOURCES))
	$(CXX) $^ $(LDFLAGS) $(LIBS) -o $@

clean:
	rm -f $(PROG) *.o *d rules_tester rf_tester tc_tester gmon.out

ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),uninstall)
-include $(subst .cpp,.d,$(SOURCES))
endif
endif
endif

%.d: %.cpp
	@$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	    sed 's,\($*\).o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	    rm -f $@.$$$$
