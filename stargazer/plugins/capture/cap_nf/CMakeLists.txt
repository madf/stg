set ( CPP_FILES cap_nf.cpp )

add_library ( mod_cap_nf MODULE ${CPP_FILES} )

target_link_libraries ( mod_cap_nf logger common )
