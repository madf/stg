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

add_custom_target (cppcheck COMMAND cppcheck --enable=all --std=c++14 ${CMAKE_SOURCE_DIR}/rscriptd ${CMAKE_SOURCE_DIR}/sgauth ${CMAKE_SOURCE_DIR}/sgconf ${CMAKE_SOURCE_DIR}/stargazer)
