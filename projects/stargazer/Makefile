###############################################################################
# $Id: Makefile,v 1.50 2010/10/07 18:27:27 faust Exp $
###############################################################################

include ../../Makefile.conf

PROG = stargazer

SRCS = ./admin_impl.cpp \
       ./admins_impl.cpp \
       ./corps_impl.cpp \
       ./eventloop.cpp \
       ./main.cpp \
       ./pidfile.cpp \
       ./plugin_runner.cpp \
       ./settings_impl.cpp \
       ./services_impl.cpp \
       ./stg_timer.cpp \
       ./store_loader.cpp \
       ./tariff_impl.cpp \
       ./tariffs_impl.cpp \
       ./traffcounter_impl.cpp \
       ./user_impl.cpp \
       ./user_property.cpp \
       ./users_impl.cpp

STGLIBS = scriptexecuter \
          dotconfpp \
	  locker \
	  logger \
	  common

STGLIBS_INCS = $(addprefix -I ../../stglibs/,$(addsuffix .lib/include,$(STGLIBS)))
STGLIBS_LIBS = $(addprefix -L ../../stglibs/,$(addsuffix .lib,$(STGLIBS)))

LIBS += $(addprefix -lstg,$(STGLIBS)) $(LIB_THREAD)

ifeq ($(OS),linux)
LIBS += -ldl
else
LIBS += -lc -liconv
endif

SEARCH_DIRS = -I ../../include

OBJS = $(notdir $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS))))

CXXFLAGS += $(DEFS) $(STGLIBS_INCS) $(SEARCH_DIRS)
CFLAGS += $(DEFS) $(STGLIBS_INCS) $(SEARCH_DIRS)
LDFLAGS += -Wl,-E $(STGLIBS_LIBS)

.PHONY: all clean distclean libs plugins install uninstall install-bin install-data
all: libs plugins $(PROG)

libs:
	$(MAKE) -C $(DIR_LIBSRC)

plugins: libs 
	$(MAKE) -C $(DIR_PLUGINS)

$(PROG): $(OBJS)
	$(CXX) $^ $(LDFLAGS) $(LIBS) -o $(PROG)

clean:
	rm -f deps $(PROG) *.o tags *.*~ .OS
	rm -f .OS
	rm -f .store
	rm -f .db.sql
	rm -f core*
	$(MAKE) -C $(DIR_LIBSRC) clean
	$(MAKE) -C $(DIR_PLUGINS) clean

distclean: clean
	rm -f $(DIR_MOD)/*
	rm -f ../../Makefile.conf

install: install-bin install-data

install-bin: $(PROG)
	install -m $(BIN_MODE) -o $(OWNER) -s $(PROG) $(PREFIX)/usr/sbin/$(PROG)
	$(MAKE) -C $(DIR_INCLUDE) install
	$(MAKE) -C $(DIR_LIBSRC) install
	$(MAKE) -C $(DIR_PLUGINS) install

install-data:
	# Install etc
	mkdir -m $(DIR_MODE) -p $(PREFIX)/etc/stargazer
	mkdir -m $(DIR_MODE) -p $(PREFIX)/etc/stargazer/conf-available.d
	mkdir -m $(DIR_MODE) -p $(PREFIX)/etc/stargazer/conf-enabled.d
	install -m $(DATA_MODE) -o $(OWNER) $(ETC_DIR)/stargazer.conf $(PREFIX)/etc/stargazer/stargazer.conf
	install -m $(DATA_MODE) -o $(OWNER) $(ETC_DIR)/conf-available.d/*.conf $(PREFIX)/etc/stargazer/conf-available.d
	ln -s ../conf-available.d/mod_ao.conf $(PREFIX)/etc/stargazer/conf-enabled.d/mod_ao.conf
	ln -s ../conf-available.d/mod_ia.conf $(PREFIX)/etc/stargazer/conf-enabled.d/mod_ia.conf
	ln -s ../conf-available.d/mod_ping.conf $(PREFIX)/etc/stargazer/conf-enabled.d/mod_ping.conf
	ln -s ../conf-available.d/mod_sg.conf $(PREFIX)/etc/stargazer/conf-enabled.d/mod_sg.conf

ifeq ($(OS),linux)
	ln -s ../conf-available.d/mod_cap_ether.conf $(PREFIX)/etc/stargazer/conf-enabled.d/mod_cap_ether.conf
else
	ln -s ../conf-available.d/mod_cap_bpf.conf $(PREFIX)/etc/stargazer/conf-enabled.d/mod_cap_bpf.conf
endif

	install -m $(DATA_MODE) -o $(OWNER) $(ETC_DIR)/rules $(PREFIX)/etc/stargazer/rules
	install -m $(BIN_MODE) -o $(OWNER) $(ETC_DIR)/On* $(PREFIX)/etc/stargazer/
	
	# Install file db
	mkdir -m $(DIR_MODE) -p $(PREFIX)/var/stargazer/admins
	mkdir -m $(DIR_MODE) -p $(PREFIX)/var/stargazer/tariffs
	mkdir -m $(DIR_MODE) -p $(PREFIX)/var/stargazer/users/test
	install -m $(DATA_MODE) -o $(OWNER) $(VAR_DIR)/admins/admin.adm $(PREFIX)/var/stargazer/admins/admin.adm
	install -m $(DATA_MODE) -o $(OWNER) $(VAR_DIR)/tariffs/tariff.tf $(PREFIX)/var/stargazer/tariffs/tariff.tf
	install -m $(DATA_MODE) -o $(OWNER) $(VAR_DIR)/users/test/conf $(PREFIX)/var/stargazer/users/test/conf
	install -m $(DATA_MODE) -o $(OWNER) $(VAR_DIR)/users/test/stat $(PREFIX)/var/stargazer/users/test/stat
	
ifeq ($(CHECK_FBCLIENT),yes)
	# Install firebird db
	mkdir -p $(PREFIX)/var/stargazer
	chown $(OWNER):$(FIREBIRD_GROUP) $(PREFIX)/var/stargazer
	chmod g+rw $(PREFIX)/var/stargazer
	echo "connect '$(DB_ADDRESS)' user '$(DB_USER)' password '$(DB_PASSWORD)';" > .db.sql
	echo "drop database;" >> .db.sql
	echo "create database '$(DB_ADDRESS)' user '$(DB_USER)' password '$(DB_PASSWORD)' default character set win1251;" >> .db.sql
	cat $(VAR_DIR)/../00-base-00.sql >> .db.sql
	$(FIREBIRD_ISQL) -i .db.sql
	rm -f .db.sql
endif

uninstall: uninstall-bin uninstall-data

uninstall-bin:
	rm -f $(PREFIX)/usr/sbin/$(PROG)
	$(MAKE) -C $(DIR_LIBSRC) uninstall
	$(MAKE) -C $(DIR_PLUGINS) uninstall
	rm -rf $(PREFIX)/usr/lib/stg

uninstall-data:
	# Uninstall etc
	rm -rf $(PREFIX)/etc/stargazer
	rm -rf $(PREFIX)/var/stargazer


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
	  echo "`$(CC) $(CXXFLAGS) -MM $$file` Makefile ../../Makefile.conf" >> deps ;\
	  echo -e '\t$$(CC) -c $$< $(CXXFLAGS)' >> deps ;\
	done
