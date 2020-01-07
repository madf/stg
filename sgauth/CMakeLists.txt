set ( CPP_FILES main.cpp settings_impl.cpp web.cpp )

set ( THREADS_PREFER_PTHREAD_FLAG ON )
find_package ( Threads REQUIRED )

add_executable ( sgauth ${CPP_FILES} )

target_link_libraries ( sgauth conffiles ia crypto common Threads::Threads )

# TODO: install
