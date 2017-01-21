#ifndef __TYPES_H__
#define __TYPES_H__

#include <string>
#include <vector>
#include <iostream>

#include "stg/OBJECT_IDENTIFIER.h"

class OID {
    public:
        explicit OID(const std::string & str);
        OID(const char * str, size_t length);
        explicit OID(const std::vector<unsigned> & arcs);
        OID(const unsigned * arcs, size_t length);
        explicit OID(OBJECT_IDENTIFIER_t * oid);
        OID(const OID & rvalue);
        ~OID();

        bool addSuffix(const char * suffix, size_t length);
        bool addSuffix(const std::string & suffix);
        bool addSuffix(const unsigned * suffix, size_t length);
        bool addSuffix(const std::vector<unsigned> & suffix);
        bool addSuffix(unsigned a, unsigned b);

        OID copyWithSuffix(const char * suffix, size_t length) const;
        OID copyWithSuffix(const std::string & suffix) const;
        OID copyWithSuffix(const unsigned * suffix, size_t length) const;
        OID copyWithSuffix(const std::vector<unsigned> & suffix) const;
        OID copyWithSuffix(unsigned a, unsigned b) const;

        std::string ToString() const;
        const std::vector<unsigned> & ToVector() const { return arcs; }
        void ToOID(OBJECT_IDENTIFIER_t * oid) const;

        OID & operator=(const OID & rvalue);
        bool operator==(const OID & rvalue) const;
        bool operator!=(const OID & rvalue) const { return !operator==(rvalue); }
        bool operator<(const OID & rvalue) const;
        bool operator>(const OID & rvalue) const
        { return !operator==(rvalue) && !operator<(rvalue); }

        bool PrefixLess(const OID & rvalue) const;

        friend std::ostream & operator<<(std::ostream & stream, const OID & oid);

    private:
        std::vector<unsigned> arcs;
};

inline
bool PrefixLess(const OID & a, const OID & b)
{
return a.PrefixLess(b);
}

#endif
