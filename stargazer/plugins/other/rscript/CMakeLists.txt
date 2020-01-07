set ( CPP_FILES rscript.cpp nrmap_parser.cpp )

add_library ( mod_remote_script MODULE ${CPP_FILES} )

target_link_libraries ( mod_remote_script crypto scriptexecuter logger common )
