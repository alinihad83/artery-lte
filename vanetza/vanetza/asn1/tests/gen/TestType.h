/*
 * Generated by asn1c-0.9.24 (http://lionet.info/asn1c)
 * From ASN.1 module "Test-Descriptions"
 * 	found in "../test.asn1"
 */

#ifndef	_TestType_H_
#define	_TestType_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum TestType {
	TestType_magicValue	= 42
} e_TestType;

/* TestType */
typedef long	 TestType_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TestType;
asn_struct_free_f TestType_free;
asn_struct_print_f TestType_print;
asn_constr_check_f TestType_constraint;
ber_type_decoder_f TestType_decode_ber;
der_type_encoder_f TestType_encode_der;
xer_type_decoder_f TestType_decode_xer;
xer_type_encoder_f TestType_encode_xer;
per_type_decoder_f TestType_decode_uper;
per_type_encoder_f TestType_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _TestType_H_ */
#include <asn_internal.h>
