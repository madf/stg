set ( C_FILES scriptexecuter.c )

add_library ( scriptexecuter STATIC ${C_FILES} )

target_include_directories ( scriptexecuter PUBLIC include )
