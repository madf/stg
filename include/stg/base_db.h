#ifndef __BASE_DB_H__
#define __BASE_DB_H__

#include <string>
#include <vector>
#include <map>

class BASE_DB {
public:

    typedef std::map<std::string, std::string> TUPLE;
    typedef std::vector<TUPLE> TUPLES;
    typedef std::vector<std::string> COLUMNS;

    BASE_DB() {};
    BASE_DB(std::string & dbHost,
	    std::string & dbDatabase,
	    std::string & dbUser,
	    std::string & dbPassword)
	: host(dbHost),
	  database(dbDatabase),
	  user(dbUser),
	  password(dbPassword)
    {};
    virtual ~BASE_DB() {};

    void SetHost(const std::string & h) { host = h; };
    void SetDatabase(const std::string & db) { database = db; };
    void SetUser(const std::string & u) { user = u; };
    void SetPassword(const std::string & p) { password = p; };

    const std::string & GetHost() const { return host; };
    const std::string & GetDatabase() const { return database; };
    const std::string & GetUser() const { return user; };
    const std::string & GetPassword() const { return password; };

    const std::string & GetErrorMsg() const { return errorMsg; };

    virtual bool Connect() { return true; };
    virtual bool Disconnect() { return true; };
    virtual bool Query(const std::string & q) { return true; };
    virtual bool Start() { return true; };
    virtual bool Commit() { return true; };
    virtual bool Rollback() { return true; };

    int GetTuples() const { return tuples; };
    int GetColumns() const { return columns; };
    int GetAffectedRows() const { return affected; };

    virtual TUPLES GetResult() const { return TUPLES(); };
    virtual TUPLE GetTuple(int n = 0) const { return TUPLE(); };
    const COLUMNS & GetColumnsNames() const { return cols; };

protected:
    std::string host;
    std::string database;
    std::string user;
    std::string password;

    std::string errorMsg;

    COLUMNS cols;

    int columns;
    int tuples;
    int affected;
};

extern "C" BASE_DB * CreateDriver();
extern "C" void DestroyDriver(BASE_DB *);

typedef BASE_DB * (* CreateDriverFn)();
typedef void (* DestroyDriverFn)(BASE_DB *);

#endif
