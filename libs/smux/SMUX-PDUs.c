/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SMUX"
 * 	found in "SMUX.asn1"
 * 	`asn1c -S /home/faust/software/asn1c/skeletons/ -fcompound-names -fwide-types`
 */

#include "SMUX-PDUs.h"

static asn_oer_constraints_t asn_OER_type_SMUX_PDUs_constr_1 CC_NOTUSED = {
	{ 0, 0 },
	-1};
static asn_per_constraints_t asn_PER_type_SMUX_PDUs_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 3,  3,  0,  5 }	/* (0..5) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_TYPE_member_t asn_MBR_SMUX_PDUs_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SMUX_PDUs, choice.open),
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_OpenPDU,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"open"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SMUX_PDUs, choice.close),
		(ASN_TAG_CLASS_APPLICATION | (1 << 2)),
		0,
		&asn_DEF_ClosePDU,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"close"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SMUX_PDUs, choice.registerRequest),
		(ASN_TAG_CLASS_APPLICATION | (2 << 2)),
		0,
		&asn_DEF_RReqPDU,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"registerRequest"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SMUX_PDUs, choice.registerResponse),
		(ASN_TAG_CLASS_APPLICATION | (3 << 2)),
		0,
		&asn_DEF_RRspPDU,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"registerResponse"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SMUX_PDUs, choice.pdus),
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_PDUs,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"pdus"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SMUX_PDUs, choice.commitOrRollback),
		(ASN_TAG_CLASS_APPLICATION | (4 << 2)),
		0,
		&asn_DEF_SOutPDU,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"commitOrRollback"
		},
};
static const unsigned asn_MAP_SMUX_PDUs_to_canonical_1[] = { 0, 1, 2, 3, 5, 4 };
static const unsigned asn_MAP_SMUX_PDUs_from_canonical_1[] = { 0, 1, 2, 3, 5, 4 };
static const asn_TYPE_tag2member_t asn_MAP_SMUX_PDUs_tag2el_1[] = {
    { (ASN_TAG_CLASS_APPLICATION | (0 << 2)), 0, 0, 0 }, /* simple */
    { (ASN_TAG_CLASS_APPLICATION | (1 << 2)), 1, 0, 0 }, /* close */
    { (ASN_TAG_CLASS_APPLICATION | (2 << 2)), 2, 0, 0 }, /* registerRequest */
    { (ASN_TAG_CLASS_APPLICATION | (3 << 2)), 3, 0, 0 }, /* registerResponse */
    { (ASN_TAG_CLASS_APPLICATION | (4 << 2)), 5, 0, 0 }, /* commitOrRollback */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 4, 0, 0 }, /* get-request */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 4, 0, 0 }, /* get-next-request */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 4, 0, 0 }, /* get-response */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 4, 0, 0 }, /* set-request */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 } /* trap */
};
static asn_CHOICE_specifics_t asn_SPC_SMUX_PDUs_specs_1 = {
	sizeof(struct SMUX_PDUs),
	offsetof(struct SMUX_PDUs, _asn_ctx),
	offsetof(struct SMUX_PDUs, present),
	sizeof(((struct SMUX_PDUs *)0)->present),
	asn_MAP_SMUX_PDUs_tag2el_1,
	10,	/* Count of tags in the map */
	asn_MAP_SMUX_PDUs_to_canonical_1,
	asn_MAP_SMUX_PDUs_from_canonical_1,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_SMUX_PDUs = {
	"SMUX-PDUs",
	"SMUX-PDUs",
	&asn_OP_CHOICE,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	{ &asn_OER_type_SMUX_PDUs_constr_1, &asn_PER_type_SMUX_PDUs_constr_1, CHOICE_constraint },
	asn_MBR_SMUX_PDUs_1,
	6,	/* Elements count */
	&asn_SPC_SMUX_PDUs_specs_1	/* Additional specs */
};

