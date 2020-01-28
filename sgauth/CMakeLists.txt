set ( CPP_FILES main.cpp settings_impl.cpp web.cpp )

set ( THREADS_PREFER_PTHREAD_FLAG ON )
find_package ( Threads REQUIRED )
find_package ( Intl REQUIRED )

file ( READ sgauth.css CSS_DATA )
configure_file ( css.h.in css.h ESCAPE_QUOTES @ONLY )

set ( CMAKE_INCLUDE_CURRENT_DIR ON )

include_directories ( ${Intl_INCLUDE_DIRS} )

add_executable ( sgauth ${CPP_FILES} )

target_link_libraries ( sgauth conffiles ia crypto common ${Intl_LIBRARIES} Threads::Threads )

# TODO: install
