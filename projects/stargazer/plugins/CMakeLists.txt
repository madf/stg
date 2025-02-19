if ( BUILD_MOD_AO )
    add_library ( mod_auth_ao MODULE authorization/ao/ao.cpp )
    target_link_libraries ( mod_auth_ao scriptexecuter logger common )
    set_target_properties ( mod_auth_ao PROPERTIES PREFIX "" )

    if ( CLANG_TIDY_EXE )
        set_target_properties ( mod_auth_ao PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
    endif ()
    if ( INCLUDE_WHAT_YOU_USE_EXE )
        set_target_properties ( mod_auth_ao PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${DO_INCLUDE_WHAT_YOU_USE}" )
    endif ()
endif ( BUILD_MOD_AO )

if ( BUILD_MOD_RADIUS )
    find_package ( Boost REQUIRED )

    add_library ( mod_radius MODULE other/radius/radius.cpp
                                    other/radius/server.cpp)
    target_link_libraries ( mod_radius PRIVATE scriptexecuter logger common )
    set_target_properties ( mod_radius PROPERTIES PREFIX "" )

    include ( ExternalProject )

    ExternalProject_Add ( async-radius
                          GIT_REPOSITORY https://github.com/madf/async-radius.git
                          GIT_TAG 1.1.1
                          GIT_SHALLOW true
                          INSTALL_COMMAND ""
                          CMAKE_ARGS -DCMAKE_CXX_FLAGS=-fPIC )

    ExternalProject_Get_property ( async-radius SOURCE_DIR BINARY_DIR )

    target_include_directories ( mod_radius PRIVATE "${SOURCE_DIR}/include" )
    target_link_directories ( mod_radius PRIVATE "${BINARY_DIR}/src" )

    add_dependencies ( mod_radius async-radius )

    find_package ( OpenSSL 1.0.0 REQUIRED )
    target_link_libraries ( mod_radius PRIVATE radproto Boost::boost OpenSSL::Crypto)

    if ( CLANG_TIDY_EXE )
        set_target_properties ( mod_radius PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
    endif ()
    if ( INCLUDE_WHAT_YOU_USE_EXE )
        set_target_properties ( mod_radius PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${DO_INCLUDE_WHAT_YOU_USE}" )
    endif ()
endif ( BUILD_MOD_RADIUS )

if ( BUILD_MOD_IA )
    add_library ( mod_auth_ia MODULE authorization/inetaccess/inetaccess.cpp )
    target_link_libraries ( mod_auth_ia scriptexecuter crypto logger common )
    set_target_properties ( mod_auth_ia PROPERTIES PREFIX "" )
    if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
        set_target_properties ( mod_auth_ia PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
    endif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )

    if ( CLANG_TIDY_EXE )
        set_target_properties ( mod_auth_ia PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
    endif ()
    if ( INCLUDE_WHAT_YOU_USE_EXE )
        set_target_properties ( mod_auth_ia PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${DO_INCLUDE_WHAT_YOU_USE}" )
    endif ()
endif ( BUILD_MOD_IA )

if ( BUILD_MOD_CAP_NF )
    add_library ( mod_cap_nf MODULE capture/cap_nf/cap_nf.cpp )
    target_link_libraries ( mod_cap_nf logger common )
    set_target_properties ( mod_cap_nf PROPERTIES PREFIX "" )

    if ( CLANG_TIDY_EXE )
        set_target_properties ( mod_cap_nf PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
    endif ()
    if ( INCLUDE_WHAT_YOU_USE_EXE )
        set_target_properties ( mod_cap_nf PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${DO_INCLUDE_WHAT_YOU_USE}" )
    endif ()
endif ( BUILD_MOD_CAP_NF )

if ( BUILD_MOD_CAP_DIVERT )
    add_library ( mod_cap_divert MODULE capture/divert_freebsd/divert_cap.cpp )
    target_link_libraries ( mod_cap_divert logger common )
    set_target_properties ( mod_cap_divert PROPERTIES PREFIX "" )

    if ( CLANG_TIDY_EXE )
        set_target_properties ( mod_cap_divert PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
    endif ()
    if ( INCLUDE_WHAT_YOU_USE_EXE )
        set_target_properties ( mod_cap_divert PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${DO_INCLUDE_WHAT_YOU_USE}" )
    endif ()
endif ( BUILD_MOD_CAP_DIVERT )

if ( BUILD_MOD_CAP_ETHER_FREEBSD )
    add_library ( mod_cap_bpf MODULE capture/ether_freebsd/ether_cap.cpp )
    target_link_libraries ( mod_cap_bpf logger common )
    set_target_properties ( mod_cap_bpf PROPERTIES PREFIX "" )

    if ( CLANG_TIDY_EXE )
        set_target_properties ( mod_cap_bpf PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
    endif ()
    if ( INCLUDE_WHAT_YOU_USE_EXE )
        set_target_properties ( mod_cap_bpf PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${DO_INCLUDE_WHAT_YOU_USE}" )
    endif ()
endif ( BUILD_MOD_CAP_ETHER_FREEBSD )

if ( BUILD_MOD_CAP_ETHER_LINUX )
    add_library ( mod_cap_ether MODULE capture/ether_linux/ether_cap.cpp )
    target_link_libraries ( mod_cap_ether logger common )
    set_target_properties ( mod_cap_ether PROPERTIES PREFIX "" )

    if ( CLANG_TIDY_EXE )
        set_target_properties ( mod_cap_ether PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
    endif ()
    if ( INCLUDE_WHAT_YOU_USE_EXE )
        set_target_properties ( mod_cap_ether PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${DO_INCLUDE_WHAT_YOU_USE}" )
    endif ()
endif ( BUILD_MOD_CAP_ETHER_LINUX )

if ( BUILD_MOD_CAP_PCAP )
    find_package ( PCap REQUIRED )
    add_library ( mod_cap_pcap MODULE capture/pcap/pcap_cap.cpp )
    target_link_libraries ( mod_cap_pcap logger common PCap::PCap )
    set_target_properties ( mod_cap_pcap PROPERTIES PREFIX "" )

    if ( CLANG_TIDY_EXE )
        set_target_properties ( mod_cap_pcap PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
    endif ()
    if ( INCLUDE_WHAT_YOU_USE_EXE )
        set_target_properties ( mod_cap_pcap PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${DO_INCLUDE_WHAT_YOU_USE}" )
    endif ()
endif ( BUILD_MOD_CAP_PCAP )

if ( BUILD_MOD_CAP_NFQUEUE )
    find_package ( NFQueue REQUIRED )
    find_package ( NFNetLink REQUIRED )
    find_package ( MNL REQUIRED )
    add_library ( mod_cap_nfqueue MODULE capture/nfqueue/nfqueue.cpp )
    target_link_libraries ( mod_cap_nfqueue logger common NF::Queue NF::NetLink MNL::MNL )
    set_target_properties ( mod_cap_nfqueue PROPERTIES PREFIX "" )

    if ( CLANG_TIDY_EXE )
        set_target_properties ( mod_cap_nfqueue PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
    endif ()
    if ( INCLUDE_WHAT_YOU_USE_EXE )
        set_target_properties ( mod_cap_nfqueue PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${DO_INCLUDE_WHAT_YOU_USE}" )
    endif ()
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

    if ( CLANG_TIDY_EXE )
        set_target_properties ( mod_conf_sg PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
    endif ()
    if ( INCLUDE_WHAT_YOU_USE_EXE )
        set_target_properties ( mod_conf_sg PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${DO_INCLUDE_WHAT_YOU_USE}" )
    endif ()
endif ( BUILD_MOD_SGCONFIG )

if ( BUILD_MOD_RPCCONFIG )
    find_package ( XMLRPC 1.58.0 REQUIRED c++2 abyss-server )
    try_compile ( XMLRPC_C_NO_AUTO_PTR ${CMAKE_CURRENT_BINARY_DIR} "${PROJECT_SOURCE_DIR}/checks/xmlrpc-c-auto_ptr.cpp" CMAKE_FLAGS -DINCLUDE_DIRECTORIES=${XMLRPC_INCLUDE_DIRS} LINK_LIBRARIES ${XMLRPC_LIBRARIES} OUTPUT_VARIABLE TRY_COMPILE_OUTPUT )
    if ( NOT XMLRPC_C_NO_AUTO_PTR )
        message ( WARNING ${TRY_COMPILE_OUTPUT} )
        message ( FATAL_ERROR "Your version of XMLRPC-C library can not be used with C++17. Try upgrading to at least 1.58.0." )
    endif ()
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
    if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
        set_target_properties ( mod_conf_rpc PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
    endif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )

    if ( CLANG_TIDY_EXE )
        set_target_properties ( mod_conf_rpc PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
    endif ()
    if ( INCLUDE_WHAT_YOU_USE_EXE )
        set_target_properties ( mod_conf_rpc PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${DO_INCLUDE_WHAT_YOU_USE}" )
    endif ()
endif ( BUILD_MOD_RPCCONFIG )

if ( BUILD_MOD_PING )
    add_library ( mod_ping MODULE other/ping/ping.cpp )
    target_link_libraries ( mod_ping scriptexecuter logger pinger common )
    set_target_properties ( mod_ping PROPERTIES PREFIX "" )

    if ( CLANG_TIDY_EXE )
        set_target_properties ( mod_ping PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
    endif ()
    if ( INCLUDE_WHAT_YOU_USE_EXE )
        set_target_properties ( mod_ping PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${DO_INCLUDE_WHAT_YOU_USE}" )
    endif ()
endif ( BUILD_MOD_PING )

if ( BUILD_MOD_RSCRYPT )
    add_library ( mod_remote_script MODULE other/rscript/rscript.cpp other/rscript/nrmap_parser.cpp )
    target_link_libraries ( mod_remote_script crypto scriptexecuter logger common )
    set_target_properties ( mod_remote_script PROPERTIES PREFIX "" )
    if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
        set_target_properties ( mod_remote_script PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
    endif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )

    if ( CLANG_TIDY_EXE )
        set_target_properties ( mod_remote_script PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
    endif ()
    if ( INCLUDE_WHAT_YOU_USE_EXE )
        set_target_properties ( mod_remote_script PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${DO_INCLUDE_WHAT_YOU_USE}" )
    endif ()
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

    if ( CLANG_TIDY_EXE )
        set_target_properties ( mod_smux PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
    endif ()
    if ( INCLUDE_WHAT_YOU_USE_EXE )
        set_target_properties ( mod_smux PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${DO_INCLUDE_WHAT_YOU_USE}" )
    endif ()
endif ( BUILD_MOD_SMUX )

if ( BUILD_MOD_STORE_FILES )
    add_library ( mod_store_files MODULE store/files/file_store.cpp )
    target_link_libraries ( mod_store_files crypto conffiles logger common )
    set_target_properties ( mod_store_files PROPERTIES PREFIX "" )

    if ( CLANG_TIDY_EXE )
        set_target_properties ( mod_store_files PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
    endif ()
    if ( INCLUDE_WHAT_YOU_USE_EXE )
        set_target_properties ( mod_store_files PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${DO_INCLUDE_WHAT_YOU_USE}" )
    endif ()
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

    if ( CLANG_TIDY_EXE )
        set_target_properties ( mod_store_firebird PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
    endif ()
    if ( INCLUDE_WHAT_YOU_USE_EXE )
        set_target_properties ( mod_store_firebird PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${DO_INCLUDE_WHAT_YOU_USE}" )
    endif ()
endif ( BUILD_MOD_STORE_FIREBIRD )

if ( BUILD_MOD_STORE_MYSQL )
    find_package ( MySQLConnector REQUIRED )
    add_library ( mod_store_mysql MODULE store/mysql/mysql_store.cpp )
    target_link_libraries ( mod_store_mysql crypto logger common MySQL::Connector )
    set_target_properties ( mod_store_mysql PROPERTIES PREFIX "" )

    if ( CLANG_TIDY_EXE )
        set_target_properties ( mod_store_mysql PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
    endif ()
    if ( INCLUDE_WHAT_YOU_USE_EXE )
        set_target_properties ( mod_store_mysql PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${DO_INCLUDE_WHAT_YOU_USE}" )
    endif ()
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
    if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
        set_target_properties ( mod_store_postgresql PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
    endif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )

    if ( CLANG_TIDY_EXE )
        set_target_properties ( mod_store_postgresql PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
    endif ()
    if ( INCLUDE_WHAT_YOU_USE_EXE )
        set_target_properties ( mod_store_postgresql PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${DO_INCLUDE_WHAT_YOU_USE}" )
    endif ()
endif ( BUILD_MOD_STORE_POSTGRESQL )
