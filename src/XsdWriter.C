//$$FILE$$
//$$VERSION$$
//$$DATE$$
//$$LICENSE$$


#include <string>
#include <istream>

#include "rcsb_types.h"
#include "XsdWriter.h"


using std::string;
using std::endl;
using std::ostream;


string XsdWriter::SCHEMA_TAG("schema");
string XsdWriter::COMPLEX_TYPE_TAG("complexType");
string XsdWriter::SEQUENCE_TAG("sequence");
string XsdWriter::ALL_TAG("all");
string XsdWriter::ANNOTATION_TAG("annotation");
string XsdWriter::DOCUMENTATION_TAG("documentation");
string XsdWriter::SIMPLE_TYPE_TAG("simpleType");
string XsdWriter::RESTRICTION_TAG("restriction");
string XsdWriter::ENUMERATION_TAG("enumeration");
string XsdWriter::UNION_TAG("union");
string XsdWriter::SIMPLE_CONTENT_TAG("simpleContent");
string XsdWriter::EXTENSION_TAG("extension");
string XsdWriter::ELEMENT_TAG("element");
string XsdWriter::ATTRIBUTE_TAG("attribute");
string XsdWriter::MIN_INCLUSIVE_TAG("minInclusive");
string XsdWriter::MAX_INCLUSIVE_TAG("maxInclusive");
string XsdWriter::MIN_EXCLUSIVE_TAG("minExclusive");
string XsdWriter::MAX_EXCLUSIVE_TAG("maxExclusive");
string XsdWriter::UNIQUE_TAG("unique");
string XsdWriter::FIELD_TAG("field");
string XsdWriter::KEY_TAG("key");
string XsdWriter::KEYREF_TAG("keyref");
string XsdWriter::SELECTOR_TAG("selector");

string XsdWriter::NAME_ATTRIBUTE("name");
string XsdWriter::VALUE_ATTRIBUTE("value");
string XsdWriter::BASE_ATTRIBUTE("base");
string XsdWriter::FIXED_ATTRIBUTE("fixed");
string XsdWriter::USE_ATTRIBUTE("use");
string XsdWriter::MIN_OCCURS_ATTRIBUTE("minOccurs");
string XsdWriter::MAX_OCCURS_ATTRIBUTE("maxOccurs");
string XsdWriter::NILLABLE_ATTRIBUTE("nillable");
string XsdWriter::XPATH_ATTRIBUTE("xpath");
string XsdWriter::TYPE_ATTRIBUTE("type");
string XsdWriter::REFER_ATTRIBUTE("refer");
string XsdWriter::TARGET_NAMESPACE_ATTRIBUTE("targetNamespace");
string XsdWriter::ELEMENT_FORM_DEFAULT_ATTRIBUTE("elementFormDefault");
string XsdWriter::ATTRIBUTE_FORM_DEFAULT_ATTRIBUTE("attributeFormDefault");


XsdWriter::XsdWriter(ostream& io) : XmlWriter(io, "xsd")
{

}


XsdWriter::~XsdWriter()
{

}


void XsdWriter::WriteSchemaOpeningTag()
{
    WriteOpeningTag(SCHEMA_TAG);
}


void XsdWriter::WriteSchemaClosingTag()
{
    WriteClosingTag(SCHEMA_TAG);
}


void XsdWriter::WriteComplexTypeOpeningTag(const bool closeBracket)
{
    WriteOpeningTag(COMPLEX_TYPE_TAG, closeBracket);
}


void XsdWriter::WriteComplexTypeClosingTag()
{
    WriteClosingTag(COMPLEX_TYPE_TAG);
}


void XsdWriter::WriteRestrictionOpeningTag()
{
    WriteOpeningTag(RESTRICTION_TAG);
}


void XsdWriter::WriteRestrictionClosingTag()
{
    WriteClosingTag(RESTRICTION_TAG);
}


