set ( CPP_FILES main.cpp
                test_raw_ip.cpp
                test_admin_conf.cpp
                test_tariff.cpp
                test_conffiles.cpp
                test_fee_charge_rules.cpp
                test_reconnect_on_tariff_change.cpp
                test_disable_session_log.cpp
                test_filter_params_log.cpp
                test_crypto.cpp
                test_bfstream.cpp
                ../projects/stargazer/tariff_impl.cpp
                ../projects/stargazer/user_impl.cpp
                ../projects/stargazer/user_property.cpp )

set ( THREADS_PREFER_PTHREAD_FLAG ON )
find_package ( Threads REQUIRED )

add_definitions ( -DUSE_ABSTRACT_SETTINGS )

add_executable ( tests ${CPP_FILES} )

target_link_libraries ( tests conffiles crypto logger scriptexecuter common Threads::Threads )

target_include_directories ( tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../projects/stargazer )
