find_package ( EXPAT REQUIRED )

set ( CPP_FILES stgconfig.cpp
                conn.cpp
                configproto.cpp
                parser.cpp
                parser_tariffs.cpp
                parser_admins.cpp
                parser_users.cpp
                parser_services.cpp
                parser_message.cpp
                parser_auth_by.cpp
                parser_user_info.cpp
                parser_server_info.cpp )

add_library ( mod_conf_sg MODULE ${CPP_FILES} )

target_link_libraries ( mod_conf_sg scriptexecuter crypto logger common EXPAT::EXPAT )
