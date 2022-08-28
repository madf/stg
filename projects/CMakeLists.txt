if ( BUILD_RSCRIPTD )
    add_subdirectory ( rscriptd )
endif ( BUILD_RSCRIPTD )

if ( BUILD_SGAUTH )
    add_subdirectory ( sgauth )
endif ( BUILD_SGAUTH )

if ( BUILD_SGCONF )
    add_subdirectory ( sgconf )
endif ( BUILD_SGCONF )

if ( BUILD_STG )
    add_subdirectory ( stargazer )
endif ( BUILD_STG )

set ( LIB_DIRS ${CMAKE_SOURCE_DIR}/libs/common
               ${CMAKE_SOURCE_DIR}/libs/conffiles
               ${CMAKE_SOURCE_DIR}/libs/crypto
               ${CMAKE_SOURCE_DIR}/libs/ia
               ${CMAKE_SOURCE_DIR}/libs/logger
               ${CMAKE_SOURCE_DIR}/libs/pinger
               ${CMAKE_SOURCE_DIR}/libs/scriptexecuter
               ${CMAKE_SOURCE_DIR}/libs/srvconf
    )

if (CPPCHECK_FOREIGN)
    set ( LIB_DIRS ${LIB_DIRS}
                   ${CMAKE_SOURCE_DIR}/libs/dotconfpp
                   ${CMAKE_SOURCE_DIR}/libs/ibpp
                   ${CMAKE_SOURCE_DIR}/libs/smux
        )
endif ()

add_custom_target (cppcheck COMMAND /usr/bin/cppcheck --enable=all --std=c++17 ${LIB_DIRS} ${CMAKE_SOURCE_DIR}/projects/rscriptd ${CMAKE_SOURCE_DIR}/projects/sgauth ${CMAKE_SOURCE_DIR}/projects/sgconf ${CMAKE_SOURCE_DIR}/projects/stargazer)
