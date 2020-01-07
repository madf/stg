set ( CPP_FILES blockio.cpp common.cpp strptime.cpp )

include_directories ( include )

add_library ( common STATIC ${CPP_FILES} )

target_include_directories ( common PUBLIC include )
