find_package ( NFQueue REQUIRED )
find_package ( NFNetLink REQUIRED )
find_package ( MNL REQUIRED )

set ( CPP_FILES nfqueue.cpp )

add_library ( mod_cap_nfqueue MODULE ${CPP_FILES} )

target_link_libraries ( mod_cap_nfqueue logger common NF::Queue NF::NetLink MNL::MNL )
