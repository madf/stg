set ( CPP_FILES conffiles.cpp )

add_library ( conffiles STATIC ${CPP_FILES} )

target_include_directories ( conffiles PUBLIC include )
