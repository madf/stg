/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RFC1157-SNMP"
 * 	found in "RFC1157-SNMP.asn1"
 * 	`asn1c -S /home/faust/software/asn1c/skeletons/ -fcompound-names -fwide-types`
 */

#include "PDUs.h"

static asn_oer_constraints_t asn_OER_type_PDUs_constr_1 CC_NOTUSED = {
	{ 0, 0 },
	-1};
asn_per_constraints_t asn_PER_type_PDUs_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 3,  3,  0,  4 }	/* (0..4) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
asn_TYPE_member_t asn_MBR_PDUs_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct PDUs, choice.get_request),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		0,
		&asn_DEF_GetRequest_PDU,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"get-request"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PDUs, choice.get_next_request),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		0,
		&asn_DEF_GetNextRequest_PDU,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"get-next-request"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PDUs, choice.get_response),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		0,
		&asn_DEF_GetResponse_PDU,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"get-response"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PDUs, choice.set_request),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		0,
		&asn_DEF_SetRequest_PDU,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"set-request"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PDUs, choice.trap),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		0,
		&asn_DEF_Trap_PDU,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"trap"
		},
};
static const asn_TYPE_tag2member_t asn_MAP_PDUs_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* get-request */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* get-next-request */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* get-response */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* set-request */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 } /* trap */
};
asn_CHOICE_specifics_t asn_SPC_PDUs_specs_1 = {
	sizeof(struct PDUs),
	offsetof(struct PDUs, _asn_ctx),
	offsetof(struct PDUs, present),
	sizeof(((struct PDUs *)0)->present),
	asn_MAP_PDUs_tag2el_1,
	5,	/* Count of tags in the map */
	0, 0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_PDUs = {
	"PDUs",
	"PDUs",
	&asn_OP_CHOICE,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	{ &asn_OER_type_PDUs_constr_1, &asn_PER_type_PDUs_constr_1, CHOICE_constraint },
	asn_MBR_PDUs_1,
	5,	/* Elements count */
	&asn_SPC_PDUs_specs_1	/* Additional specs */
};

