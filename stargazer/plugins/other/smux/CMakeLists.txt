set ( CPP_FILES smux.cpp
                sensors.cpp
                tables.cpp
                handlers.cpp
                utils.cpp
                types.cpp )

add_library ( mod_smux MODULE ${CPP_FILES} )

target_link_libraries ( mod_smux scriptexecuter logger smux common )
