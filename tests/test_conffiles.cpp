#define BOOST_TEST_MODULE STGConfFiles

#include "stg/conffiles.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wparentheses"
#include <boost/test/unit_test.hpp>
#pragma GCC diagnostic pop

#include <string>
#include <fstream>

#include <unistd.h> // unlink

BOOST_AUTO_TEST_SUITE(ConfFiles)

BOOST_AUTO_TEST_CASE(ReadWrite)
{
    {
        CONFIGFILE cf("/tmp/test.cf", true);

        BOOST_CHECK_EQUAL(cf.Error(), 0);

        cf.WriteString("a", "a-string");
        cf.WriteInt("b", 0);
        cf.WriteDouble("e", 2.718281828);

        BOOST_CHECK_EQUAL(cf.Flush(), 0);
    }

    {
        CONFIGFILE cf("/tmp/test.cf");

        BOOST_CHECK_EQUAL(cf.Error(), 0);

        std::string svalue;
        BOOST_CHECK_EQUAL(cf.ReadString("a", &svalue, "a-default"), 0);
        int ivalue;
        BOOST_CHECK_EQUAL(cf.ReadInt("b", &ivalue, -1), 0);
        double dvalue = 0;
        BOOST_CHECK_EQUAL(cf.ReadDouble("e", &dvalue, 0), 0);

        BOOST_CHECK_EQUAL(svalue, "a-string");
        BOOST_CHECK_EQUAL(ivalue, 0);
        BOOST_CHECK(dvalue != 0);
    }

    BOOST_CHECK_EQUAL(unlink("/tmp/test.cf"), 0);
}

BOOST_AUTO_TEST_CASE(EmptyLinesAndComments)
{
    {
        std::ofstream f("/tmp/test.cf");

        BOOST_CHECK(static_cast<bool>(f));

        f << "\n"
          << "a=a-string# a string\n"
          << "              \n"
          << "b=0\n"
          << "#abc\n"
          << "e=2.718281828\n";
    }

    {
        CONFIGFILE cf("/tmp/test.cf");

        BOOST_CHECK_EQUAL(cf.Error(), 0);

        std::string svalue;
        BOOST_CHECK_EQUAL(cf.ReadString("a", &svalue, "a-default"), 0);
        int ivalue;
        BOOST_CHECK_EQUAL(cf.ReadInt("b", &ivalue, -1), 0);
        double dvalue = 0;
        BOOST_CHECK_EQUAL(cf.ReadDouble("e", &dvalue, 0), 0);

        BOOST_CHECK_EQUAL(svalue, "a-string");
        BOOST_CHECK_EQUAL(ivalue, 0);
        BOOST_CHECK(dvalue != 0);
    }

    BOOST_CHECK_EQUAL(unlink("/tmp/test.cf"), 0);
}

BOOST_AUTO_TEST_SUITE_END()
