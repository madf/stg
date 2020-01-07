find_path ( PCap_INCLUDE_DIR NAMES pcap.h pcap/pcap.h DOC "Path to PCap header files." )
mark_as_advanced ( PCap_INCLUDE_DIR )

find_library ( PCap_LIB NAMES pcap DOC "Location of PCap library." )
mark_as_advanced ( PCap_LIB )

if ( PCap_INCLUDE_DIR )
    file ( READ "${PCap_INCLUDE_DIR}/pcap/pcap.h" ver )

    string ( REGEX MATCH "PCAP_VERSION_MAJOR ([0-9]*)" _ ${ver} )
    set ( ver_major ${CMAKE_MATCH_1} )

    string ( REGEX MATCH "PCAP_VERSION_MINOR ([0-9]*)" _ ${ver} )
    set ( ver_minor ${CMAKE_MATCH_1} )

    set ( PCap_VERSION "${ver_major}.${ver_minor}" )

    unset ( ver )
    unset ( ver_major )
    unset ( ver_minor )
endif ( PCap_INCLUDE_DIR )

include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args ( PCap
                                    REQUIRED_VARS PCap_LIB PCap_INCLUDE_DIR
                                    VERSION_VAR PCap_VERSION )

# Create the imported target
if ( PCap_FOUND )
    set ( PCap_INCLUDE_DIRS ${PCap_INCLUDE_DIR} )
    set ( PCap_LIBRARIES ${PCap_LIB} )
    if ( NOT TARGET PCap::PCap )
        add_library ( PCap::PCap UNKNOWN IMPORTED )
        set_target_properties ( PCap::PCap PROPERTIES
                                IMPORTED_LOCATION "${PCap_LIB}"
                                INTERFACE_INCLUDE_DIRECTORIES "${PCap_INCLUDE_DIR}" )
    endif ( NOT TARGET PCap::PCap )
endif ( PCap_FOUND )
