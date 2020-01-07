set ( CPP_FILES logger.cpp )

add_library ( logger STATIC ${CPP_FILES} )

target_include_directories ( logger PUBLIC include )