void XsdWriter::WriteUniqueOpeningTag()
{
    WriteOpeningTag(UNIQUE_TAG);
}


void XsdWriter::WriteUniqueClosingTag()
{
    WriteClosingTag(UNIQUE_TAG);
}


void XsdWriter::WriteEnumerationOpeningTag()
{
    WriteOpeningTag(ENUMERATION_TAG);
}


void XsdWriter::WriteEnumerationClosingTag()
{
    WriteClosingTag(ENUMERATION_TAG);
}


void XsdWriter::WriteSequenceOpeningTag()
{
    WriteOpeningTag(SEQUENCE_TAG, true);
}


void XsdWriter::WriteSequenceClosingTag()
{
    WriteClosingTag(SEQUENCE_TAG);
}


void XsdWriter::WriteAllOpeningTag()
{
    WriteOpeningTag(ALL_TAG, true);
}


void XsdWriter::WriteAllClosingTag()
{
    WriteClosingTag(ALL_TAG);
}


void XsdWriter::WriteAnnotationOpeningTag()
{
    WriteOpeningTag(ANNOTATION_TAG, true);
}


void XsdWriter::WriteAnnotationClosingTag()
{
    WriteClosingTag(ANNOTATION_TAG);
}


void XsdWriter::WriteDocumentationOpeningTag()
{
    WriteOpeningTag(DOCUMENTATION_TAG);
}


void XsdWriter::WriteDocumentationClosingTag()
{
    WriteClosingTag(DOCUMENTATION_TAG);
}


void XsdWriter::WriteSimpleTypeOpeningTag()
{
    WriteOpeningTag(SIMPLE_TYPE_TAG, true);
}


void XsdWriter::WriteSimpleTypeClosingTag()
{
    WriteClosingTag(SIMPLE_TYPE_TAG);
}


void XsdWriter::WriteUnionOpeningTag()
{
    WriteOpeningTag(UNION_TAG, true);
}


void XsdWriter::WriteUnionClosingTag()
{
    WriteClosingTag(UNION_TAG);
}


void XsdWriter::WriteSimpleContentOpeningTag()
{
    WriteOpeningTag(SIMPLE_CONTENT_TAG, true);
}


void XsdWriter::WriteSimpleContentClosingTag()
{
    WriteClosingTag(SIMPLE_CONTENT_TAG);
}


void XsdWriter::WriteElementOpeningTag()
{
    WriteOpeningTag(ELEMENT_TAG);
}


void XsdWriter::WriteElementClosingTag()
{
    WriteClosingTag(ELEMENT_TAG);
}


void XsdWriter::WriteAttributeOpeningTag()
{
    WriteOpeningTag(ATTRIBUTE_TAG);
}


void XsdWriter::WriteAttributeClosingTag()
{
    WriteClosingTag(ATTRIBUTE_TAG);
}


void XsdWriter::WriteMinInclusiveOpeningTag()
{
    WriteOpeningTag(MIN_INCLUSIVE_TAG);
}


void XsdWriter::WriteMinInclusiveClosingTag()
{
    WriteClosingTag(MIN_INCLUSIVE_TAG);
}

void XsdWriter::WriteMaxInclusiveOpeningTag()
{
    WriteOpeningTag(MAX_INCLUSIVE_TAG);
}


void XsdWriter::WriteMaxInclusiveClosingTag()
{
    WriteClosingTag(MAX_INCLUSIVE_TAG);
}

void XsdWriter::WriteMinExclusiveOpeningTag()
{
    WriteOpeningTag(MIN_EXCLUSIVE_TAG);
}


void XsdWriter::WriteMinExclusiveClosingTag()
{
    WriteClosingTag(MIN_EXCLUSIVE_TAG);
}


void XsdWriter::WriteMaxExclusiveOpeningTag()
{
    WriteOpeningTag(MAX_EXCLUSIVE_TAG);
}


