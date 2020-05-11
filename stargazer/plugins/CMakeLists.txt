if ( BUILD_MOD_AO )
    add_library ( mod_auth_ao MODULE authorization/ao/ao.cpp )
    target_link_libraries ( mod_auth_ao scriptexecuter logger common )
    set_target_properties ( mod_auth_ao PROPERTIES PREFIX "" )
endif ( BUILD_MOD_AO )

if ( BUILD_MOD_IA )
    add_library ( mod_auth_ia MODULE authorization/inetaccess/inetaccess.cpp )
    target_link_libraries ( mod_auth_ia scriptexecuter crypto logger common )
    set_target_properties ( mod_auth_ia PROPERTIES PREFIX "" )
    if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
        set_target_properties ( mod_auth_ia PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
    endif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
endif ( BUILD_MOD_IA )

if ( BUILD_MOD_CAP_NF )
    add_library ( mod_cap_nf MODULE capture/cap_nf/cap_nf.cpp )
    target_link_libraries ( mod_cap_nf logger common )
    set_target_properties ( mod_cap_nf PROPERTIES PREFIX "" )
endif ( BUILD_MOD_CAP_NF )

if ( BUILD_MOD_CAP_DIVERT )
    add_library ( mod_cap_divert MODULE capture/divert_freebsd/divert_cap.cpp )
    target_link_libraries ( mod_cap_divert logger common )
    set_target_properties ( mod_cap_divert PROPERTIES PREFIX "" )
endif ( BUILD_MOD_CAP_DIVERT )

if ( BUILD_MOD_CAP_ETHER_FREEBSD )
    add_library ( mod_cap_bpf MODULE capture/ether_freebsd/ether_cap.cpp )
    target_link_libraries ( mod_cap_bpf logger common )
    set_target_properties ( mod_cap_bpf PROPERTIES PREFIX "" )
endif ( BUILD_MOD_CAP_ETHER_FREEBSD )

if ( BUILD_MOD_CAP_ETHER_LINUX )
    add_library ( mod_cap_ether MODULE capture/ether_linux/ether_cap.cpp )
    target_link_libraries ( mod_cap_ether logger common )
    set_target_properties ( mod_cap_ether PROPERTIES PREFIX "" )
endif ( BUILD_MOD_CAP_ETHER_LINUX )

if ( BUILD_MOD_CAP_PCAP )
    find_package ( PCap REQUIRED )
    add_library ( mod_cap_pcap MODULE capture/pcap/pcap_cap.cpp )
    target_link_libraries ( mod_cap_pcap logger common PCap::PCap )
    set_target_properties ( mod_cap_pcap PROPERTIES PREFIX "" )
endif ( BUILD_MOD_CAP_PCAP )

if ( BUILD_MOD_CAP_NFQUEUE )
    find_package ( NFQueue REQUIRED )
    find_package ( NFNetLink REQUIRED )
    find_package ( MNL REQUIRED )
    add_library ( mod_cap_nfqueue MODULE capture/nfqueue/nfqueue.cpp )
    target_link_libraries ( mod_cap_nfqueue logger common NF::Queue NF::NetLink MNL::MNL )
    set_target_properties ( mod_cap_nfqueue PROPERTIES PREFIX "" )
endif ( BUILD_MOD_CAP_NFQUEUE )

if ( BUILD_MOD_SGCONFIG )
    find_package ( EXPAT REQUIRED )
    add_library ( mod_conf_sg MODULE configuration/sgconfig/stgconfig.cpp
                                     configuration/sgconfig/conn.cpp
                                     configuration/sgconfig/configproto.cpp
                                     configuration/sgconfig/parser.cpp
                                     configuration/sgconfig/parser_tariffs.cpp
                                     configuration/sgconfig/parser_admins.cpp
                                     configuration/sgconfig/parser_users.cpp
                                     configuration/sgconfig/parser_services.cpp
                                     configuration/sgconfig/parser_message.cpp
                                     configuration/sgconfig/parser_auth_by.cpp
                                     configuration/sgconfig/parser_user_info.cpp
                                     configuration/sgconfig/parser_server_info.cpp )
    target_link_libraries ( mod_conf_sg scriptexecuter crypto logger common EXPAT::EXPAT )
    set_target_properties ( mod_conf_sg PROPERTIES PREFIX "" )
    if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
        set_target_properties ( mod_conf_sg PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
    endif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
endif ( BUILD_MOD_SGCONFIG )

if ( BUILD_MOD_RPCCONFIG )
    find_package ( XMLRPC REQUIRED c++ abyss-server )
    include_directories ( ${XMLRPC_INCLUDE_DIRS} )
    add_library ( mod_conf_rpc MODULE configuration/rpcconfig/rpcconfig.cpp
                                      configuration/rpcconfig/user_helper.cpp
                                      configuration/rpcconfig/tariff_helper.cpp
                                      configuration/rpcconfig/info_methods.cpp
                                      configuration/rpcconfig/users_methods.cpp
                                      configuration/rpcconfig/tariffs_methods.cpp
                                      configuration/rpcconfig/admins_methods.cpp
                                      configuration/rpcconfig/messages_methods.cpp )
                              target_link_libraries ( mod_conf_rpc scriptexecuter logger common ${XMLRPC_LIBRARIES} )
    set_target_properties ( mod_conf_rpc PROPERTIES PREFIX "" )
endif ( BUILD_MOD_RPCCONFIG )

if ( BUILD_MOD_PING )
    add_library ( mod_ping MODULE other/ping/ping.cpp )
    target_link_libraries ( mod_ping scriptexecuter logger pinger common )
    set_target_properties ( mod_ping PROPERTIES PREFIX "" )
endif ( BUILD_MOD_PING )

if ( BUILD_MOD_RSCRYPT )
    add_library ( mod_remote_script MODULE other/rscript/rscript.cpp other/rscript/nrmap_parser.cpp )
    target_link_libraries ( mod_remote_script crypto scriptexecuter logger common )
    set_target_properties ( mod_remote_script PROPERTIES PREFIX "" )
    if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
        set_target_properties ( mod_remote_script PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
    endif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
endif ( BUILD_MOD_RSCRYPT )

if ( BUILD_MOD_SMUX )
    add_library ( mod_smux MODULE other/smux/smux.cpp
                                  other/smux/sensors.cpp
                                  other/smux/tables.cpp
                                  other/smux/handlers.cpp
                                  other/smux/utils.cpp
                                  other/smux/types.cpp )
    target_link_libraries ( mod_smux scriptexecuter logger smux common )
    set_target_properties ( mod_smux PROPERTIES PREFIX "" )
endif ( BUILD_MOD_SMUX )

if ( BUILD_MOD_STORE_FILES )
    add_library ( mod_store_files MODULE store/files/file_store.cpp )
    target_link_libraries ( mod_store_files crypto conffiles logger common )
    set_target_properties ( mod_store_files PROPERTIES PREFIX "" )
endif ( BUILD_MOD_STORE_FILES )

if ( BUILD_MOD_STORE_FIREBIRD )
    find_package ( FBClient REQUIRED )
    add_library ( mod_store_firebird MODULE store/firebird/firebird_store.cpp
                                            store/firebird/firebird_store_admins.cpp
                                            store/firebird/firebird_store_corporations.cpp
                                            store/firebird/firebird_store_messages.cpp
                                            store/firebird/firebird_store_services.cpp
                                            store/firebird/firebird_store_tariffs.cpp
                                            store/firebird/firebird_store_users.cpp
                                            store/firebird/firebird_store_utils.cpp )
    target_link_libraries ( mod_store_firebird crypto common logger ibpp FBClient::FBClient )
    set_target_properties ( mod_store_firebird PROPERTIES PREFIX "" )
endif ( BUILD_MOD_STORE_FIREBIRD )

if ( BUILD_MOD_STORE_MYSQL )
    find_package ( MySQLConnector REQUIRED )
    add_library ( mod_store_mysql MODULE store/mysql/mysql_store.cpp )
    target_link_libraries ( mod_store_mysql crypto logger common MySQL::Connector )
    set_target_properties ( mod_store_mysql PROPERTIES PREFIX "" )
endif ( BUILD_MOD_STORE_MYSQL )

if ( BUILD_MOD_STORE_POSTGRESQL )
    find_package ( PostgreSQL REQUIRED )
    include_directories ( ${PostgreSQL_INCLUDE_DIRS} )
    add_library ( mod_store_postgresql MODULE store/postgresql/postgresql_store.cpp
                                              store/postgresql/postgresql_store_admins.cpp
                                              store/postgresql/postgresql_store_corporations.cpp
                                              store/postgresql/postgresql_store_messages.cpp
                                              store/postgresql/postgresql_store_services.cpp
                                              store/postgresql/postgresql_store_tariffs.cpp
                                              store/postgresql/postgresql_store_users.cpp
                                              store/postgresql/postgresql_store_utils.cpp )
                                      target_link_libraries ( mod_store_postgresql crypto logger common ${PostgreSQL_LIBRARIES} )
    set_target_properties ( mod_store_postgresql PROPERTIES PREFIX "" )
endif ( BUILD_MOD_STORE_POSTGRESQL )
