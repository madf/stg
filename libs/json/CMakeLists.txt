set ( CPP_FILES generator.cpp parser.cpp )

add_library ( json STATIC ${CPP_FILES} )

target_include_directories ( json PUBLIC include )

target_link_libraries ( json common )
