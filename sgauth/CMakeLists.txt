set ( CPP_FILES main.cpp settings_impl.cpp web.cpp )

set ( THREADS_PREFER_PTHREAD_FLAG ON )
find_package ( Threads REQUIRED )

file ( READ sgauth.css CSS_DATA )
configure_file ( css.h.in css.h ESCAPE_QUOTES @ONLY )

set ( CMAKE_INCLUDE_CURRENT_DIR ON )

add_executable ( sgauth ${CPP_FILES} )

target_link_libraries ( sgauth conffiles ia crypto common Threads::Threads )

# TODO: install
