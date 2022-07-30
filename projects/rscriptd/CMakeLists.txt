set ( CPP_FILES main.cpp listener.cpp pidfile.cpp )

set ( THREADS_PREFER_PTHREAD_FLAG ON )
find_package ( Threads REQUIRED )

add_executable ( rscriptd ${CPP_FILES} )

target_link_libraries ( rscriptd scriptexecuter conffiles logger crypto common Threads::Threads )

if ( CLANG_TIDY_EXE )
    set_target_properties ( rscriptd PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}" )
endif ()
if ( INCLUDE_WHAT_YOU_USE_EXE )
    set_target_properties ( rscriptd PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${DO_INCLUDE_WHAT_YOU_USE}" )
endif ()

# TODO: install
