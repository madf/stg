set ( CPP_FILES ia.cpp )

add_library ( ia STATIC ${CPP_FILES} )

target_include_directories ( ia PUBLIC include )

target_link_libraries ( ia crypto common )
