/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RFC1213-MIB"
 * 	found in "RFC1213-MIB.asn1"
 * 	`asn1c -S /home/faust/software/asn1c/skeletons/ -fcompound-names -fwide-types`
 */

#ifndef	_IpRouteEntry_H_
#define	_IpRouteEntry_H_


#include <asn_application.h>

/* Including external dependencies */
#include "IpAddress.h"
#include <INTEGER.h>
#include <OBJECT_IDENTIFIER.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* IpRouteEntry */
typedef struct IpRouteEntry {
	IpAddress_t	 ipRouteDest;
	INTEGER_t	 ipRouteIfIndex;
	INTEGER_t	 ipRouteMetric1;
	INTEGER_t	 ipRouteMetric2;
	INTEGER_t	 ipRouteMetric3;
	INTEGER_t	 ipRouteMetric4;
	IpAddress_t	 ipRouteNextHop;
	INTEGER_t	 ipRouteType;
	INTEGER_t	 ipRouteProto;
	INTEGER_t	 ipRouteAge;
	IpAddress_t	 ipRouteMask;
	INTEGER_t	 ipRouteMetric5;
	OBJECT_IDENTIFIER_t	 ipRouteInfo;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} IpRouteEntry_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_IpRouteEntry;

#ifdef __cplusplus
}
#endif

#endif	/* _IpRouteEntry_H_ */
#include <asn_internal.h>
