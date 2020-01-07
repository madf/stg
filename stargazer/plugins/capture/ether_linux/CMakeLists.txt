set ( CPP_FILES ether_cap.cpp )

add_library ( mod_cap_ether MODULE ${CPP_FILES} )

target_link_libraries ( mod_cap_ether logger common )
