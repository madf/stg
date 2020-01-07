set ( CMAKE_POSITION_INDEPENDENT_CODE ON )

if ( BUILD_LIB_COMMON )
    add_subdirectory (common)
endif ( BUILD_LIB_COMMON )

if ( BUILD_LIB_CONFFILES )
    add_subdirectory (conffiles)
endif ( BUILD_LIB_CONFFILES )

if ( BUILD_LIB_CRYPTO )
    add_subdirectory (crypto)
endif ( BUILD_LIB_CRYPTO )

if ( BUILD_LIB_DOTCONFPP )
    add_subdirectory (dotconfpp)
endif ( BUILD_LIB_DOTCONFPP )

if ( BUILD_LIB_IA )
    add_subdirectory (ia)
endif ( BUILD_LIB_IA )

if ( BUILD_LIB_IBPP )
    add_subdirectory (ibpp)
endif ( BUILD_LIB_IBPP )

if ( BUILD_LIB_JSON )
    add_subdirectory (json)
endif ( BUILD_LIB_JSON )

if ( BUILD_LIB_LOGGER )
    add_subdirectory (logger)
endif ( BUILD_LIB_LOGGER )

if ( BUILD_LIB_PINGER )
    add_subdirectory (pinger)
endif ( BUILD_LIB_PINGER )

if ( BUILD_LIB_SCRIPTEXECUTER )
    add_subdirectory (scriptexecuter)
endif ( BUILD_LIB_SCRIPTEXECUTER )

if ( BUILD_LIB_SMUX )
    add_subdirectory (smux)
endif ( BUILD_LIB_SMUX )

if ( BUILD_LIB_SRVCONF )
    add_subdirectory (srvconf)
endif ( BUILD_LIB_SRVCONF )
