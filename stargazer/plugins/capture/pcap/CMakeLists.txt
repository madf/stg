find_package ( PCap REQUIRED )

set ( CPP_FILES pcap_cap.cpp )

add_library ( mod_cap_pcap MODULE ${CPP_FILES} )

target_link_libraries ( mod_cap_pcap logger common PCap::PCap )
