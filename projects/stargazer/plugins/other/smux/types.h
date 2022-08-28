#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include "stg/OBJECT_IDENTIFIER.h"
#pragma GCC diagnostic pop

#include <string>
#include <vector>
#include <iostream>

class OID
{
    public:
        explicit OID(const std::string & str);
        OID(const char * str, size_t length);
        explicit OID(const std::vector<uint32_t> & arcs);
        OID(const uint32_t * arcs, size_t length);
        explicit OID(OBJECT_IDENTIFIER_t * oid);
        OID(const OID & rvalue);
        ~OID();

        bool addSuffix(const char * suffix, size_t length);
        bool addSuffix(const std::string & suffix);
        bool addSuffix(const uint32_t * suffix, size_t length);
        bool addSuffix(const std::vector<uint32_t> & suffix);
        bool addSuffix(uint32_t a, uint32_t b);

        OID copyWithSuffix(const char * suffix, size_t length) const;
        OID copyWithSuffix(const std::string & suffix) const;
        OID copyWithSuffix(const uint32_t * suffix, size_t length) const;
        OID copyWithSuffix(const std::vector<uint32_t> & suffix) const;
        OID copyWithSuffix(uint32_t a, uint32_t b) const;

        std::string ToString() const;
        const std::vector<uint32_t> & ToVector() const { return arcs; }
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
        std::vector<uint32_t> arcs;
};

inline
bool PrefixLess(const OID & a, const OID & b)
{
return a.PrefixLess(b);
}
