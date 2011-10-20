/*
 * Generated by asn1c-0.9.21 (http://lionet.info/asn1c)
 * From ASN.1 module "SMUX"
 * 	found in "SMUX.asn1"
 * 	`asn1c -fskeletons-copy`
 */

#ifndef	_SMUX_PDUs_H_
#define	_SMUX_PDUs_H_


#include <asn_application.h>

/* Including external dependencies */
#include "OpenPDU.h"
#include "ClosePDU.h"
#include "RReqPDU.h"
#include "RRspPDU.h"
#include "PDUs.h"
#include "SOutPDU.h"
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SMUX_PDUs_PR {
	SMUX_PDUs_PR_NOTHING,	/* No components present */
	SMUX_PDUs_PR_open,
	SMUX_PDUs_PR_close,
	SMUX_PDUs_PR_registerRequest,
	SMUX_PDUs_PR_registerResponse,
	SMUX_PDUs_PR_pdus,
	SMUX_PDUs_PR_commitOrRollback
} SMUX_PDUs_PR;

/* SMUX-PDUs */
typedef struct SMUX_PDUs {
	SMUX_PDUs_PR present;
	union SMUX_PDUs_u {
		OpenPDU_t	 open;
		ClosePDU_t	 close;
		RReqPDU_t	 registerRequest;
		RRspPDU_t	 registerResponse;
		PDUs_t	 pdus;
		SOutPDU_t	 commitOrRollback;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SMUX_PDUs_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SMUX_PDUs;

#ifdef __cplusplus
}
#endif

#endif	/* _SMUX_PDUs_H_ */