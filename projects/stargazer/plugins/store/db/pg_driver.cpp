#include <sstream>

#include "pg_driver.h"

BASE_DB * CreateDriver()
{
    return new PG_DRIVER();
}

void DestroyDriver(BASE_DB * drv)
{
    delete drv;
}

PG_DRIVER::~PG_DRIVER()
{
    if (conn != NULL)
	PQfinish(conn);
}

bool PG_DRIVER::Connect()
{
    std::stringstream params;
    params << "host=" << host
	   << "dbname=" << database
	   << "user=" << user
	   << "password=" << password;
    std::string str = params.str();
    conn = PQconnectdb(str.c_str());
    errorMsg = PQerrorMessage(conn);
    return PQstatus(conn) != CONNECTION_OK;
}

bool PG_DRIVER::Disconnect()
{
    if (PQstatus(conn) == CONNECTION_OK) {
	PQfinish(conn);
	errorMsg = PQerrorMessage(conn);
	return PQstatus(conn) != CONNECTION_BAD;
    }

    return false;
}

bool PG_DRIVER::Query(const std::string & query)
{
    cols.erase(cols.begin(), cols.end());
    cols.reserve(columns);

    PQclear(result);
    result = PQexec(conn, query.c_str());
    errorMsg = PQerrorMessage(conn);
    tuples = PQntuples(result);
    columns = PQnfields(result);
    affected = atoi(PQcmdTuples(result));

    if (tuples) {
	for (int i = 0; i < columns; ++i)
	    cols.push_back(PQfname(result, i));
    }

    if (!result)
	return true;

    if (PQresultStatus(result) == PGRES_COMMAND_OK)
	return false;

    if (PQresultStatus(result) == PGRES_TUPLES_OK)
	return false;

    return true;
}

bool PG_DRIVER::Start()
{
    return Query("BEGIN");
}

bool PG_DRIVER::Commit()
{
    return Query("COMMIT");
}

bool PG_DRIVER::Rollback()
{
    return Query("ROLLBACK");
}

BASE_DB::TUPLE PG_DRIVER::GetTuple(int n) const
{
    BASE_DB::TUPLE tuple;

    for (int i = 0; i < columns; ++i)
	tuple[cols[i]] = PQgetvalue(result, n, i);

    return tuple;
}

BASE_DB::TUPLES PG_DRIVER::GetResult() const
{
    BASE_DB::TUPLES tpls;

    for (int i = 0; i < tuples; ++i)
	tpls.push_back(GetTuple(i));

    return tpls;
}
