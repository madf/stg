set ( CPP_FILES file_store.cpp )

add_library ( mod_store_files MODULE ${CPP_FILES} )

target_link_libraries ( mod_store_files crypto conffiles logger common )
