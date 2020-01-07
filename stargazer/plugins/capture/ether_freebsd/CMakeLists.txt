set ( CPP_FILES ether_cap.cpp )

add_library ( mod_cap_bpf MODULE ${CPP_FILES} )

target_link_libraries ( mod_cap_bpf logger common )
