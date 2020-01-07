set ( CPP_FILES ao.cpp )

add_library ( mod_auth_ao MODULE ${CPP_FILES} )

target_link_libraries ( mod_auth_ao scriptexecuter logger common )
