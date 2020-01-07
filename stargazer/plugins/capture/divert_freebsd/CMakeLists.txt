set ( CPP_FILES divert_cap.cpp )

add_library ( mod_cap_divert MODULE ${CPP_FILES} )

target_link_libraries ( mod_cap_divert logger common )
