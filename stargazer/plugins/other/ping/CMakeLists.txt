set ( CPP_FILES ping.cpp )

add_library ( mod_ping MODULE ${CPP_FILES} )

target_link_libraries ( mod_ping scriptexecuter logger pinger common )
