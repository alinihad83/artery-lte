macro(asn1c_sources VAR DIR)
    file(GLOB _sources ${DIR}/*.c)
    list(REMOVE_ITEM _sources ${DIR}/converter-sample.c)
    set(${VAR} ${_sources})
endmacro()

find_program(ASN1C NAMES asn1c DOC "ASN.1 compiler")
if(ASN1C)
    execute_process(COMMAND ${ASN1C} -version ERROR_VARIABLE _asn1c_version)
    string(REGEX MATCH "[0-9]\\.[0-9]\\.[0-9]+" ASN1C_VERSION ${_asn1c_version})
endif()
