ASN1_SOURCES = RFC1213-MIB.asn1 \
	       RFC1155-SMI.asn1 \
	       RFC1157-SNMP.asn1 \
	       SMUX.asn1

asn1: $(ASN1_SOURCES)
	asn1c -fskeletons-copy -fcompound-names $^
	rm Makefile.am.sample
