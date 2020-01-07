find_package ( EXPAT REQUIRED )

set ( CPP_FILES main.cpp options.cpp api_action.cpp actions.cpp admins.cpp tariffs.cpp users.cpp services.cpp corps.cpp info.cpp xml.cpp )

set ( THREADS_PREFER_PTHREAD_FLAG ON )
find_package ( Threads REQUIRED )

add_executable ( sgconf ${CPP_FILES} )

target_link_libraries ( sgconf srvconf crypto common EXPAT::EXPAT )

# TODO: install
