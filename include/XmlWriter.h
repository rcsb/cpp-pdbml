//$$FILE$$
//$$VERSION$$
//$$DATE$$
//$$LICENSE$$


/**
** \file XmlWriter.h
**
** XML Writer class.
*/


#ifndef XMLWRITER_H
#define XMLWRITER_H


#include <string>
#include <ostream>

#include "rcsb_types.h"


class XmlWriter
{
  public:
    XmlWriter(std::ostream& io, const std::string& ns);
    ~XmlWriter();

    const std::string& GetNamespace();

    void WriteDeclaration();
    void WriteXsdNamespace();
    void WriteXsiNamespace();

    void WriteLangAttribute();
    void WriteNamespaceAttribute(const std::string& ns,
      const std::string& value, const bool noSpace = false);
    void WriteSchemaLocationAttribute(const std::string& value,
      const bool noSpace = false);
    void WriteNilAttribute(const std::string& value);

    void WriteComment(const std::string& comment);


    void WriteOpeningTag(const std::string& tag,
      const bool closeBracket = false);
    void WriteQualifiedOpeningTag(const std::string& tag,
      const bool closeBracket = false);

    void WriteClosingTag(const std::string& tag = std::string());
    void WriteQualifiedClosingTag(const std::string& tag);

    void WriteAttribute(const std::string& name, const std::string& value,
      const unsigned int width, const bool format = false,
      const bool noSpace = false);
    void WriteQualifiedAttribute(const std::string& name,
      const std::string& value,
      const bool doNotQualifyUnderscore = false,
      const std::string& prependString = std::string());
    void WriteAttribute(const std::string& name, const eTypeCode iType);
    void WriteAttributeValue(const std::string& name, const std::string& value,
      const eTypeCode iType, const unsigned int width);

    void WriteNamespace();

    void WriteClosingBracket(const bool doNotAppendNewLine = false);


    void Indent();
    void IncrementIndent(const unsigned int indentLevels = 1);
    void DecrementIndent(const unsigned int indentLevels = 1);
    void SetIndentSpaces(const unsigned int indentSpaces);
    void WriteSpace();
    void WriteNewLine();


    void _FormatData(const std::string& value, const unsigned int type,
      const unsigned int width);

    void _FormatFloatDataXML(const std::string& cs);
    void _FormatIntegerDataXML(const std::string& cs);
    void _FormatStringDataXML(const std::string& cs, const unsigned int width);
    void _FormatTextDataXML(const std::string& cs);
    void _FormatDateDataXML(const std::string& cs, const unsigned int width);

  protected:
    std::ostream& _io;
    std::string _ns;

  private:
    unsigned int _indentSpaces;

    void _WriteDeclarationOpeningTag();
    void _WriteDeclarationClosingTag();

    void _WriteNamespaceXML(std::ostream& io, const std::string& ns);

    void _ConvertDataTypeXML(const eTypeCode iType);

    void _QualifyNameXML(std::ostream& io, const std::string& name,
      const std::string& ns = std::string(),
      const bool doNotPrepUndscoreToNumb = false,
      const bool doNotQualifyUnderscore = false);
};


#endif
