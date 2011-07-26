#ifndef __TYPES_H__
#define __TYPES_H__

#include <string>
#include <vector>
#include <istream>

#include "asn1/OBJECT_IDENTIFIER.h"

class OID {
    public:
        OID(const std::string & str);
        OID(const char * str, size_t length);
        OID(const std::vector<unsigned> & arcs);
        OID(const unsigned * arcs, size_t length);
        OID(OBJECT_IDENTIFIER_t * oid);
        OID(const OID & rvalue);
        ~OID();

        std::string ToString() const;
        const std::vector<unsigned> & ToVector() const { return arcs; }
        void ToOID(OBJECT_IDENTIFIER_t * oid) const;

        OID & operator=(const OID & rvalue);
        bool operator==(const OID & rvalue) const;
        bool operator<(const OID & rvalue) const;

        std::ostream & operator<<(std::ostream & stream) const;

    private:
        std::vector<unsigned> arcs;
};

#endif
