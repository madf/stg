set ( CPP_FILES dotconfpp.cpp mempool.cpp )

add_library ( dotconfpp STATIC ${CPP_FILES} )

target_include_directories ( dotconfpp PUBLIC include )
