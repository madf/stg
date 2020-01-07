if ( BUILD_MOD_PING )
    add_subdirectory ( ping )
endif ( BUILD_MOD_PING )

if ( BUILD_MOD_RADIUS )
    add_subdirectory ( radius )
endif ( BUILD_MOD_RADIUS )

if ( BUILD_MOD_RSCRYPT )
    add_subdirectory ( rscript )
endif ( BUILD_MOD_RSCRYPT )

if ( BUILD_MOD_SMUX )
    add_subdirectory ( smux )
endif ( BUILD_MOD_SMUX )
