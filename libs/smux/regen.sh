#!/bin/sh

set -euo pipefail

ASN1C=${ASN1C:-"/usr/bin/asn1c"}

ASN1C_FLAGS="${ASN1C_FLAGS:-""} -fcompound-names -fwide-types"

${ASN1C} ${ASN1C_FLAGS} RFC1213-MIB.asn1 RFC1155-SMI.asn1 RFC1157-SNMP.asn1 SMUX.asn1
mv *.h include/stg/
rm -f Makefile.am.sample
rm -f converter-sample.c
rm -f Makefile.am.asn1convert
rm -f Makefile.am.libasncodec
rm -f converter-example.mk
rm -f converter-example.c
