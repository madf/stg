set ( CPP_FILES pinger.cpp )

add_library ( pinger STATIC ${CPP_FILES} )

target_include_directories ( pinger PUBLIC include )

target_link_libraries ( pinger common )
