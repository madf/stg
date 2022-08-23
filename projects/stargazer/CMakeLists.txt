set ( CPP_FILES main.cpp
                admins_impl.cpp
                users_impl.cpp
                tariffs_impl.cpp
                corps_impl.cpp
                services_impl.cpp
                user_impl.cpp
                tariff_impl.cpp
                eventloop.cpp
                async_pool.cpp
                pidfile.cpp
                plugin_runner.cpp
                plugin_mgr.cpp
                settings_impl.cpp
                stg_timer.cpp
                store_loader.cpp
                traffcounter_impl.cpp
                user_property.cpp )

set ( THREADS_PREFER_PTHREAD_FLAG ON )
find_package ( Threads REQUIRED )

add_executable ( stargazer ${CPP_FILES} )

target_link_libraries ( stargazer scriptexecuter dotconfpp logger common Threads::Threads ${CMAKE_DL_LIBS} )

if ( CLANG_TIDY_EXE )
    set_target_properties ( stargazer PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
endif ()
if ( INCLUDE_WHAT_YOU_USE_EXE )
    set_target_properties ( stargazer PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${DO_INCLUDE_WHAT_YOU_USE}" )
endif ()

add_subdirectory ( plugins )

# TODO: install
