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

add_executable ( test_conffiles test_conffiles.cpp )
target_link_libraries ( test_conffiles conffiles Boost::unit_test_framework )
add_test ( stgconffiles test_conffiles )

add_executable ( test_fee_charge_rules test_fee_charge_rules.cpp ../projects/stargazer/user_impl.cpp ../projects/stargazer/tariff_impl.cpp ../projects/stargazer/user_property.cpp )
target_link_libraries ( test_fee_charge_rules logger scriptexecuter common Boost::unit_test_framework )
target_include_directories ( test_fee_charge_rules PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../projects/stargazer )
add_test ( fee_charge_rules test_fee_charge_rules )

add_executable ( test_reconnect_on_tariff_change test_reconnect_on_tariff_change.cpp ../projects/stargazer/user_impl.cpp ../projects/stargazer/tariff_impl.cpp ../projects/stargazer/user_property.cpp )
target_link_libraries ( test_reconnect_on_tariff_change logger scriptexecuter common Boost::unit_test_framework )
target_include_directories ( test_reconnect_on_tariff_change PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../projects/stargazer )
add_test ( reconnect_on_tariff_change test_reconnect_on_tariff_change )

add_executable ( test_disable_session_log test_disable_session_log.cpp ../projects/stargazer/user_impl.cpp ../projects/stargazer/tariff_impl.cpp ../projects/stargazer/user_property.cpp )
target_link_libraries ( test_disable_session_log logger scriptexecuter common Boost::unit_test_framework )
target_include_directories ( test_disable_session_log PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../projects/stargazer )
add_test ( disable_session_log test_disable_session_log )

add_executable ( test_filter_params_log test_filter_params_log.cpp ../projects/stargazer/user_impl.cpp ../projects/stargazer/tariff_impl.cpp ../projects/stargazer/user_property.cpp )
target_link_libraries ( test_filter_params_log logger scriptexecuter common Boost::unit_test_framework )
target_include_directories ( test_filter_params_log PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../projects/stargazer )
add_test ( filter_params_log test_filter_params_log )
