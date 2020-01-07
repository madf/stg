if ( BUILD_MOD_STORE_FILES )
    add_subdirectory ( files )
endif ( BUILD_MOD_STORE_FILES )

if ( BUILD_MOD_STORE_FIREBIRD )
    add_subdirectory ( firebird )
endif ( BUILD_MOD_STORE_FIREBIRD )

if ( BUILD_MOD_STORE_MYSQL )
    add_subdirectory ( mysql )
endif ( BUILD_MOD_STORE_MYSQL )

if ( BUILD_MOD_STORE_POSTGRESQL )
    add_subdirectory ( postgresql )
endif ( BUILD_MOD_STORE_POSTGRESQL )
