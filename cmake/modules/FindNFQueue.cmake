find_path ( NFQueue_INCLUDE_DIR NAMES libnetfilter_queue/libnetfilter_queue.h DOC "Path to NFQueue header files." )
mark_as_advanced ( NFQueue_INCLUDE_DIR )

find_library ( NFQueue_LIB NAMES netfilter_queue DOC "Location of NFQueue library." )
mark_as_advanced ( NFQueue_LIB )

include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args ( NFQueue
                                    REQUIRED_VARS NFQueue_LIB NFQueue_INCLUDE_DIR
                                    VERSION_VAR NFQueue_VERSION )

# Create the imported target
if ( NFQueue_FOUND )
    set ( NFQueue_INCLUDE_DIRS ${NFQueue_INCLUDE_DIR} )
    set ( NFQueue_LIBRARIES ${NFQueue_LIB} )
    if ( NOT TARGET NF::Queue )
        add_library ( NF::Queue UNKNOWN IMPORTED )
        set_target_properties ( NF::Queue PROPERTIES
                                IMPORTED_LOCATION "${NFQueue_LIB}"
                                INTERFACE_INCLUDE_DIRECTORIES "${NFQueue_INCLUDE_DIR}" )
    endif ( NOT TARGET NF::Queue )
endif ( NFQueue_FOUND )
