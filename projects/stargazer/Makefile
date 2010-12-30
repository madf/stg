###############################################################################
# $Id: Makefile,v 1.50 2010/10/07 18:27:27 faust Exp $
###############################################################################

include ../../Makefile.conf

PROG = stargazer

SRCS = ./admin.cpp \
       ./admins.cpp \
       ./main.cpp \
       ./settings.cpp \
       ./stg_timer.cpp \
       ./tariff.cpp \
       ./tariffs.cpp \
       ./traffcounter.cpp \
       ./user.cpp \
       ./user_property.cpp \
       ./users.cpp \
       ./plugin_runner.cpp \
       ./store_loader.cpp \
       ./pidfile.cpp \
       ./eventloop.cpp

STGLIBS =  -lstg_logger \
    	   -lstg_locker \
           -lstg_common \
           -lscript_executer \
           -ldotconfpp

LIBS += -lexpat

ifeq ($(OS),linux)
LIBS += $(LIB_THREAD) \
        -ldl
else
LIBS += $(LIB_THREAD) \
        -lc
endif

SEARCH_DIRS = -I $(DIR_INCLUDE)

OBJS = $(notdir $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS))))

LDFLAGS += -Wl,-E -L$(DIR_LIB) -Wl,-rpath,$(PREFIX)/usr/lib/stg -Wl,-rpath-link,$(DIR_LIB) 
vpath %.so $(DIR_LIB)

.PHONY: all clean distclean libs plugins install uninstall install-bin install-data
all: libs plugins $(PROG)

libs:
	$(MAKE) -C $(DIR_LIBSRC)

plugins: libs 
	$(MAKE) -C $(DIR_PLUGINS)

$(PROG): $(OBJS) libs
	$(CXX) $(OBJS) $(LDFLAGS) $(LIBS) $(STGLIBS) -o $(PROG)

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

install-bin:
	mkdir -m $(BIN_MODE) -p $(PREFIX)/usr/sbin
	install -m $(BIN_MODE) -o $(OWNER) -s $(PROG) $(PREFIX)/usr/sbin/$(PROG)
	$(MAKE) -C $(DIR_LIBSRC) install
	$(MAKE) -C $(DIR_PLUGINS) install

install-data:
	# Install etc
	mkdir -m $(DATA_MODE) -p $(PREFIX)/etc/stargazer
	mkdir -m $(DATA_MODE) -p $(PREFIX)/etc/stargazer/conf-available.d
	mkdir -m $(DATA_MODE) -p $(PREFIX)/etc/stargazer/conf-enabled.d
	install -m $(DATA_MODE) -o $(OWNER) $(ETC_DIR)/stargazer.conf $(PREFIX)/etc/stargazer/stargazer.conf
	install -m $(DATA_MODE) -o $(OWNER) $(ETC_DIR)/conf-available.d/*.conf $(PREFIX)/etc/stargazer/conf-available.d
	ln -s $(PREFIX)/etc/stargazer/conf-available.d/mod_ao.conf $(PREFIX)/etc/stargazer/conf-enabled.d/mod_ao.conf
	ln -s $(PREFIX)/etc/stargazer/conf-available.d/mod_ia.conf $(PREFIX)/etc/stargazer/conf-enabled.d/mod_ia.conf
	ln -s $(PREFIX)/etc/stargazer/conf-available.d/mod_ping.conf $(PREFIX)/etc/stargazer/conf-enabled.d/mod_ping.conf
	ln -s $(PREFIX)/etc/stargazer/conf-available.d/mod_sg.conf $(PREFIX)/etc/stargazer/conf-enabled.d/mod_sg.conf

ifeq ($(OS),linux)
	ln -s $(PREFIX)/etc/stargazer/conf-available.d/mod_cap_ether.conf $(PREFIX)/etc/stargazer/conf-enabled.d/mod_cap_ether.conf
else
	ln -s $(PREFIX)/etc/stargazer/conf-available.d/mod_cap_bpf.conf $(PREFIX)/etc/stargazer/conf-enabled.d/mod_cap_bpf.conf
endif

	install -m $(DATA_MODE) -o $(OWNER) $(ETC_DIR)/rules $(PREFIX)/etc/stargazer/rules
	install -m $(BIN_MODE) -o $(OWNER) $(ETC_DIR)/On* $(PREFIX)/etc/stargazer/
	
	# Install file db
	mkdir -m $(DATA_MODE) -p $(PREFIX)/var/stargazer/admins
	mkdir -m $(DATA_MODE) -p $(PREFIX)/var/stargazer/tariffs
	mkdir -m $(DATA_MODE) -p $(PREFIX)/var/stargazer/users/test
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

deps:	$(SRCS) ../../Makefile.conf
	$(MAKE) -C $(DIR_LIBSRC) includes
	@>deps ;\
	for file in $(SRCS); do\
	  echo "`$(CC) $(CXXFLAGS) $(SEARCH_DIRS) -MM $$file` Makefile ../../Makefile.conf" >> deps ;\
	  echo -e '\t$$(CC) -c $$< $(CXXFLAGS) $(SEARCH_DIRS) $(DEFS)' >> deps ;\
	done
