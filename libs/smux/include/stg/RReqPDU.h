/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SMUX"
 * 	found in "SMUX.asn1"
 * 	`asn1c -S /home/faust/software/asn1c/skeletons/ -fcompound-names -fwide-types`
 */

#ifndef	_RReqPDU_H_
#define	_RReqPDU_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ObjectName.h"
#include <NativeInteger.h>
#include <INTEGER.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum RReqPDU__operation {
	RReqPDU__operation_delete	= 0,
	RReqPDU__operation_readOnly	= 1,
	RReqPDU__operation_readWrite	= 2
} e_RReqPDU__operation;

/* RReqPDU */
typedef struct RReqPDU {
	ObjectName_t	 subtree;
	long	 priority;
	INTEGER_t	 operation;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RReqPDU_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RReqPDU;
extern asn_SEQUENCE_specifics_t asn_SPC_RReqPDU_specs_1;
extern asn_TYPE_member_t asn_MBR_RReqPDU_1[3];

#ifdef __cplusplus
}
#endif

#endif	/* _RReqPDU_H_ */
#include <asn_internal.h>
