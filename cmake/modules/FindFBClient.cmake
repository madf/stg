find_path ( FBClient_INCLUDE_DIR NAMES ibase.h DOC "Path to FBClient header files." )
mark_as_advanced ( FBClient_INCLUDE_DIR )

find_library ( FBClient_LIB NAMES fbclient DOC "Location of FBClient library." )
mark_as_advanced ( FBClient_LIB )

include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args ( FBClient
                                    REQUIRED_VARS FBClient_LIB FBClient_INCLUDE_DIR
                                    VERSION_VAR FBClient_VERSION )

# Create the imported target
if ( FBClient_FOUND )
    set ( FBClient_INCLUDE_DIRS ${FBClient_INCLUDE_DIR} )
    set ( FBClient_LIBRARIES ${FBClient_LIB} )
    if ( NOT TARGET FBClient::FBClient )
        add_library ( FBClient::FBClient UNKNOWN IMPORTED )
        set_target_properties ( FBClient::FBClient PROPERTIES
                                IMPORTED_LOCATION "${FBClient_LIB}"
                                INTERFACE_INCLUDE_DIRECTORIES "${FBClient_INCLUDE_DIR}" )
    endif ( NOT TARGET FBClient::FBClient )
endif ( FBClient_FOUND )
