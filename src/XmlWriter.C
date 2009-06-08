//$$FILE$$
//$$VERSION$$
//$$DATE$$
//$$LICENSE$$


#include <exception>
#include <stdexcept>
#include <string>
#include <istream>
#include <iostream>

#include "rcsb_types.h"
#include "CifString.h"
#include "GenString.h"
#include "XmlWriter.h"


using std::exception;
using std::out_of_range;
using std::string;
using std::ostream;
using std::cerr;
using std::endl;


XmlWriter::XmlWriter(ostream& io, const string& ns) : _io(io), _ns(ns),
  _indentSpaces(0)
{

}


XmlWriter::~XmlWriter()
{

}


const string& XmlWriter::GetNamespace()
{
    return (_ns);
}


void XmlWriter::WriteDeclaration()
{
    _WriteDeclarationOpeningTag();

    WriteAttribute("version", "1.0", string("1.0").size());

    WriteAttribute("encoding", "UTF-8", string("UTF-8").size());

    _WriteDeclarationClosingTag();
}


void XmlWriter::WriteXsdNamespace()
{
    WriteAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema",
      string("http://www.w3.org/2001/XMLSchema").size());
}


void XmlWriter::WriteXsiNamespace()
{
    WriteAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance",
      string("http://www.w3.org/2001/XMLSchema-instance").size(), false, true);
}


void XmlWriter::WriteLangAttribute()
{
    WriteAttribute("xml:lang", "en", string("en").size());
}


void XmlWriter::WriteNamespaceAttribute(const string& ns, const string& value,
  const bool noSpace)
{
    if (!ns.empty())
        WriteAttribute("xmlns:" + ns, value, value.size(), false, noSpace);
    else
        WriteAttribute("xmlns", value, value.size(), false, noSpace);
}


void XmlWriter::WriteSchemaLocationAttribute(const string& value,
  const bool noSpace)
{
    WriteAttribute("xsi:schemaLocation", value, value.size(), false, noSpace);
}


void XmlWriter::WriteNilAttribute(const string& value)
{
    WriteAttribute("xsi:nil", value, value.size());
}


void XmlWriter::WriteComment(const string& comment)
{
    _io << "<!-- " << comment << " -->";
    _io << endl;
}


void XmlWriter::WriteOpeningTag(const string& tag, const bool closeBracket)
{
    _io << "<";
    WriteNamespace();
    _io << tag;

    if (closeBracket)
    {
        WriteClosingBracket();
    }
}

void XmlWriter::WriteQualifiedOpeningTag(const string& tag,
  const bool closeBracket)
{
    _io << "<";

    _QualifyNameXML(_io, tag, _ns);

    if (closeBracket)
    {
        WriteClosingBracket();
    }
}


void XmlWriter::WriteClosingTag(const string& tag)
{
    if (!tag.empty())
        _io << "<";
    else
        _io << " ";

    _io << "/";

    if (!tag.empty())
    {
        WriteNamespace();
        _io << tag;
    }

    WriteClosingBracket();
}


void XmlWriter::WriteQualifiedClosingTag(const string& tag)
{
    _io << "</";

    _QualifyNameXML(_io, tag, _ns);

    WriteClosingBracket();
}


void XmlWriter::WriteAttribute(const string& name, const string& value, 
  const unsigned int width, const bool format, const bool noSpace)
{
    if (!noSpace)
        _io << " ";

    _io << name << "=";

    _io << "\"";

    if (format)
        _FormatStringDataXML(value, width);
    else
      _io << value;

    _io << "\"";
}


void XmlWriter::WriteQualifiedAttribute(const string& name,
  const string& value, const bool doNotQualifyUnderscore,
  const string& prependString)
{
    _io << " ";

    _io << name << "=";
    _io << "\"";
    if (!prependString.empty())
    {
        _io << prependString;
    }

    if (doNotQualifyUnderscore)
        _QualifyNameXML(_io, value, string(), false, true);
    else
        _QualifyNameXML(_io, value);

    _io << "\"";

}


