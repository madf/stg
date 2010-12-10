#include <unistd.h> // unlink

#include <string>
#include <fstream>

#include <tut/tut.hpp>

#include "conffiles.h"

namespace tut
{
    struct conffile_data {
    };

    typedef test_group<conffile_data> tg;
    tg conffile_test_group("CONIGFILE tests group");

    typedef tg::object testobject;

    template<>
    template<>
    void testobject::test<1>()
    {
        set_test_name("Check read/write");

        {
            CONFIGFILE cf("/tmp/test.cf", true);

            ensure_equals("Correct construction", cf.Error(), 0);

            ensure_equals("Correct writing 'a' string", cf.WriteString("a", "a-string"), 0);
            ensure_equals("Correct writing 'b' integer (0)", cf.WriteInt("b", 0), 0);
            ensure_equals("Correct writing 'e' double (2.718281828)", cf.WriteDouble("e", 2.718281828), 0);
        }

        {
            CONFIGFILE cf("/tmp/test.cf");

            ensure_equals("Correct construction (part 2)", cf.Error(), 0);
            
            std::string svalue;
            ensure_equals("Correct reading 'a' param as string", cf.ReadString("a", &svalue, "a-default"), 0);
            int ivalue;
            ensure_equals("Correct reading 'b' param as integer", cf.ReadInt("b", &ivalue, -1), 0);
            double dvalue = 0;
            ensure_equals("Correct reading 'e' param as double", cf.ReadDouble("e", &dvalue, 0), 0);

            ensure_equals("Correct 'a' value", svalue, "a-string");
            ensure_equals("Correct 'b' value", ivalue, 0);
            ensure("Correct 'e' value", dvalue != 0);
        }

        ensure_equals("Correct temporary file unlinking", unlink("/tmp/test.cf"), 0);
    }

    template<>
    template<>
    void testobject::test<2>()
    {
        set_test_name("Check empty lines and comments");

        {
            ofstream f("/tmp/test.cf");

            ensure("Correct construction (part 3)", f);

            f << "\n"
              << "a=a-string# a string\n"
              << "              \n"
              << "b=0\n"
              << "#abc\n"
              << "e=2.718281828\n";
        }

        {
            CONFIGFILE cf("/tmp/test.cf");

            ensure_equals("Correct construction (part 4)", cf.Error(), 0);
            
            std::string svalue;
            ensure_equals("Correct reading 'a' param as string", cf.ReadString("a", &svalue, "a-default"), 0);
            int ivalue;
            ensure_equals("Correct reading 'b' param as integer", cf.ReadInt("b", &ivalue, -1), 0);
            double dvalue = 0;
            ensure_equals("Correct reading 'e' param as double", cf.ReadDouble("e", &dvalue, 0), 0);

            ensure_equals("Correct 'a' value", svalue, "a-string");
            ensure_equals("Correct 'b' value", ivalue, 0);
            ensure("Correct 'e' value", dvalue != 0);
        }

        ensure_equals("Correct temporary file unlinking", unlink("/tmp/test.cf"), 0);
    }
}
