set ( CPP_FILES netunit.cpp servconf.cpp )
file ( GLOB PARSERS parsers/*.cpp )

add_library ( srvconf STATIC ${CPP_FILES} ${PARSERS} )

target_include_directories ( srvconf PUBLIC include )

target_link_libraries ( srvconf crypto common )
