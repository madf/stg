set ( CPP_FILES main.cpp
                test_conffiles.cpp
                test_fee_charge_rules.cpp
                test_reconnect_on_tariff_change.cpp
                test_disable_session_log.cpp
                test_filter_params_log.cpp
                ../projects/stargazer/tariff_impl.cpp
                ../projects/stargazer/user_impl.cpp
                ../projects/stargazer/user_property.cpp )

set ( THREADS_PREFER_PTHREAD_FLAG ON )
find_package ( Threads REQUIRED )
find_package ( Boost REQUIRED unit_test_framework )

add_definitions ( -DBOOST_TEST_DYN_LINK )
add_definitions ( -DUSE_ABSTRACT_SETTINGS )

add_executable ( test_admin_conf test_admin_conf.cpp )
target_link_libraries ( test_admin_conf Boost::unit_test_framework )
add_test ( admin_conf test_admin_conf )

add_executable ( test_raw_ip test_raw_ip.cpp )
target_link_libraries ( test_raw_ip Boost::unit_test_framework )
add_test ( raw_ip test_raw_ip )

add_executable ( test_bfstream test_bfstream.cpp )
target_link_libraries ( test_bfstream crypto Boost::unit_test_framework )
add_test ( bfstream test_bfstream )

add_executable ( test_crypto test_crypto.cpp )
target_link_libraries ( test_crypto crypto Boost::unit_test_framework )
add_test ( stgcrypto test_crypto )

add_executable ( test_tariff test_tariff.cpp ../projects/stargazer/tariff_impl.cpp )
target_link_libraries ( test_tariff crypto common Boost::unit_test_framework )
target_include_directories ( test_tariff PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../projects/stargazer )
add_test ( tariff test_tariff )

add_executable ( tests ${CPP_FILES} )

target_link_libraries ( tests conffiles crypto logger scriptexecuter common Threads::Threads )

target_include_directories ( tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../projects/stargazer )

add_test ( tests tests )
