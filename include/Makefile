include ../../Makefile.conf

INCS = stg/admin_conf.h \
       stg/admin_conf.inc.h \
       stg/admin.h \
       stg/admins.h \
       stg/auth.h \
       stg/const.h \
       stg/corp_conf.h \
       stg/ia_packets.h \
       stg/message.h \
       stg/module_settings.h \
       stg/noncopyable.h \
       stg/notifer.h \
       stg/os_int.h \
       stg/plugin.h \
       stg/rad_packets.h \
       stg/raw_ip_packet.h \
       stg/resetable.h \
       stg/rs_packets.h \
       stg/service_conf.h \
       stg/settings.h \
       stg/store.h \
       stg/tariff_conf.h \
       stg/tariff.h \
       stg/tariffs.h \
       stg/traffcounter.h \
       stg/user_conf.h \
       stg/user.h \
       stg/user_ips.h \
       stg/user_property.h \
       stg/users.h \
       stg/user_stat.h \
       stg/user_traff.h \
       stg/utime.h \
       stg/version.h

install: install-bin

install-bin:
	mkdir -m $(DIR_MODE) -p $(PREFIX)/usr/include/stg
	install -m $(DATA_MODE) -o $(OWNER) -t $(PREFIX)/usr/include/stg/ $(INCS)
