find_package ( YAJL REQUIRED )

set ( CPP_FILES radius.cpp
                config.cpp
                conn.cpp )

add_library ( mod_radius MODULE ${CPP_FILES} )

target_link_libraries ( mod_radius logger json common YAJL::YAJL )