void XmlWriter::WriteAttribute(const string& name, const eTypeCode iType)
{
    _io << " ";
    _io << name << "=";
    _io << "\"";
    _ConvertDataTypeXML(iType);
    _io << "\"";
}


void XmlWriter::WriteAttributeValue(const string& name,
  const string& value, const eTypeCode iType, const unsigned int width)
{
    _io << " ";
    _QualifyNameXML(_io, name);

    _io << "=\"";

    _FormatData(value, iType, width);

    _io << "\"";
}


void XmlWriter::WriteNamespace()
{
    if (!_ns.empty())
    {
        _io << _ns << ":";
    }
}


void XmlWriter::WriteClosingBracket(const bool doNotAppendNewLine)
{
    _io << ">";

    if (!doNotAppendNewLine)
    {
        _io << endl;
    }
}


void XmlWriter::Indent()
{
    for (unsigned int i = 0; i < _indentSpaces; ++i)
    {
        _io << " ";
    }
}


void XmlWriter::IncrementIndent(const unsigned int indentLevels)
{
    _indentSpaces += (indentLevels * 3);
}


void XmlWriter::DecrementIndent(const unsigned int indentLevels)
{
    if (_indentSpaces != 0)
    {
        _indentSpaces -= (indentLevels * 3);
    }
}


void XmlWriter::SetIndentSpaces(const unsigned int indentSpaces)
{
    _indentSpaces = indentSpaces;
}


void XmlWriter::WriteSpace()
{
    _io << " ";
}


void XmlWriter::WriteNewLine()
{
    _io << endl;
}


void XmlWriter::_FormatData(const string& value, const unsigned int type,
  const unsigned int width)
{
    switch (type)
    {
        case eTYPE_CODE_INT: 
            _FormatIntegerDataXML(value);
            break;
        case eTYPE_CODE_FLOAT:
            _FormatFloatDataXML(value);
            break;
        case eTYPE_CODE_STRING:
            _FormatStringDataXML(value, width);
            break;
        case eTYPE_CODE_TEXT:
            _FormatTextDataXML(value);
            break;
        case eTYPE_CODE_DATETIME:
            _FormatDateDataXML(value, width);
            break;
        default:
        {
            throw out_of_range("Invalid type code in XmlWriter::_FormatData");
        }
    }
}


void XmlWriter::_FormatIntegerDataXML(const string& cs) 
{
    try
    {
        String::StringToInt(cs);
    }
    catch (const exception& exc)
    {
        cerr << exc.what() << endl;

        cerr << "Warning - Value \"" << cs << "\" is not an integer. Will be "\
          "written as is." << endl;

        _io << cs;

        throw;
    }

    _io << cs;
}


void XmlWriter::_FormatFloatDataXML(const string& cs) 
{
    try
    {
        String::StringToDouble(cs);

        if (String::IsScientific(cs))
        {
            string fixedFormatCs;

            String::ToFixedFormat(fixedFormatCs, cs);

            _io << fixedFormatCs;
        }
        else
        {
            _io << cs;
        }
    }
    catch (const exception& exc)
    {
        cerr << exc.what() << endl;

        cerr << "Warning - Value \"" << cs << "\" is not a float. Will be "\
          "written as is." << endl;

        _io << cs;

        throw;
    }
}


void XmlWriter::_FormatStringDataXML(const string& cs,
  const unsigned int width) 
{
    // A null string or the special CIF characters are treated as NULL . 
    if (CifString::IsEmptyValue(cs))
    {
        return;
    }

    // A zero length string is treated as BLANK
    // VLAD POTENTIAL PROBLEM
    if (cs.empty())
    {
        return;
    }

    unsigned int len = cs.size();

    if (len > width)
        len = width;

    for (unsigned int i=0; i < len; i++)
    {
        if (Char::IsWhiteSpace(cs[i]))   // convert all white space to SPACE
            _io << " ";
        else if (cs[i] == '>')
            _io << "&gt;";
        else if (cs[i] =='<')
            _io << "&lt;";
        else if (cs[i] =='\'')
            _io << "&apos;";
        else if (cs[i] =='\"')  
            _io << "&quot;";
        else if (cs[i] =='&')
            _io << "&amp;";
        else if (cs[i] =='%')
            _io << "&#37;";
        else
            _io << cs[i];
    }
}


