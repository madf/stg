/*
 * Generated by asn1c-0.9.21 (http://lionet.info/asn1c)
 * From ASN.1 module "RFC1213-MIB"
 * 	found in "RFC1213-MIB.asn1"
 * 	`asn1c -fskeletons-copy`
 */

#ifndef	_EgpNeighEntry_H_
#define	_EgpNeighEntry_H_


#include <asn_application.h>

/* Including external dependencies */
#include <INTEGER.h>
#include "IpAddress.h"
#include "Counter.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* EgpNeighEntry */
typedef struct EgpNeighEntry {
	INTEGER_t	 egpNeighState;
	IpAddress_t	 egpNeighAddr;
	INTEGER_t	 egpNeighAs;
	Counter_t	 egpNeighInMsgs;
	Counter_t	 egpNeighInErrs;
	Counter_t	 egpNeighOutMsgs;
	Counter_t	 egpNeighOutErrs;
	Counter_t	 egpNeighInErrMsgs;
	Counter_t	 egpNeighOutErrMsgs;
	Counter_t	 egpNeighStateUps;
	Counter_t	 egpNeighStateDowns;
	INTEGER_t	 egpNeighIntervalHello;
	INTEGER_t	 egpNeighIntervalPoll;
	INTEGER_t	 egpNeighMode;
	INTEGER_t	 egpNeighEventTrigger;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} EgpNeighEntry_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_EgpNeighEntry;

#ifdef __cplusplus
}
#endif

#endif	/* _EgpNeighEntry_H_ */