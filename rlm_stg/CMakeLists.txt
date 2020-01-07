find_package ( YAJL REQUIRED )

set ( CPP_FILES conn.cpp iface.cpp stg_client.cpp )
set ( C_FILES rlm_stg.c radlog.c )

set ( THREADS_PREFER_PTHREAD_FLAG ON )
find_package ( Threads REQUIRED )

add_library ( rlm_stg MODULE ${CPP_FILES} ${C_FILES} )

target_link_libraries ( rlm_stg crypto common json dl YAJL::YAJL )

# TODO: install
