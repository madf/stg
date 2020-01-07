find_path ( MNL_INCLUDE_DIR NAMES libmnl/libmnl.h DOC "Path to MNL header files." )
mark_as_advanced ( MNL_INCLUDE_DIR )

find_library ( MNL_LIB NAMES mnl DOC "Location of MNL library." )
mark_as_advanced ( MNL_LIB )

include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args ( MNL
                                    REQUIRED_VARS MNL_LIB MNL_INCLUDE_DIR
                                    VERSION_VAR MNL_VERSION )

# Create the imported target
if ( MNL_FOUND )
    set ( MNL_INCLUDE_DIRS ${MNL_INCLUDE_DIR} )
    set ( MNL_LIBRARIES ${MNL_LIB} )
    if ( NOT TARGET MNL::MNL )
        add_library ( MNL::MNL UNKNOWN IMPORTED )
        set_target_properties ( MNL::MNL PROPERTIES
                                IMPORTED_LOCATION "${MNL_LIB}"
                                INTERFACE_INCLUDE_DIRECTORIES "${MNL_INCLUDE_DIR}" )
    endif ( NOT TARGET MNL::MNL )
endif ( MNL_FOUND )
