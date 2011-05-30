###############################################################################
# $Id: Makefile,v 1.11 2008/12/04 17:21:14 faust Exp $
###############################################################################

include ../../../../../Makefile.conf

LIBS += $(LIB_THREAD)

PROG = mod_snmp_agent.so

SRCS =  snmp.cpp \
	asn1/DisplayString.c	\
	asn1/PhysAddress.c	\
	asn1/IfEntry.c	\
	asn1/AtEntry.c	\
	asn1/IpAddrEntry.c	\
	asn1/IpRouteEntry.c	\
	asn1/IpNetToMediaEntry.c	\
	asn1/TcpConnEntry.c	\
	asn1/UdpEntry.c	\
	asn1/EgpNeighEntry.c	\
	asn1/ObjectName.c	\
	asn1/ObjectSyntax.c	\
	asn1/SimpleSyntax.c	\
	asn1/ApplicationSyntax.c	\
	asn1/NetworkAddress.c	\
	asn1/IpAddress.c	\
	asn1/Counter.c	\
	asn1/Gauge.c	\
	asn1/TimeTicks.c	\
	asn1/Opaque.c	\
	asn1/Message.c	\
	asn1/PDUs.c	\
	asn1/GetRequest-PDU.c	\
	asn1/GetNextRequest-PDU.c	\
	asn1/GetResponse-PDU.c	\
	asn1/SetRequest-PDU.c	\
	asn1/PDU.c	\
	asn1/Trap-PDU.c	\
	asn1/VarBind.c	\
	asn1/VarBindList.c	\
	asn1/SMUX-PDUs.c	\
	asn1/OpenPDU.c	\
	asn1/SimpleOpen.c	\
	asn1/ClosePDU.c	\
	asn1/RReqPDU.c	\
	asn1/RRspPDU.c	\
	asn1/SOutPDU.c \
	asn1/ANY.c \
	asn1/BOOLEAN.c \
	asn1/INTEGER.c \
	asn1/NULL.c \
	asn1/NativeEnumerated.c \
	asn1/NativeInteger.c \
	asn1/OBJECT_IDENTIFIER.c \
	asn1/asn_SEQUENCE_OF.c \
	asn1/asn_SET_OF.c \
	asn1/constr_CHOICE.c \
	asn1/constr_SEQUENCE.c \
	asn1/constr_SEQUENCE_OF.c \
	asn1/constr_SET_OF.c \
	asn1/OCTET_STRING.c \
	asn1/BIT_STRING.c \
	asn1/asn_codecs_prim.c \
	asn1/ber_tlv_length.c \
	asn1/ber_tlv_tag.c \
	asn1/ber_decoder.c \
	asn1/der_encoder.c \
	asn1/constr_TYPE.c \
	asn1/constraints.c \
	asn1/xer_support.c \
	asn1/xer_decoder.c \
	asn1/xer_encoder.c \
	asn1/per_support.c \
	asn1/per_decoder.c \
	asn1/per_encoder.c

STGLIBS = -lstgcommon

CXXFLAGS += -Iasn1

include ../../Makefile.in

.PHONY: asn1

asn1:
	$(MAKE) -C asn1 asn1
