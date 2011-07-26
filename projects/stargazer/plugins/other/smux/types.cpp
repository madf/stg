#include <stdexcept>
#include <algorithm>
#include <sstream>

#include "types.h"

bool StringToArcs(const char * str, size_t length, std::vector<unsigned> & arcs)
{
unsigned a[1024];
if (length == 0)
    return false;
const char * left = str;
if (*left == '.')
    ++left;
size_t arcPos = 0;
while ((left - str) < length)
    {
    char * pos = NULL;
    unsigned arc = strtoul(left, &pos, 10);
    if (pos == left)
        return false;
    a[arcPos++] = arc;
    if (arcPos >= 1024)
        return false;
    left = pos + 1;
    }

std::vector<unsigned> newArcs(a, a + arcPos);
arcs.swap(newArcs);
return true;
}

OID::OID(const std::string & str)
{
if (!StringToArcs(str.c_str(), str.length(), arcs))
    throw std::runtime_error("Invalid oid");
}

OID::OID(const char * str, size_t length)
{
if (!StringToArcs(str, length, arcs))
    throw std::runtime_error("Invalid oid");
}

OID::OID(const std::vector<unsigned> & a)
    : arcs(a)
{
}

OID::OID(const unsigned * a, size_t length)
{
std::vector<unsigned> newArcs(a, a + length);
arcs.swap(newArcs);
}

OID::OID(OBJECT_IDENTIFIER_t * oid)
{
unsigned a[1024];
int count = OBJECT_IDENTIFIER_get_arcs(oid, a, sizeof(a[0]), 1024);

if (count > 1024)
    throw std::runtime_error("OID is too long");

std::vector<unsigned> newArcs(a, a + count);
arcs.swap(newArcs);
}

OID::OID(const OID & rvalue)
    : arcs(rvalue.arcs)
{
}

OID::~OID()
{
}

std::string OID::ToString() const
{
std::stringstream stream;
for (size_t i = 0; i < arcs.size(); ++i)
    stream << "." << arcs[i];
return stream.str();
}

void OID::ToOID(OBJECT_IDENTIFIER_t * oid) const
{
OBJECT_IDENTIFIER_set_arcs(oid, &arcs.front(), sizeof(unsigned), arcs.size());
}

OID & OID::operator=(const OID & rvalue)
{
arcs = rvalue.arcs;
return *this;
}

bool OID::operator==(const OID & rvalue) const
{
if (arcs.size() != rvalue.arcs.size())
    return false;
for (size_t i = 0; i < arcs.size(); ++i)
    if (arcs[i] != rvalue.arcs[i])
        return false;
return true;
}

bool OID::operator<(const OID & rvalue) const
{
for (size_t i = 0; i < std::min(arcs.size(), rvalue.arcs.size()); ++i)
    if (arcs[i] > rvalue.arcs[i])
        return false;
if (rvalue.arcs.size() < arcs.size())
    return false;
return true;
}

std::ostream & operator<<(std::ostream & stream, const OID & oid)
{
for (size_t i = 0; i < oid.arcs.size(); ++i)
    stream << "." << oid.arcs[i];
return stream;
}
