find_package ( XMLRPC REQUIRED c++ abyss-server )

set ( CPP_FILES rpcconfig.cpp
                user_helper.cpp
                tariff_helper.cpp
                info_methods.cpp
                users_methods.cpp
                tariffs_methods.cpp
                admins_methods.cpp
                messages_methods.cpp )

add_library ( mod_conf_rpc MODULE ${CPP_FILES} )

target_link_libraries ( mod_conf_rpc scriptexecuter logger common xmlrpc_server_abyss++ )
