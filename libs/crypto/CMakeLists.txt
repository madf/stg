set ( CPP_FILES bfstream.cpp )
set ( C_FILES ag_md5.c blowfish.c )

add_library ( crypto STATIC ${CPP_FILES} ${C_FILES} )

target_include_directories ( crypto PUBLIC include )
