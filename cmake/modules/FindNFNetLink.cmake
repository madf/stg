find_path ( NFNetLink_INCLUDE_DIR NAMES libnfnetlink/libnfnetlink.h DOC "Path to NFNetLink header files." )
mark_as_advanced ( NFNetLink_INCLUDE_DIR )

find_library ( NFNetLink_LIB NAMES nfnetlink DOC "Location of NFNetLink library." )
mark_as_advanced ( NFNetLink_LIB )

include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args ( NFNetLink
                                    REQUIRED_VARS NFNetLink_LIB NFNetLink_INCLUDE_DIR
                                    VERSION_VAR NFNetLink_VERSION )

# Create the imported target
if ( NFNetLink_FOUND )
    set ( NFNetLink_INCLUDE_DIRS ${NFNetLink_INCLUDE_DIR} )
    set ( NFNetLink_LIBRARIES ${NFNetLink_LIB} )
    if ( NOT TARGET NF::NetLink )
        add_library ( NF::NetLink UNKNOWN IMPORTED )
        set_target_properties ( NF::NetLink PROPERTIES
                                IMPORTED_LOCATION "${NFNetLink_LIB}"
                                INTERFACE_INCLUDE_DIRECTORIES "${NFNetLink_INCLUDE_DIR}" )
    endif ( NOT TARGET NF::NetLink )
endif ( NFNetLink_FOUND )
