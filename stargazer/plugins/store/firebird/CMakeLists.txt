find_package ( FBClient REQUIRED )

set ( CPP_FILES firebird_store.cpp
                firebird_store_admins.cpp
                firebird_store_corporations.cpp
                firebird_store_messages.cpp
                firebird_store_services.cpp
                firebird_store_tariffs.cpp
                firebird_store_users.cpp
                firebird_store_utils.cpp )

add_library ( mod_store_firebird MODULE ${CPP_FILES} )

target_link_libraries ( mod_store_firebird crypto common logger ibpp FBClient::FBClient )