void XmlWriter::_FormatTextDataXML(const string& cs) 
{

    // A null string or the special CIF characters are treated as NULL . 
    if (CifString::IsEmptyValue(cs))
    {
        return;
    }

    // A zero length string is treated as BLANK
    // VLAD POTENTIAL PROBLEM
    if (cs.empty())
    {
        return;
    }

    for (unsigned int i = 0; i < cs.size(); ++i)
    {
        if (Char::IsWhiteSpace(cs[i]) && cs[i] != '\n')
        // convert all white space to SPACE
            _io << " ";
        else if (cs[i] == '>') 
            _io << "&gt;";
        else if (cs[i] =='<')
            _io << "&lt;";
        else if (cs[i] =='\'')  
            _io << "&apos;";
        else if (cs[i] =='\"')  
            _io << "&quot;";
        else if (cs[i] =='&')  
            _io << "&amp;";
        else if (cs[i] =='%')  
            _io << "&#37;";
        else
            _io << cs[i];
    }
}


void XmlWriter::_FormatDateDataXML(const string& cs,
  const unsigned int width) 
{
    if (CifString::IsEmptyValue(cs))
    {
        return;
    }

    unsigned int len = cs.size();

    if (len > width)
        len = width;

    for (unsigned int i = 0; i < len; ++i)
    {
        if (Char::IsWhiteSpace(cs[i]))
        {
            // skip any white space
            continue;
        }
        else
        {
            _io << cs[i];
        }
    }
}


void XmlWriter::_WriteDeclarationOpeningTag()
{
    _io << "<?xml";
}


void XmlWriter::_WriteDeclarationClosingTag()
{
    _io << " ?>";
    _io << endl;
}


void XmlWriter::_QualifyNameXML(ostream& io, const string& name,
  const string& ns, const bool doNotPrepUndscoreToNumb,
  const bool doNotQualifyUnderscore)
{
    _WriteNamespaceXML(io, ns);

    if (!doNotPrepUndscoreToNumb)
    {
        if (!name.empty())
        {
            if (Char::IsDigit(name[0]))
                io << '_';
        }
    }

    for (unsigned int i = 0; i < name.size(); ++i)
    {
        if (name[i] == '[' || name[i] == ']' || name[i] == '%' ||
          name[i] == '<' || name[i] == '>') 
        {
            continue;
        }
        else if (name[i] == '/')
        {
            if (!doNotQualifyUnderscore)
                io << "_over_";
            else
                io << name[i];
            continue;
        }

        io << name[i];
    }
}


void XmlWriter::_WriteNamespaceXML(ostream& io, const string& ns)
{
    if (!ns.empty())
    {
        io << ns << ":";
    }
}


void XmlWriter::_ConvertDataTypeXML(const eTypeCode iType)
{
    if (iType == eTYPE_CODE_INT)
    {
        _io << "xsd:integer";
    }
    else if (iType == eTYPE_CODE_FLOAT)
    {
        _io << "xsd:decimal";
    }
    else if (iType == eTYPE_CODE_STRING)
    {
        _io << "xsd:string";
    }
    else if (iType == eTYPE_CODE_TEXT)
    {
        _io << "xsd:string";
    }
    else if (iType == eTYPE_CODE_DATETIME)
    {
        _io << "xsd:date";
    }
    else
    {
        throw out_of_range("Invalid type code in "\
          "XmlWriter::_ConvertDataTypeXML");
    }
}

