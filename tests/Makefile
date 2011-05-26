CXXFLAGS+=-g3 -Wall -W -Wextra -DLINUX -I../include -I../projects/stargazer -I../stglibs/stgconffiles.lib -I. -DUSE_ABSTRACT_SETTINGS
LIBS=-lpthread
PROG=tests

SOURCES=main.cpp \
	test_raw_ip.cpp \
	test_admin_conf.cpp \
	test_tariff.cpp \
	test_conffiles.cpp \
	test_fee_charge_rules.cpp \
	test_reconnect_on_tariff_change.cpp \
	../projects/stargazer/tariff_impl.cpp \
	../projects/stargazer/user_impl.cpp \
	../projects/stargazer/user_property.cpp \
	../stglibs/conffiles.lib/conffiles.cpp \
	../stglibs/common.lib/common.cpp \
	../stglibs/logger.lib/logger.cpp \
	../stglibs/scriptexecuter.lib/scriptexecuter.cpp

.PHONY: all includes clean

all: includes $(PROG)

includes:
	make -C ../stglibs includes

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
