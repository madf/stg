/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RFC1155-SMI"
 * 	found in "RFC1155-SMI.asn1"
 * 	`asn1c -S /home/faust/software/asn1c/skeletons/ -fcompound-names -fwide-types`
 */

#include "NetworkAddress.h"

static asn_oer_constraints_t asn_OER_type_NetworkAddress_constr_1 CC_NOTUSED = {
	{ 0, 0 },
	-1};
asn_per_constraints_t asn_PER_type_NetworkAddress_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 0,  0,  0,  0 }	/* (0..0) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
asn_TYPE_member_t asn_MBR_NetworkAddress_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct NetworkAddress, choice.internet),
		(ASN_TAG_CLASS_APPLICATION | (0 << 2)),
		0,
		&asn_DEF_IpAddress,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"internet"
		},
};
static const asn_TYPE_tag2member_t asn_MAP_NetworkAddress_tag2el_1[] = {
    { (ASN_TAG_CLASS_APPLICATION | (0 << 2)), 0, 0, 0 } /* internet */
};
asn_CHOICE_specifics_t asn_SPC_NetworkAddress_specs_1 = {
	sizeof(struct NetworkAddress),
	offsetof(struct NetworkAddress, _asn_ctx),
	offsetof(struct NetworkAddress, present),
	sizeof(((struct NetworkAddress *)0)->present),
	asn_MAP_NetworkAddress_tag2el_1,
	1,	/* Count of tags in the map */
	0, 0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_NetworkAddress = {
	"NetworkAddress",
	"NetworkAddress",
	&asn_OP_CHOICE,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	{ &asn_OER_type_NetworkAddress_constr_1, &asn_PER_type_NetworkAddress_constr_1, CHOICE_constraint },
	asn_MBR_NetworkAddress_1,
	1,	/* Elements count */
	&asn_SPC_NetworkAddress_specs_1	/* Additional specs */
};

