set ( CPP_FILES inetaccess.cpp )

add_library ( mod_auth_ia MODULE ${CPP_FILES} )

target_link_libraries ( mod_auth_ia scriptexecuter crypto logger common )
