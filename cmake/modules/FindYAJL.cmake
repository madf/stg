find_path ( YAJL_INCLUDE_DIR NAMES yajl/yajl_parse.h yajl/yajl_version.h DOC "Path to YAJL header files." )
mark_as_advanced ( YAJL_INCLUDE_DIR )

find_library ( YAJL_LIB NAMES yajl DOC "Location of YAJL library." )
mark_as_advanced ( YAJL_LIB )

if ( YAJL_INCLUDE_DIR )
    file ( READ "${YAJL_INCLUDE_DIR}/yajl/yajl_version.h" ver )

    string ( REGEX MATCH "YAJL_MAJOR ([0-9]*)" _ ${ver} )
    set ( ver_major ${CMAKE_MATCH_1} )

    string ( REGEX MATCH "YAJL_MINOR ([0-9]*)" _ ${ver} )
    set ( ver_minor ${CMAKE_MATCH_1} )

    string ( REGEX MATCH "YAJL_MICRO ([0-9]*)" _ ${ver} )
    set ( ver_micro ${CMAKE_MATCH_1} )

    set ( YAJL_VERSION "${ver_major}.${ver_minor}.${ver_micro}" )

    unset ( ver )
    unset ( ver_major )
    unset ( ver_minor )
    unset ( ver_micro )
endif ( YAJL_INCLUDE_DIR )

include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args ( YAJL
                                    REQUIRED_VARS YAJL_LIB YAJL_INCLUDE_DIR
                                    VERSION_VAR YAJL_VERSION )

# Create the imported target
if ( YAJL_FOUND )
    set ( YAJL_INCLUDE_DIRS ${YAJL_INCLUDE_DIR} )
    set ( YAJL_LIBRARIES ${YAJL_LIB} )
    if ( NOT TARGET YAJL::YAJL )
        add_library ( YAJL::YAJL UNKNOWN IMPORTED )
        set_target_properties ( YAJL::YAJL PROPERTIES
                                IMPORTED_LOCATION "${YAJL_LIB}"
                                INTERFACE_INCLUDE_DIRECTORIES "${YAJL_INCLUDE_DIR}" )
    endif ( NOT TARGET YAJL::YAJL )
endif ( YAJL_FOUND )