void XsdWriter::WriteMaxExclusiveClosingTag()
{
    WriteClosingTag(MAX_EXCLUSIVE_TAG);
}


void XsdWriter::WriteExtensionOpeningTag()
{
    WriteOpeningTag(EXTENSION_TAG);
}


void XsdWriter::WriteExtensionClosingTag()
{
    WriteClosingTag(EXTENSION_TAG);
}


void XsdWriter::WriteFieldOpeningTag()
{
    WriteOpeningTag(FIELD_TAG);
}


void XsdWriter::WriteFieldClosingTag()
{
    WriteClosingTag(FIELD_TAG);
}


void XsdWriter::WriteKeyOpeningTag()
{
    WriteOpeningTag(KEY_TAG);
}


void XsdWriter::WriteKeyClosingTag()
{
    WriteClosingTag(KEY_TAG);
}


void XsdWriter::WriteKeyrefOpeningTag()
{
    WriteOpeningTag(KEYREF_TAG);
}


void XsdWriter::WriteKeyrefClosingTag()
{
    WriteClosingTag(KEYREF_TAG);
}


void XsdWriter::WriteSelectorOpeningTag()
{
    WriteOpeningTag(SELECTOR_TAG);
}


void XsdWriter::WriteNameAttribute(const string& value)
{
    WriteQualifiedAttribute(NAME_ATTRIBUTE, value);
}


void XsdWriter::WriteValueAttribute(const string& value, const bool format)
{
    WriteAttribute(VALUE_ATTRIBUTE, value, value.size(), format);
}


void XsdWriter::WriteBaseAttribute(const eTypeCode iType)
{
    WriteAttribute(BASE_ATTRIBUTE, iType);
}


void XsdWriter::WriteFixedAttribute(const string& value)
{
    WriteAttribute(FIXED_ATTRIBUTE, value, value.size(), true);
}


void XsdWriter::WriteUseAttribute(const string& value)
{
    WriteAttribute(USE_ATTRIBUTE, value, value.size());
}


void XsdWriter::WriteMinOccursAttribute(const string& value)
{
    WriteAttribute(MIN_OCCURS_ATTRIBUTE, value, value.size());
}


void XsdWriter::WriteMaxOccursAttribute(const string& value)
{
    WriteAttribute(MAX_OCCURS_ATTRIBUTE, value, value.size());
}


void XsdWriter::WriteNillableAttribute(const string& value)
{
    WriteAttribute(NILLABLE_ATTRIBUTE, value, value.size());
}


void XsdWriter::WriteXpathAttribute(const string& value,
  const bool prependAtSymbol)
{
    if (prependAtSymbol)
        WriteQualifiedAttribute(XPATH_ATTRIBUTE, value, true, "@");
    else
        WriteQualifiedAttribute(XPATH_ATTRIBUTE, value, true);
}


void XsdWriter::WriteTypeAttribute(const string& value)
{
    WriteAttribute(TYPE_ATTRIBUTE, value, value.size());
}


void XsdWriter::WriteReferAttribute(const string& value)
{
    WriteQualifiedAttribute(REFER_ATTRIBUTE, value);
}


void XsdWriter::WriteTargetNamespaceAttribute(const string& value)
{
    WriteAttribute(TARGET_NAMESPACE_ATTRIBUTE, value, value.size(), false,
      true);
}


void XsdWriter::WriteElementFormDefaultAttribute(const string& value)
{
    WriteAttribute(ELEMENT_FORM_DEFAULT_ATTRIBUTE, value, value.size(),
      false, true);
}


void XsdWriter::WriteAttributeFormDefaultAttribute(const string& value)
{
    WriteAttribute(ATTRIBUTE_FORM_DEFAULT_ATTRIBUTE, value, value.size());
}


void XsdWriter::_WriteDataTypeAsAttribute(const eTypeCode iType)
{
    WriteAttribute(TYPE_ATTRIBUTE, iType);
}

