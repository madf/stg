find_path ( MySQLConnector_INCLUDE_DIR NAMES mysql/mysql.h mysql/mysql_version.h DOC "Path to MySQLConnector header files." )
mark_as_advanced ( MySQLConnector_INCLUDE_DIR )

find_library ( MySQLConnector_LIB NAMES mysqlclient DOC "Location of MySQLConnector library." )
mark_as_advanced ( MySQLConnector_LIB )

if ( MySQLConnector_INCLUDE_DIR )
    file ( READ "${MySQLConnector_INCLUDE_DIR}/mysql/mysql_version.h" ver )

    string ( REGEX MATCH "LIBMYSQL_VERSION ([0-9]*)" _ ${ver} )

    set ( MySQLConnector_VERSION ${CMAKE_MATCH_1} )

    unset ( ver )
endif ( MySQLConnector_INCLUDE_DIR )

include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args ( MySQLConnector
                                    REQUIRED_VARS MySQLConnector_LIB MySQLConnector_INCLUDE_DIR
                                    VERSION_VAR MySQLConnector_VERSION )

# Create the imported target
if ( MySQLConnector_FOUND )
    set ( MySQLConnector_INCLUDE_DIRS ${MySQLConnector_INCLUDE_DIR} )
    set ( MySQLConnector_LIBRARIES ${MySQLConnector_LIB} )
    if ( NOT TARGET MySQL::Connector )
        add_library ( MySQL::Connector UNKNOWN IMPORTED )
        set_target_properties ( MySQL::Connector PROPERTIES
                                IMPORTED_LOCATION "${MySQLConnector_LIB}"
                                INTERFACE_INCLUDE_DIRECTORIES "${MySQLConnector_INCLUDE_DIR}" )
    endif ( NOT TARGET MySQL::Connector )
endif ( MySQLConnector_FOUND )
