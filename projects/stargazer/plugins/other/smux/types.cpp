#include "types.h"

#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <sstream>

namespace
{

bool ParseArcs(const char * str, ptrdiff_t length, uint32_t * a, size_t * pos);
bool StringToArcs(const char * str, size_t length, std::vector<uint32_t> & arcs);
bool AppendToArcs(const char * str, size_t length, std::vector<uint32_t> & arcs);

bool ParseArcs(const char * str, ptrdiff_t length, uint32_t * a, size_t * pos)
{
if (length == 0)
    return false;
const char * left = str;
if (*left == '.')
    ++left;
size_t arcPos = 0;
while ((left - str) < length)
    {
    char * p = NULL;
    uint32_t arc = static_cast<uint32_t>(strtoul(left, &p, 10));
    if (p == left)
        return false;
    a[arcPos++] = arc;
    if (arcPos >= 1024)
        return false;
    left = p + 1;
    }
*pos = arcPos;
return true;
}

bool StringToArcs(const char * str, size_t length, std::vector<uint32_t> & arcs)
{
uint32_t a[1024];
size_t pos = 0;

if (!ParseArcs(str, length, a, &pos))
    return false;

arcs.assign(a, a + pos);
return true;
}

bool AppendToArcs(const char * str, size_t length, std::vector<uint32_t> & arcs)
{
uint32_t a[1024];
size_t pos = 0;

if (!ParseArcs(str, length, a, &pos))
    return false;

std::copy(&a[0], &a[pos], std::back_inserter(arcs));
return true;
}

}

OID::OID(const std::string & str)
    : arcs()
{
if (!StringToArcs(str.c_str(), str.length(), arcs))
    throw std::runtime_error("Invalid oid");
}

OID::OID(const char * str, size_t length)
    : arcs()
{
if (!StringToArcs(str, length, arcs))
    throw std::runtime_error("Invalid oid");
}

OID::OID(const std::vector<uint32_t> & a)
    : arcs(a)
{
}

OID::OID(const uint32_t * a, size_t length)
    : arcs()
{
std::vector<uint32_t> newArcs(a, a + length);
arcs.swap(newArcs);
}

OID::OID(OBJECT_IDENTIFIER_t * oid)
    : arcs()
{
uint32_t a[1024];
int count = OBJECT_IDENTIFIER_get_arcs(oid, a, 1024);

if (count > 1024)
    throw std::runtime_error("OID is too long");

std::vector<uint32_t> newArcs(a, a + count);
arcs.swap(newArcs);
}

OID::OID(const OID & rvalue)
    : arcs(rvalue.arcs)
{
}

OID::~OID()
{
}

bool OID::addSuffix(const char * suffix, size_t length)
{
if (!AppendToArcs(suffix, length, arcs))
    return false;
return true;
}

bool OID::addSuffix(const std::string & suffix)
{
if (!AppendToArcs(suffix.c_str(), suffix.length(), arcs))
    return false;
return true;
}

bool OID::addSuffix(const uint32_t * suffix, size_t length)
{
std::copy(suffix, suffix + length, std::back_inserter(arcs));
return true;
}

bool OID::addSuffix(const std::vector<uint32_t> & suffix)
{
std::copy(suffix.begin(), suffix.end(), std::back_inserter(arcs));
return true;
}

bool OID::addSuffix(uint32_t a, uint32_t b)
{
arcs.push_back(a);
arcs.push_back(b);
return true;
}

OID OID::copyWithSuffix(const char * suffix, size_t length) const
{
OID oid(*this);
if (!oid.addSuffix(suffix, length))
    throw std::runtime_error("Invalid suffix");
return oid;
}

OID OID::copyWithSuffix(const std::string & suffix) const
{
OID oid(*this);
if (!oid.addSuffix(suffix))
    throw std::runtime_error("Invalid suffix");
return oid;
}

OID OID::copyWithSuffix(const uint32_t * suffix, size_t length) const
{
OID oid(*this);
if (!oid.addSuffix(suffix, length))
    throw std::runtime_error("Invalid suffix");
return oid;
}

OID OID::copyWithSuffix(const std::vector<uint32_t> & suffix) const
{
OID oid(*this);
if (!oid.addSuffix(suffix))
    throw std::runtime_error("Invalid suffix");
return oid;
}

OID OID::copyWithSuffix(uint32_t a, uint32_t b) const
{
OID oid(*this);
oid.addSuffix(a, b);
return oid;
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
OBJECT_IDENTIFIER_set_arcs(oid, &arcs.front(), static_cast<uint32_t>(arcs.size()));
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
size_t i = 0;
size_t min = std::min(arcs.size(), rvalue.arcs.size());
while (i < min &&
       arcs[i] == rvalue.arcs[i])
    ++i;
if (i == min)
    {
    if (rvalue.arcs.size() > arcs.size())
        return true;
    return false;
    }

if (arcs[i] < rvalue.arcs[i])
    return true;

return false;
}

bool OID::PrefixLess(const OID & rvalue) const
{
size_t i = 0;
size_t min = std::min(arcs.size(), rvalue.arcs.size());
while (i < min &&
       arcs[i] == rvalue.arcs[i])
    ++i;
if (i == min)
    return false;
if (arcs[i] < rvalue.arcs[i])
    return true;
return false;
}

std::ostream & operator<<(std::ostream & stream, const OID & oid)
{
for (size_t i = 0; i < oid.arcs.size(); ++i)
    stream << "." << oid.arcs[i];
return stream;
}
