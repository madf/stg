#ifndef __VALUE_2_OS_H__
#define __VALUE_2_OS_H__

template <typename T>
bool ValueToOS(const T & value, ObjectSyntax * objectSyntax);

template <>
inline
bool ValueToOS<int>(const int & value, ObjectSyntax * objectSyntax)
{
objectSyntax->present = ObjectSyntax_PR_simple;
SimpleSyntax_t * simpleSyntax = &objectSyntax->choice.simple;
simpleSyntax->present = SimpleSyntax_PR_number;
asn_long2INTEGER(&simpleSyntax->choice.number, value);
return true;
}

#endif
