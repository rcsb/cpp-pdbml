//$$FILE$$
//$$VERSION$$
//$$DATE$$
//$$LICENSE$$


/**
** \file XsdWriter.h
**
** XSD Writer class.
*/


#ifndef XSDWRITER_H
#define XSDWRITER_H


#include <istream>
#include <string>

#include "rcsb_types.h"
#include "XmlWriter.h"


class XsdWriter : public XmlWriter
{
  public:
    XsdWriter(std::ostream& io);
 
    ~XsdWriter();

    void WriteSchemaOpeningTag();
    void WriteSchemaClosingTag();

    void WriteComplexTypeOpeningTag(const bool closeBracket = false);
    void WriteComplexTypeClosingTag();

    void WriteSequenceOpeningTag();
    void WriteSequenceClosingTag();

    void WriteAllOpeningTag();
    void WriteAllClosingTag();

    void WriteAnnotationOpeningTag();
    void WriteAnnotationClosingTag();

    void WriteDocumentationOpeningTag();
    void WriteDocumentationClosingTag();

    void WriteElementOpeningTag();
    void WriteElementClosingTag();

    void WriteAttributeOpeningTag();
    void WriteAttributeClosingTag();

    void WriteSimpleTypeOpeningTag();
    void WriteSimpleTypeClosingTag();

    void WriteRestrictionOpeningTag();
    void WriteRestrictionClosingTag();

    void WriteEnumerationOpeningTag();
    void WriteEnumerationClosingTag();

    void WriteUnionOpeningTag();
    void WriteUnionClosingTag();

    void WriteSimpleContentOpeningTag();
    void WriteSimpleContentClosingTag();

    void WriteExtensionOpeningTag();
    void WriteExtensionClosingTag();

    void WriteMinInclusiveOpeningTag();
    void WriteMinInclusiveClosingTag();

    void WriteMaxInclusiveOpeningTag();
    void WriteMaxInclusiveClosingTag();

    void WriteMinExclusiveOpeningTag();
    void WriteMinExclusiveClosingTag();

    void WriteMaxExclusiveOpeningTag();
    void WriteMaxExclusiveClosingTag();

    void WriteUniqueOpeningTag();
    void WriteUniqueClosingTag();

    void WriteFieldOpeningTag();
    void WriteFieldClosingTag();

    void WriteKeyOpeningTag();
    void WriteKeyClosingTag();

    void WriteKeyrefOpeningTag();
    void WriteKeyrefClosingTag();

    void WriteSelectorOpeningTag();

    void WriteNameAttribute(const std::string& value);
    void WriteValueAttribute(const std::string& value,
      const bool format = false);
    void WriteBaseAttribute(const eTypeCode iType);
    void WriteFixedAttribute(const std::string& value);
    void WriteUseAttribute(const std::string& value);
    void WriteMinOccursAttribute(const std::string& value);
    void WriteMaxOccursAttribute(const std::string& value);
    void WriteNillableAttribute(const std::string& value);
    void WriteXpathAttribute(const std::string& value,
      const bool prependAtSymbol = false);
    void WriteTypeAttribute(const std::string& value);
    void WriteReferAttribute(const std::string& value);
    void WriteTargetNamespaceAttribute(const std::string& value);
    void WriteElementFormDefaultAttribute(const std::string& value);
    void WriteAttributeFormDefaultAttribute(const std::string& value);

    void _WriteDataTypeAsAttribute(const eTypeCode iType);

  private:
    static std::string SCHEMA_TAG;
    static std::string COMPLEX_TYPE_TAG;
    static std::string SEQUENCE_TAG;
    static std::string ALL_TAG;
    static std::string ANNOTATION_TAG;
    static std::string DOCUMENTATION_TAG;
    static std::string SIMPLE_TYPE_TAG;
    static std::string RESTRICTION_TAG;
    static std::string ENUMERATION_TAG;
    static std::string UNION_TAG;
    static std::string SIMPLE_CONTENT_TAG;
    static std::string EXTENSION_TAG;
    static std::string ELEMENT_TAG;
    static std::string ATTRIBUTE_TAG;
    static std::string MIN_INCLUSIVE_TAG;
    static std::string MAX_INCLUSIVE_TAG;
    static std::string MIN_EXCLUSIVE_TAG;
    static std::string MAX_EXCLUSIVE_TAG;
    static std::string UNIQUE_TAG;
    static std::string FIELD_TAG;
    static std::string KEY_TAG;
    static std::string KEYREF_TAG;
    static std::string SELECTOR_TAG;

    static std::string NAME_ATTRIBUTE;
    static std::string VALUE_ATTRIBUTE;
    static std::string BASE_ATTRIBUTE;
    static std::string FIXED_ATTRIBUTE;
    static std::string USE_ATTRIBUTE;
    static std::string MIN_OCCURS_ATTRIBUTE;
    static std::string MAX_OCCURS_ATTRIBUTE;
    static std::string NILLABLE_ATTRIBUTE;
    static std::string XPATH_ATTRIBUTE;
    static std::string TYPE_ATTRIBUTE;
    static std::string REFER_ATTRIBUTE;
    static std::string TARGET_NAMESPACE_ATTRIBUTE;
    static std::string ELEMENT_FORM_DEFAULT_ATTRIBUTE;
    static std::string ATTRIBUTE_FORM_DEFAULT_ATTRIBUTE;
};

#endif
