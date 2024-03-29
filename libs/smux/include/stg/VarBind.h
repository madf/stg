/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RFC1157-SNMP"
 * 	found in "RFC1157-SNMP.asn1"
 * 	`asn1c -S /home/faust/software/asn1c/skeletons/ -fcompound-names -fwide-types`
 */

#ifndef	_VarBind_H_
#define	_VarBind_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ObjectName.h"
#include "ObjectSyntax.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* VarBind */
typedef struct VarBind {
	ObjectName_t	 name;
	ObjectSyntax_t	 value;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} VarBind_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_VarBind;
extern asn_SEQUENCE_specifics_t asn_SPC_VarBind_specs_1;
extern asn_TYPE_member_t asn_MBR_VarBind_1[2];

#ifdef __cplusplus
}
#endif

#endif	/* _VarBind_H_ */
#include <asn_internal.h>
