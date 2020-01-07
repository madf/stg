file ( GLOB C_FILES *.c )

add_library ( smux STATIC ${C_FILES} )

target_include_directories ( smux PUBLIC include include/stg )
