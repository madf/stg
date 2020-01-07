set ( CPP_FILES all_in_one.cpp )

add_library ( ibpp STATIC ${CPP_FILES} )

target_include_directories ( ibpp PUBLIC include )
