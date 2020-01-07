find_package ( PostgreSQL REQUIRED )

set ( CPP_FILES postgresql_store.cpp
                postgresql_store_admins.cpp
                postgresql_store_corporations.cpp
                postgresql_store_messages.cpp
                postgresql_store_services.cpp
                postgresql_store_tariffs.cpp
                postgresql_store_users.cpp
                postgresql_store_utils.cpp )

add_library ( mod_store_postgresql MODULE ${CPP_FILES} )

target_link_libraries ( mod_store_postgresql crypto logger common PostgreSQL::PostgreSQL )
