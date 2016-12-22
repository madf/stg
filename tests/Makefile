INCS = -I . \
       -I ../include \
       -I ../stglibs/common.lib/include \
       -I ../stglibs/conffiles.lib/include \
       -I ../stglibs/logger.lib/include \
       -I ../stglibs/scriptexecuter.lib/include \
       -I ../stglibs/crypto.lib/include \
       -I ../projects/stargazer
DEFS = -DLINUX \
       -DUSE_ABSTRACT_SETTINGS
CFLAGS += -g3 -Wall -W -Wextra -Wno-unused-function $(INCS) $(DEFS)
CXXFLAGS += $(CFLAGS)
LIBS = -lpthread
PROG = tests

SOURCES = main.cpp \
	  test_raw_ip.cpp \
  	  test_admin_conf.cpp \
	  test_tariff.cpp \
	  test_conffiles.cpp \
	  test_fee_charge_rules.cpp \
	  test_reconnect_on_tariff_change.cpp \
	  test_disable_session_log.cpp \
	  test_filter_params_log.cpp \
	  test_crypto.cpp \
	  test_bfstream.cpp \
	  ../projects/stargazer/tariff_impl.cpp \
	  ../projects/stargazer/user_impl.cpp \
	  ../projects/stargazer/user_property.cpp \
	  ../stglibs/conffiles.lib/conffiles.cpp \
	  ../stglibs/common.lib/common.cpp \
	  ../stglibs/common.lib/strptime.cpp \
	  ../stglibs/logger.lib/logger.cpp \
	  ../stglibs/scriptexecuter.lib/scriptexecuter.cpp \
	  ../stglibs/crypto.lib/bfstream.cpp \
	  ../stglibs/crypto.lib/blowfish.c

.PHONY: all clean

all: $(PROG)

$(PROG): $(addsuffix .o,$(basename $(SOURCES)))
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@

clean:
	rm -f $(addsuffix .o,$(basename $(SOURCES))) $(addsuffix .d,$(SOURCES)) $(PROG)

ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(addsuffix .d,$(SOURCES))
endif
endif

%.cpp.d: %.cpp
	@$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\).o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.c.d: %.c
	@$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\).o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
