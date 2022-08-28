/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RFC1157-SNMP"
 * 	found in "RFC1157-SNMP.asn1"
 * 	`asn1c -S /home/faust/software/asn1c/skeletons/ -fcompound-names -fwide-types`
 */

#ifndef	_GetResponse_PDU_H_
#define	_GetResponse_PDU_H_


#include <asn_application.h>

/* Including external dependencies */
#include "PDU.h"

#ifdef __cplusplus
extern "C" {
#endif

/* GetResponse-PDU */
typedef PDU_t	 GetResponse_PDU_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GetResponse_PDU;
asn_struct_free_f GetResponse_PDU_free;
asn_struct_print_f GetResponse_PDU_print;
asn_constr_check_f GetResponse_PDU_constraint;
ber_type_decoder_f GetResponse_PDU_decode_ber;
der_type_encoder_f GetResponse_PDU_encode_der;
xer_type_decoder_f GetResponse_PDU_decode_xer;
xer_type_encoder_f GetResponse_PDU_encode_xer;
oer_type_decoder_f GetResponse_PDU_decode_oer;
oer_type_encoder_f GetResponse_PDU_encode_oer;
per_type_decoder_f GetResponse_PDU_decode_uper;
per_type_encoder_f GetResponse_PDU_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _GetResponse_PDU_H_ */
#include <asn_internal.h>
