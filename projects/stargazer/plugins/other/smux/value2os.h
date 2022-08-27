#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include "stg/ObjectSyntax.h"
#pragma GCC diagnostic pop

#include <string>

template <typename T>
void ValueToOS(const T & value, ObjectSyntax * objectSyntax);

template <>
inline
void ValueToOS<int>(const int & value, ObjectSyntax * objectSyntax)
{
objectSyntax->present = ObjectSyntax_PR_simple;
SimpleSyntax_t * simpleSyntax = &objectSyntax->choice.simple;
simpleSyntax->present = SimpleSyntax_PR_number;
asn_long2INTEGER(&simpleSyntax->choice.number, value);
}

template <>
inline
void ValueToOS<unsigned int>(const unsigned int & value, ObjectSyntax * objectSyntax)
{
objectSyntax->present = ObjectSyntax_PR_simple;
SimpleSyntax_t * simpleSyntax = &objectSyntax->choice.simple;
simpleSyntax->present = SimpleSyntax_PR_number;
asn_long2INTEGER(&simpleSyntax->choice.number, value);
}

template <>
inline
void ValueToOS<long>(const long & value, ObjectSyntax * objectSyntax)
{
objectSyntax->present = ObjectSyntax_PR_simple;
SimpleSyntax_t * simpleSyntax = &objectSyntax->choice.simple;
simpleSyntax->present = SimpleSyntax_PR_number;
asn_long2INTEGER(&simpleSyntax->choice.number, value);
}

template <>
inline
void ValueToOS<unsigned long>(const unsigned long & value, ObjectSyntax * objectSyntax)
{
objectSyntax->present = ObjectSyntax_PR_simple;
SimpleSyntax_t * simpleSyntax = &objectSyntax->choice.simple;
simpleSyntax->present = SimpleSyntax_PR_number;
asn_long2INTEGER(&simpleSyntax->choice.number, value);
}

template <>
inline
void ValueToOS<std::string>(const std::string & value, ObjectSyntax * objectSyntax)
{
objectSyntax->present = ObjectSyntax_PR_simple;
SimpleSyntax_t * simpleSyntax = &objectSyntax->choice.simple;
simpleSyntax->present = SimpleSyntax_PR_string;
OCTET_STRING_fromBuf(&simpleSyntax->choice.string, value.c_str(), static_cast<int>(value.length()));
}
