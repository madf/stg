set ( CPP_FILES main.cpp listener.cpp pidfile.cpp )

set ( THREADS_PREFER_PTHREAD_FLAG ON )
find_package ( Threads REQUIRED )

add_executable ( rscriptd ${CPP_FILES} )

target_link_libraries ( rscriptd scriptexecuter conffiles logger crypto common Threads::Threads )

# TODO: install
