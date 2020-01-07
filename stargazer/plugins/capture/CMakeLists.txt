if ( BUILD_MOD_CAP_NF )
    add_subdirectory ( cap_nf )
endif ( BUILD_MOD_CAP_NF )

if ( BUILD_MOD_CAP_ETHER_LINUX )
    add_subdirectory ( ether_linux )
endif ( BUILD_MOD_CAP_ETHER_LINUX )

if ( BUILD_MOD_CAP_ETHER_FREEBSD )
    add_subdirectory ( ether_freebsd )
endif ( BUILD_MOD_CAP_ETHER_FREEBSD )

if ( BUILD_MOD_CAP_DIVERT )
    add_subdirectory ( divert_freebsd )
endif ( BUILD_MOD_CAP_DIVERT )

if ( BUILD_MOD_CAP_PCAP )
    add_subdirectory ( pcap )
endif ( BUILD_MOD_CAP_PCAP )

if ( BUILD_MOD_CAP_NFQUEUE )
    add_subdirectory ( nfqueue )
endif ( BUILD_MOD_CAP_NFQUEUE )
