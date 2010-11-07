#ifndef __PG_DRIVER_H__
#define __PG_DRIVER_H__

#include <libpq-fe.h>

#include "base_db.h"

class PG_DRIVER : public BASE_DB {
public:
    virtual ~PG_DRIVER();

    virtual bool Connect();
    virtual bool Disconnect();
    virtual bool Query(const std::string &);
    virtual bool Start();
    virtual bool Commit();
    virtual bool Rollback();

    virtual BASE_DB::TUPLES GetResult() const;
    virtual BASE_DB::TUPLE GetTuple(int n = 0) const;

private:
    PGconn * conn;
    PGresult * result;
};

#endif
