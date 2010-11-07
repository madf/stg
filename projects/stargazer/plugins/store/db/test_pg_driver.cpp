#include <iostream>
#include <string>
#include <sstream>

#include <dlfcn.h>

#include "base_db.h"

int main(int argc, char ** argv)
{
    BASE_DB * db;

    void * lh = dlopen("./pg_driver.so", RTLD_NOW | RTLD_GLOBAL);

    if (lh == NULL) {
	std::cout << "Error loading shared object file pg_driver.so. Reason: '" << dlerror() << "'" << std::endl;
	return EXIT_FAILURE;
    }

    CreateDriverFn CreateDriver = reinterpret_cast<CreateDriverFn>(dlsym(lh, "CreateDriver"));
    if (CreateDriver == NULL) {
	std::cout << "Error getting symbol 'CreateDriver' address. Reason: '" << dlerror() << "'" << std::endl;
	dlclose(lh);
	return EXIT_FAILURE;
    }
    DestroyDriverFn DestroyDriver = reinterpret_cast<DestroyDriverFn>(dlsym(lh, "DestroyDriver"));
    if (DestroyDriver == NULL) {
	std::cout << "Error getting symbol 'DestroyDriver' address. Reason: '" << dlerror() << "'" << std::endl;
	dlclose(lh);
	return EXIT_FAILURE;
    }

    db = CreateDriver();

    db->SetHost("localhost");
    db->SetDatabase("stargazer");
    db->SetUser("stg");
    db->SetPassword("123456");

    if (db->Connect()) {
	std::cout << "Error connecting db. Reason: '" << db->GetErrorMsg() << "'" << std::endl;
	DestroyDriver(db);
	dlclose(lh);
	return EXIT_FAILURE;
    }

    std::stringstream query;
    query << "SELECT * FROM information_schema.tables";

    if (db->Query(query.str())) {
	std::cout << "Error querying db. Reason: '" << db->GetErrorMsg() << "'" << std::endl;
	db->Disconnect();
	DestroyDriver(db);
	dlclose(lh);
	return EXIT_FAILURE;
    }

    std::cout << "Tuples: " << db->GetTuples() << std::endl;
    std::cout << "Columns: " << db->GetColumns() << std::endl;
    BASE_DB::COLUMNS cols;
    BASE_DB::COLUMNS::iterator it;
    cols = db->GetColumnsNames();
    std::cout << "Cols count: " << cols.size() << std::endl;
    std::cout << "Columns names:" << std::endl;
    for (it = cols.begin(); it != cols.end(); ++it)
	std::cout << *it << " ";
    std::cout << std::endl;

    for (int i = 0; i < db->GetTuples(); ++i) {
	BASE_DB::TUPLE tuple(db->GetTuple(i));
	BASE_DB::TUPLE::iterator it;
	for (it = tuple.begin(); it != tuple.end(); ++it)
	    std::cout << it->second << " ";
	std::cout << std::endl;
    }

    query.str("");
    query << "create table test ( id bigserial, message text )";
    if (db->Query(query.str())) {
	std::cout << "Error querying db. Reason: '" << db->GetErrorMsg() << "'" << std::endl;
	db->Disconnect();
	DestroyDriver(db);
	dlclose(lh);
	return EXIT_FAILURE;
    }

    query.str("");
    query << "insert into test (message) values ('abc');";
    query << "insert into test (message) values ('def');";
    query << "insert into test (message) values ('zxc');";
    if (db->Query(query.str())) {
	std::cout << "Error querying db. Reason: '" << db->GetErrorMsg() << "'" << std::endl;
	db->Disconnect();
	DestroyDriver(db);
	dlclose(lh);
	return EXIT_FAILURE;
    }

    query.str("");
    query << "SELECT * FROM test";
    if (db->Query(query.str())) {
	std::cout << "Error querying db. Reason: '" << db->GetErrorMsg() << "'" << std::endl;
	db->Disconnect();
	DestroyDriver(db);
	dlclose(lh);
	return EXIT_FAILURE;
    }
    std::cout << "Tuples: " << db->GetTuples() << std::endl;
    std::cout << "Columns: " << db->GetColumns() << std::endl;
    cols = db->GetColumnsNames();
    std::cout << "Cols count: " << cols.size() << std::endl;
    std::cout << "Columns names:" << std::endl;
    for (it = cols.begin(); it != cols.end(); ++it)
	std::cout << *it << " ";
    std::cout << std::endl;

    for (int i = 0; i < db->GetTuples(); ++i) {
	BASE_DB::TUPLE tuple(db->GetTuple(i));
	BASE_DB::TUPLE::iterator it;
	for (it = tuple.begin(); it != tuple.end(); ++it)
	    std::cout << it->second << " ";
	std::cout << std::endl;
    }

    query.str("");
    query << "drop table test";
    if (db->Query(query.str())) {
	std::cout << "Error querying db. Reason: '" << db->GetErrorMsg() << "'" << std::endl;
	db->Disconnect();
	DestroyDriver(db);
	dlclose(lh);
	return EXIT_FAILURE;
    }

    if (db->Disconnect()) {
	std::cout << "Error connecting db. Reason: '" << db->GetErrorMsg() << "'" << std::endl;
	DestroyDriver(db);
	dlclose(lh);
	return EXIT_FAILURE;
    }

    DestroyDriver(db);

    dlclose(lh);

    return EXIT_SUCCESS;
}
