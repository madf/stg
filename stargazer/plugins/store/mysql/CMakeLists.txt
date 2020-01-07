find_package ( MySQLConnector REQUIRED )

set ( CPP_FILES mysql_store.cpp )

add_library ( mod_store_mysql MODULE ${CPP_FILES} )

target_link_libraries ( mod_store_mysql crypto logger common MySQL::Connector )
