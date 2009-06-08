//$$FILE$$
//$$VERSION$$
//$$DATE$$
//$$LICENSE$$


// For localtime, time, tm on SGI platform. Investigate and remove this.
#include "time.h"

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "rcsb_types.h"
#include "GenString.h"
#include "CifString.h"
#include "GenCont.h"
#include "CifFile.h"
#include "CifParserBase.h"
#include "ParentChild.h"
#include "XsdWriter.h"
#include "PdbMlWriter.h"
#include "PdbMlSchema.h"


using std::string;
using std::vector;
using std::ostringstream;
using std::cerr;
using std::endl;
using std::sort;


const string PdbMlSchema::DATABLOCK_ELEMENT("datablock");
const string PdbMlSchema::DATABLOCK_NAME_ATTRIBUTE("datablockName");

const string PdbMlSchema::CATEGORY_ELEMENT_SUFFIX("Category");
const string PdbMlSchema::CATEGORY_TYPE_SUFFIX("Type");


static void format_cif_example(string& obuf, const string& str);
static void format_cif_descriptionXML(string& obuf, const string& str);
static void GetLineFromMultiline(string& line, string::size_type& iPos,
  const string& multiLine);
static void GetWordInLine(string& word, string::size_type& currLinePos,
  const string& line);


const string PdbMlSchema::SCHEMA_LOCATION_DIRECTORY
  ("http://pdbml.pdb.org/schema/");


PdbMlSchema::PdbMlSchema(XsdWriter& xsdWriter, ParentChild& parentChild,
  DataInfo& dataInfo, const string& ns, const string& prefix) :
  _xsdWriter(xsdWriter), _parentChild(parentChild), _dataInfo(dataInfo),
  _ns(ns), _prefix(prefix)
{
    if (!_ns.empty())
    {
        _nsPrefix = _ns + ":";
    }
}


void PdbMlSchema::Convert()
{
    string dictVer;
    _dataInfo.GetVersion(dictVer);

    if (dictVer.empty())
        dictVer = "1.00";

    _xsdWriter.WriteDeclaration();
    _xsdWriter.WriteNewLine();

    _WriteSchemaComment(dictVer);
    _xsdWriter.WriteNewLine();

    _xsdWriter.WriteSchemaOpeningTag();

    _WriteSchemaAttributes(_ns);
    _xsdWriter.WriteNewLine();

    vector<string> categories = _dataInfo.GetCatNames();
    sort(categories.begin(), categories.end());

    vector<string> items = _dataInfo.GetItemsNames();
    sort(items.begin(), items.end());

    _xsdWriter.IncrementIndent();

    _WriteCategoriesTypes(categories, items);

    _WriteDatablockType(categories);

    _WriteDatablockElement(categories);

    _xsdWriter.DecrementIndent();

    _xsdWriter.WriteSchemaClosingTag();
}


void PdbMlSchema::MakeSchemaFileName(string& schemaFileName,
   const string& prefix, const string& dictVer)
{
    schemaFileName = prefix;

    if (!dictVer.empty())
        schemaFileName += "-v" + dictVer;

    schemaFileName += ".xsd";
}


void PdbMlSchema::MakeFullSchemaFileName(string& schemaFileName,
   const string& prefix, const string& dictVer)
{
    schemaFileName = SCHEMA_LOCATION_DIRECTORY;

    string append;
    MakeSchemaFileName(append, prefix, dictVer);

    schemaFileName += append;
}


void PdbMlSchema::_WriteSchemaAttributes(const string& ns)
{
    _xsdWriter.WriteXsdNamespace();
    _xsdWriter.WriteNewLine();

    _xsdWriter.IncrementIndent();

    string fullSchemaFileName;
    MakeFullSchemaFileName(fullSchemaFileName, _prefix);
 
    _xsdWriter.Indent();
    _xsdWriter.WriteNamespaceAttribute(ns, fullSchemaFileName, true);
    _xsdWriter.WriteNewLine();

    _xsdWriter.Indent();
    _xsdWriter.WriteTargetNamespaceAttribute(fullSchemaFileName);
    _xsdWriter.WriteNewLine();
 
    _xsdWriter.Indent();
    _xsdWriter.WriteElementFormDefaultAttribute("qualified");
    _xsdWriter.WriteAttributeFormDefaultAttribute("unqualified");
    _xsdWriter.WriteClosingBracket();

    _xsdWriter.DecrementIndent();
}


void PdbMlSchema::_WriteSchemaComment(const string& dictVer)
{
    char timeString[31];

    time_t curr_time;
    time(&curr_time);

    struct tm* tmptr = localtime(&curr_time);
    strftime(timeString, 30, "%Y-%m-%d", tmptr);

    _xsdWriter.WriteComment("XSD type schema generated on "
      + string(timeString));

    string fullSchemaFileName;
    MakeFullSchemaFileName(fullSchemaFileName, _prefix, dictVer);
    _xsdWriter.WriteComment("Schema file: " + fullSchemaFileName);

    _xsdWriter.WriteComment("Please direct questions or comments to "\
      "John Westbrook (jwest@rcsb.rutgers.edu)");
}


void PdbMlSchema::_WriteCategoriesTypes(const vector<string>& categories,
  const vector<string>& items)
{
    for (unsigned int i=0; i < categories.size(); i++)
    {
        _xsdWriter.Indent();
        _xsdWriter.WriteComplexTypeOpeningTag();

        string catTypeName;
        PdbMlSchema::MakeCategoryTypeName(catTypeName, categories[i]);
        _xsdWriter.WriteNameAttribute(catTypeName);
        _xsdWriter.WriteClosingBracket();

        _xsdWriter.IncrementIndent();

        // Category level documentation  - 
        _WriteCategoryDocumentation(categories[i]);

        _xsdWriter.Indent();
        _xsdWriter.WriteSequenceOpeningTag();

        _xsdWriter.IncrementIndent();

        _xsdWriter.Indent();
        _xsdWriter.WriteElementOpeningTag();
        _xsdWriter.WriteNameAttribute(categories[i]);
        _xsdWriter.WriteMinOccursAttribute("0");
        _xsdWriter.WriteMaxOccursAttribute("unbounded");
        _xsdWriter.WriteClosingBracket();

        _xsdWriter.IncrementIndent();

        _xsdWriter.Indent();
        _xsdWriter.WriteComplexTypeOpeningTag(true);

        _xsdWriter.IncrementIndent();

        _xsdWriter.Indent();
        _xsdWriter.WriteAllOpeningTag();

        vector<string> keys = _dataInfo.GetCatKeys(categories[i]);
        sort (keys.begin(), keys.end());

        _xsdWriter.IncrementIndent();
        for (unsigned int j=0; j < items.size(); j++)
        {
            string categoryName;
            CifString::GetCategoryFromCifItem(categoryName, items[j]);

            if (categoryName.empty())
                continue;

            if (!String::IsCiEqual(categoryName, categories[i]))
                continue;

            if (GenCont::IsInVector(items[j], keys))
                continue;

            // Non-key attribures as elements 
            _WriteNonKeyItem(items[j]);
        }

        _xsdWriter.DecrementIndent();

        _xsdWriter.Indent();
        _xsdWriter.WriteAllClosingTag();

        for (unsigned int j=0; j < keys.size(); j++)
        {
            if (keys[j] == CifString::UnknownValue)
                continue;

            // Keys as attributes
            _WriteKeyItem(keys[j]);
        }

        _xsdWriter.DecrementIndent();

        _xsdWriter.Indent();
        _xsdWriter.WriteComplexTypeClosingTag();

        _xsdWriter.DecrementIndent();

        _xsdWriter.Indent();
        _xsdWriter.WriteElementClosingTag();

        _xsdWriter.DecrementIndent();

        _xsdWriter.Indent();
        _xsdWriter.WriteSequenceClosingTag();

        _xsdWriter.DecrementIndent();

        _xsdWriter.Indent();
        _xsdWriter.WriteComplexTypeClosingTag();

        _xsdWriter.WriteNewLine();
    }
}


void PdbMlSchema::_WriteDatablockType(const vector<string>& categories)
{
    // Define datablockType as unordered list of category elements!
    // Add <unique> and <key> elements here.

    _xsdWriter.Indent();
    _xsdWriter.WriteComplexTypeOpeningTag();
    string dataBlockCatTypeName;
    PdbMlSchema::MakeCategoryTypeName(dataBlockCatTypeName,
      DATABLOCK_ELEMENT);
    _xsdWriter.WriteNameAttribute(dataBlockCatTypeName);
    _xsdWriter.WriteClosingBracket();

    _xsdWriter.IncrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteAllOpeningTag();

    _xsdWriter.IncrementIndent();
    for (unsigned int i=0; i < categories.size(); i++)
    {
        _xsdWriter.Indent();
        _xsdWriter.WriteElementOpeningTag();

        string catElemName;
        PdbMlSchema::MakeCategoryElementName(catElemName, categories[i]);
        _xsdWriter.WriteNameAttribute(catElemName);

        string catTypeName;
        PdbMlSchema::MakeCategoryTypeName(catTypeName, categories[i]);
        _xsdWriter.WriteTypeAttribute(_nsPrefix + catTypeName);
        _xsdWriter.WriteMinOccursAttribute("0");
        _xsdWriter.WriteMaxOccursAttribute("1");
        _xsdWriter.WriteClosingBracket();

        _WriteCategoryKeys(categories[i]);

        _xsdWriter.Indent();
        _xsdWriter.WriteElementClosingTag();
    }

    _xsdWriter.DecrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteAllClosingTag();

    _xsdWriter.Indent();
    _xsdWriter.WriteAttributeOpeningTag();
    _xsdWriter.WriteNameAttribute(DATABLOCK_NAME_ATTRIBUTE);
    _xsdWriter._WriteDataTypeAsAttribute(eTYPE_CODE_STRING);
    _xsdWriter.WriteUseAttribute("optional");
    _xsdWriter.WriteClosingTag();

    _xsdWriter.DecrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteComplexTypeClosingTag();
}


void PdbMlSchema::_WriteDatablockElement(const vector<string>& categories)
{
    //  Instance of the datablock element including key/keyref elements.

    _xsdWriter.Indent();
    _xsdWriter.WriteElementOpeningTag();

    _xsdWriter.WriteNameAttribute(DATABLOCK_ELEMENT);
    string dataBlockTypeName;
    PdbMlSchema::MakeCategoryTypeName(dataBlockTypeName,
      DATABLOCK_ELEMENT);
    _xsdWriter.WriteTypeAttribute(_nsPrefix + dataBlockTypeName);
    _xsdWriter.WriteClosingBracket();

    _xsdWriter.IncrementIndent();

    for (unsigned int i=0; i < categories.size(); i++)
    {
        _WriteCategoryKeysAndKeyrefs(categories[i]); 
    }

    _xsdWriter.DecrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteElementClosingTag();
}


void PdbMlSchema::_WriteCategoryDocumentation(const string& catName) 
{
    // Category level documentation

    _xsdWriter.Indent();
    _xsdWriter.WriteAnnotationOpeningTag();

    _xsdWriter.IncrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteDocumentationOpeningTag();
    _xsdWriter.WriteLangAttribute();
    _xsdWriter.WriteClosingBracket();

    const vector<string>& categoryDescription = _dataInfo.GetCatAttribute(
      catName, CifString::CIF_DDL_CATEGORY_CATEGORY,
      CifString::CIF_DDL_ITEM_DESCRIPTION);

    string descriptionXML;
    format_cif_descriptionXML(descriptionXML, categoryDescription[0]);

    _xsdWriter._FormatTextDataXML(descriptionXML);
    if (!CifString::IsEmptyValue(descriptionXML))
        _xsdWriter.WriteNewLine();

    const vector<string>& exampleCase = _dataInfo.GetCatAttribute(catName,
      CifString::CIF_DDL_CATEGORY_CATEGORY_EXAMPLES,
      CifString::CIF_DDL_ITEM_CASE);

    const vector<string>& exampleDetail = _dataInfo.GetCatAttribute(catName,
      CifString::CIF_DDL_CATEGORY_CATEGORY_EXAMPLES,
      CifString::CIF_DDL_ITEM_DETAIL);

    for (unsigned int i = 0; i < exampleCase.size(); ++i)
    {
        _xsdWriter._FormatTextDataXML(exampleDetail[i]);
        if (!CifString::IsEmptyValue(exampleDetail[i]))
            _xsdWriter.WriteNewLine();

        if (CifString::IsEmptyValue(exampleCase[i]))
        {
            continue;
        }

        string exampleCIF;
        format_cif_example(exampleCIF, exampleCase[i]);

        ostringstream exampleXMLStream;

        CifFile* fobjIn = new CifFile();
        
        CifParser* cifParserR = new CifParser(fobjIn);

        string diags;
        cifParserR->ParseString(exampleCIF, diags);

        delete (cifParserR);

        if (!diags.empty())
        {
            cerr << "Diags for category " << catName << "  = " <<
              diags << endl;
        }

        PdbMlWriter pdbMlWriter(exampleXMLStream, _ns, _dataInfo);

        vector<string> blockNames;
        fobjIn->GetBlockNames(blockNames);
        for (unsigned int ib = 0; ib < blockNames.size(); ++ib)
        {
            const string& blockName = blockNames[ib];
            Block& block = fobjIn->GetBlock(blockName);
            vector<string> tableNames;
            block.GetTableNames(tableNames);
            for (unsigned int it = 0; it < tableNames.size(); ++it)
            {
                if (!_dataInfo.IsCatDefined(tableNames[it]))
                {
                    cerr << " Skipping conversion to XML of the unknown "\
                      "table \"" << tableNames[it] << "\"" << endl;
                    continue;
                }

                ISTable* t = block.GetTablePtr(tableNames[it]);
                if ((t != NULL) && (t->GetNumRows() > 0))
                {
                    vector<unsigned int> emptyWidths;
                    pdbMlWriter.WriteTable(t, emptyWidths);
                }
            }
        }

        delete (fobjIn);

        _xsdWriter._FormatTextDataXML(exampleXMLStream.str());
        if (!CifString::IsEmptyValue(exampleXMLStream.str()))
            _xsdWriter.WriteNewLine();
    }

    _xsdWriter.WriteNewLine();

    _xsdWriter.Indent();
    _xsdWriter.WriteDocumentationClosingTag();
 
    _xsdWriter.DecrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteAnnotationClosingTag();
}


void PdbMlSchema::_WriteNonKeyItem(const string& itemName)
{
    string attribName;
    CifString::GetItemFromCifItem(attribName, itemName);

    _xsdWriter.Indent();
    _xsdWriter.WriteElementOpeningTag();

    _xsdWriter.WriteNameAttribute(attribName);

    _WriteItemAttributes(itemName, false);

    _xsdWriter.Indent();
    _xsdWriter.WriteElementClosingTag();
}


void PdbMlSchema::_WriteKeyItem(const string& itemName) 
{
    string attribName;
    CifString::GetItemFromCifItem(attribName, itemName);

    _xsdWriter.Indent();
    _xsdWriter.WriteAttributeOpeningTag();

    _xsdWriter.WriteNameAttribute(attribName);

    _WriteItemAttributes(itemName, true);

    _xsdWriter.Indent();
    _xsdWriter.WriteAttributeClosingTag();
}


void PdbMlSchema::_WriteCategoryKeys(const string& catName) 
{
    vector<string> keys = _dataInfo.GetCatKeys(catName);
    sort (keys.begin(), keys.end());

    if (keys.empty())
    {
        return;
    }

    _xsdWriter.IncrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteUniqueOpeningTag();
    _xsdWriter.WriteNameAttribute(catName + "Unique");
    _xsdWriter.WriteClosingBracket();

    _xsdWriter.IncrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteSelectorOpeningTag();

    _xsdWriter.WriteXpathAttribute(catName);
    _xsdWriter.WriteClosingTag();

    for (unsigned int i=0; i < keys.size(); i++)
    {
      if (CifString::IsUnknownValue(keys[i]))
      {
          // VLAD - LOGGING IMPLEMENTATION
          cerr << "ERROR - Category \"" << catName <<
            "\" has invalid key \"" << keys[i] << "\"" << endl;

          continue;
      }

      string attribName;
      CifString::GetItemFromCifItem(attribName, keys[i]);

      _xsdWriter.Indent();
      _xsdWriter.WriteFieldOpeningTag();
      _xsdWriter.WriteXpathAttribute(attribName, true);
      _xsdWriter.WriteClosingTag();
    }

    _xsdWriter.DecrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteUniqueClosingTag();

    _xsdWriter.DecrementIndent();
}


void PdbMlSchema::_WriteCategoryKeysAndKeyrefs(const string& catName) 
{
    // First, write one combo key for all category keys irrespectively
    // whether they are linked to any children or not. This will be
    // identified as key 0.

    const vector<string>& catKeys = _dataInfo.GetCatKeys(catName);

    _WriteComboKey(catName, catKeys, String::IntToString(0));

    // Get all combo keys participating in parent-child relationships.
    const vector<vector<string> >& parComboKeys =
     _parentChild.GetComboKeys(catName);

    for (unsigned int keyI = 0; keyI < parComboKeys.size(); ++keyI)
    {
        bool parentKeyWritten = false;

        vector<vector<vector<string> > >& childrenKeys =
          _parentChild.GetChildrenKeys(parComboKeys[keyI]);

        for (unsigned int childI = 0; childI < childrenKeys.size(); ++childI)
        {
            string childCatName;
            if (!childrenKeys[childI].empty())
            {
                CifString::GetCategoryFromCifItem(childCatName,
                  childrenKeys[childI][0][0]); 
            }

            for (unsigned int childKeyI = 0; childKeyI <
              childrenKeys[childI].size(); ++childKeyI)
            {
                vector<string> chKeys;

                for (unsigned int keyItemI = 0; keyItemI <
                  childrenKeys[childI][childKeyI].size(); ++keyItemI)
                {
                    string chAttribName;
                    CifString::GetItemFromCifItem(chAttribName,
                      childrenKeys[childI][childKeyI][keyItemI]);
                    chKeys.push_back(chAttribName);
                }

                if (!parentKeyWritten)
                {
                    _WriteComboKey(catName, parComboKeys[keyI],
                      String::IntToString(keyI + 1));

                    parentKeyWritten = true;
                }

                string keyRefName = catName + "Keyref" + "_" +
                  String::IntToString(keyI + 1) + "_" +
                  String::IntToString(childI) + "_" +
                  String::IntToString(childKeyI);
                string keyName = _nsPrefix + catName + "Key" + "_" +
                  String::IntToString(keyI + 1);

                string childCatElemName;
                PdbMlSchema::MakeCategoryElementName(childCatElemName,
                  childCatName);

                string xPath = childCatElemName + string("/") + childCatName;

                sort(chKeys.begin(), chKeys.end());

                _WriteKeyRef(keyRefName, keyName, xPath, chKeys, childCatName);
            } // for (all child's keys)
        } // for (all children)
    } // for (all parent combo keys)
}


void PdbMlSchema::_WriteItemAttributes(const string& itemName,
  const bool isKey)
{
    if (isKey)
    {
        _xsdWriter.WriteUseAttribute("required");
    }
    else
    {
        string minOccurs;

        if (_dataInfo.IsItemMandatory(itemName))
        {
            minOccurs = "1";
        }
        else
        {
            minOccurs = "0";
        }

        _xsdWriter.WriteMinOccursAttribute(minOccurs);
        _xsdWriter.WriteMaxOccursAttribute("1");
        _xsdWriter.WriteNillableAttribute("true");
    }

    bool simpleTyping = _dataInfo.IsSimpleDataType(itemName);

    if (simpleTyping)
    {
        _WriteDataTypeAsAttribute(itemName);
    }

    _xsdWriter.WriteClosingBracket();

    _xsdWriter.IncrementIndent();

    _WriteItemDocumentation(itemName);

    if (!simpleTyping)
    {
        _WriteDataTypeAsElement(itemName);
    }

    _xsdWriter.DecrementIndent();
}


void PdbMlSchema::_WriteItemDocumentation(const string& itemName) 
{
    // Item level documentation  - 

    vector<string> description;
    description = _dataInfo.GetItemAttribute(itemName,
      CifString::CIF_DDL_CATEGORY_ITEM_DESCRIPTION,
      CifString::CIF_DDL_ITEM_DESCRIPTION);

    if (description.empty())
        description.push_back(string());
  
    string descriptionXML;
    format_cif_descriptionXML(descriptionXML, description[0]);

    _xsdWriter.Indent();
    _xsdWriter.WriteAnnotationOpeningTag();

    _xsdWriter.IncrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteDocumentationOpeningTag();
    _xsdWriter.WriteLangAttribute();
    _xsdWriter.WriteClosingBracket();

    if (!descriptionXML.empty())
    {
        _xsdWriter._FormatTextDataXML(descriptionXML);
        if (!CifString::IsEmptyValue(descriptionXML))
            _xsdWriter.WriteNewLine();
        if (!description[0].empty())
        {
            description[0].clear();
        }
    }

    const vector<string>& exampleCase = _dataInfo.GetItemAttribute(itemName,
      CifString::CIF_DDL_CATEGORY_ITEM_EXAMPLES,
      CifString::CIF_DDL_ITEM_CASE);

    const vector<string>& exampleDetail = _dataInfo.GetItemAttribute(itemName,
      CifString::CIF_DDL_CATEGORY_ITEM_EXAMPLES,
      CifString::CIF_DDL_ITEM_DETAIL);

    for (unsigned int i=0; i < exampleCase.size(); i++)
    {
        if (!exampleDetail.empty() && exampleDetail.size() > i)
        {
            _xsdWriter._FormatTextDataXML(exampleDetail[i]);
            if (!CifString::IsEmptyValue(exampleDetail[i]))
                _xsdWriter.WriteNewLine();
        }
        _xsdWriter._FormatTextDataXML(exampleCase[i]);
        if (!CifString::IsEmptyValue(exampleCase[i]))
            _xsdWriter.WriteNewLine();
    }

    _xsdWriter.Indent();
    _xsdWriter.WriteDocumentationClosingTag();
  
    _xsdWriter.DecrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteAnnotationClosingTag();
}


void PdbMlSchema::_WriteDataTypeAsElement(const string& itemName) 
{

  const vector<string>& rangeMin = _dataInfo.GetItemAttribute(itemName,
    CifString::CIF_DDL_CATEGORY_ITEM_RANGE,
    CifString::CIF_DDL_ITEM_MINIMUM);

  const vector<string>& rangeMax = _dataInfo.GetItemAttribute(itemName,
    CifString::CIF_DDL_CATEGORY_ITEM_RANGE,
    CifString::CIF_DDL_ITEM_MAXIMUM);

  bool iRange = false;

  if ((!rangeMin.empty()) && (!rangeMax.empty()) &&
    rangeMin.size() == rangeMax.size() && rangeMax.size() > 0)
  {
      iRange = true;
  }

  // 
  //  Assume that we may either have  range restrictions or an enumeration
  //  but not both!  Treat these cases first...
  //

  const vector<string>& enums = _dataInfo.GetItemAttribute(itemName,
    CifString::CIF_DDL_CATEGORY_ITEM_ENUMERATION,
    CifString::CIF_DDL_ITEM_VALUE);

  const vector<string>& units = _dataInfo.GetItemAttribute(itemName,
    CifString::CIF_DDL_CATEGORY_ITEM_UNITS,
    CifString::CIF_DDL_ITEM_CODE);

  eTypeCode iType = _dataInfo._GetDataType(itemName);

  if (!enums.empty())
  {
    _xsdWriter.Indent();
    _xsdWriter.WriteSimpleTypeOpeningTag();

    _xsdWriter.IncrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteRestrictionOpeningTag();
    _xsdWriter.WriteBaseAttribute(iType);
    _xsdWriter.WriteClosingBracket();

    _xsdWriter.IncrementIndent();
    for (unsigned int i=0; i < enums.size(); i++)
    {
        _xsdWriter.Indent();
        _xsdWriter.WriteEnumerationOpeningTag();
        _xsdWriter.WriteValueAttribute(enums[i], true);
        _xsdWriter.WriteClosingTag();
    }
    _xsdWriter.DecrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteRestrictionClosingTag();

    _xsdWriter.DecrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteSimpleTypeClosingTag();
  }
  else if (iRange)
  {
    _xsdWriter.Indent();
    _xsdWriter.WriteSimpleTypeOpeningTag();

    _xsdWriter.IncrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteUnionOpeningTag();

    _xsdWriter.IncrementIndent();

    for (unsigned int i=0; i < rangeMin.size(); i++)
    {
      _xsdWriter.Indent();
      _xsdWriter.WriteSimpleTypeOpeningTag();

      _xsdWriter.IncrementIndent();

      _xsdWriter.Indent();
      _xsdWriter.WriteRestrictionOpeningTag();
      _xsdWriter.WriteBaseAttribute(iType);
      _xsdWriter.WriteClosingBracket();

      _xsdWriter.IncrementIndent();
      if (String::IsCiEqual(rangeMin[i], rangeMax[i]))
      {

        if (!CifString::IsEmptyValue(rangeMin[i]))
        {
          _xsdWriter.Indent();
          _xsdWriter.WriteMinInclusiveOpeningTag();
          _xsdWriter.WriteValueAttribute(rangeMin[i]);
          _xsdWriter.WriteClosingTag();
	}

	if (!CifString::IsEmptyValue(rangeMax[i]))
        {
          _xsdWriter.Indent();
          _xsdWriter.WriteMaxInclusiveOpeningTag();
          _xsdWriter.WriteValueAttribute(rangeMax[i]);
          _xsdWriter.WriteClosingTag();
	}
      }
      else
      {
	if (!CifString::IsEmptyValue(rangeMin[i]))
        {
          _xsdWriter.Indent();
          _xsdWriter.WriteMinExclusiveOpeningTag();
          _xsdWriter.WriteValueAttribute(rangeMin[i]);
          _xsdWriter.WriteClosingTag();
	}
	if (!CifString::IsEmptyValue(rangeMax[i]))
        {
          _xsdWriter.Indent();
          _xsdWriter.WriteMaxExclusiveOpeningTag();
          _xsdWriter.WriteValueAttribute(rangeMax[i]);
          _xsdWriter.WriteClosingTag();
	}
      }
      _xsdWriter.DecrementIndent();

      _xsdWriter.Indent();
      _xsdWriter.WriteRestrictionClosingTag();

      _xsdWriter.DecrementIndent();

      _xsdWriter.Indent();
      _xsdWriter.WriteSimpleTypeClosingTag();
    }

    _xsdWriter.DecrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteUnionClosingTag();

    _xsdWriter.DecrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteSimpleTypeClosingTag();
  }
  else if (!units.empty())
  {
      _xsdWriter.Indent();
      _xsdWriter.WriteComplexTypeOpeningTag(true);

      _xsdWriter.IncrementIndent();

      _xsdWriter.Indent();
      _xsdWriter.WriteSimpleContentOpeningTag();

      _xsdWriter.IncrementIndent();

      _xsdWriter.Indent();
      _xsdWriter.WriteExtensionOpeningTag();
      _xsdWriter.WriteBaseAttribute(iType);
      _xsdWriter.WriteClosingBracket();

      _xsdWriter.IncrementIndent();

      _xsdWriter.Indent();
      _xsdWriter.WriteAttributeOpeningTag();
      _xsdWriter.WriteFixedAttribute(units[0]);
      _xsdWriter.WriteNameAttribute("units");
      _xsdWriter._WriteDataTypeAsAttribute(eTYPE_CODE_STRING);
      _xsdWriter.WriteUseAttribute("optional");
      _xsdWriter.WriteClosingTag();

      _xsdWriter.DecrementIndent();

      _xsdWriter.Indent();
      _xsdWriter.WriteExtensionClosingTag();

      _xsdWriter.DecrementIndent();

      _xsdWriter.Indent();
      _xsdWriter.WriteSimpleContentClosingTag();

      _xsdWriter.DecrementIndent();

      _xsdWriter.Indent();
      _xsdWriter.WriteComplexTypeClosingTag();
  }
}


void PdbMlSchema::_WriteDataTypeAsAttribute(const string& itemName)
{
    eTypeCode iType = _dataInfo._GetDataType(itemName);

    _xsdWriter._WriteDataTypeAsAttribute(iType);
}


void format_cif_example(string& obuf, const string& str)
{
    obuf.clear();

    if (str.empty())
    {
        return;
    }

    obuf += '\n';

    string tlineBuf;
    string::size_type currPos = 0;
 
    GetLineFromMultiline(tlineBuf, currPos, str);
    while (!tlineBuf.empty())
    {
        String::StripLeadingWs(tlineBuf);
        obuf += tlineBuf;
        GetLineFromMultiline(tlineBuf, currPos, str);
    }

    obuf += '\n';
    obuf += '\n';
}


void format_cif_descriptionXML(string& obuf, const string& str)
{
    // Read mmCIF description data and expand any explict item name reference
    // to attribute xxxx of category yyyy.
    obuf.clear();

    if (str.empty())
    {
        return;
    }

    string::size_type currPos = 0;
    string ilineBuf;

    GetLineFromMultiline(ilineBuf, currPos, str);
    while (!ilineBuf.empty())
    {
        String::StripLeadingWs(ilineBuf);

        unsigned int iFlag = 0;
        string olineBuf;
 
        string::size_type currLinePos = 0;

        string word;
        GetWordInLine(word, currLinePos, ilineBuf);

        while (!word.empty())
        {
            try
            {
                string categoryName;
                string attribName; 

                CifString::GetCategoryFromCifItem(categoryName, word); 
                CifString::GetItemFromCifItem(attribName, word); 

                if (attribName[attribName.size() - 1] == '.' ||
                  attribName[attribName.size() - 1] == ';' ||
                  attribName[attribName.size() - 1] == ',')
                {
#ifdef VLAD_TEST
                    cerr << "Wierd attrib name: \"" << attribName <<
                      "\"" << ", for token: \"" << word << "\"" << endl;
#endif
                    attribName.erase(attribName.size() - 1);
                }

                iFlag++;

                olineBuf += "attribute ";
                olineBuf += attribName;
                olineBuf += " in category ";
                olineBuf += categoryName; 

                if (word[word.size() - 1] == '.' ||
                  word[word.size() - 1] == ';' ||
                  word[word.size() - 1] == ',')
                {
                    olineBuf += word[word.size() - 1];
                }
            }
            catch (EmptyValueException)
            {
                olineBuf += word;
            }

            olineBuf += " ";

            GetWordInLine(word, currLinePos, ilineBuf);
        }

        if (ilineBuf.empty())
        {
            obuf += '\n';
        }
        else
        {
            if (iFlag == 1)
            {
                // If only one CIF item in the line
                obuf += olineBuf;
            }
            else
            {
#ifdef VLAD_TEST
                if (iFlag != 0)
                    cerr << "Entered iFlag diff than 1 and 0. iFlag is: " <<
                      iFlag << " , and ilineBuf is \"" << ilineBuf << "\"" << endl;
#endif
                obuf += ilineBuf;
            }
        }

        GetLineFromMultiline(ilineBuf, currPos, str);
    }
}


void GetLineFromMultiline(string& line, string::size_type& currPos,
  const string& multiLine)
{
    if (currPos == string::npos)
    {
        line.clear();

        return;
    }

    string::size_type newLineInd = multiLine.find('\n', currPos);

    if (newLineInd != string::npos)
    {
        // Found a newline. Copy including the newline.
        line.assign(multiLine, currPos, newLineInd - currPos + 1);

        currPos = newLineInd + 1;
    }
    else
    {
        // Copy until the end. Do not insert newline.
        line.assign(multiLine, currPos, multiLine.size() - currPos);

        currPos = string::npos;
    }
}


void GetWordInLine(string& word, string::size_type& currLinePos,
  const string& line)
{
    if (currLinePos == string::npos)
    {
        word.clear();

        return;
    }

    string delimiters("\t ");

    // Find first non-delimiting char
    string::size_type nonDelimInd = line.find_first_not_of(delimiters,
      currLinePos);

    if (nonDelimInd == string::npos)
    {
        word.clear();

        currLinePos = string::npos;

        return;
    }

    // Find first delimiting char
    string::size_type delimInd = line.find_first_of(delimiters, nonDelimInd);

    if (delimInd != string::npos)
    {
        // Found a delimiter. Copy without delimiter.
        word.assign(line, nonDelimInd, delimInd - nonDelimInd);

        currLinePos = delimInd + 1;
    }
    else
    {
        // Copy until the end.
        word.assign(line, nonDelimInd, line.size() - nonDelimInd);

        currLinePos = string::npos;
    }
}


void PdbMlSchema::MakeCategoryElementName(string& catElemName,
  const string& catName)
{
    catElemName = catName + CATEGORY_ELEMENT_SUFFIX;
}


void PdbMlSchema::MakeCategoryTypeName(string& catTypeName,
  const string& catName)
{
    catTypeName = catName + CATEGORY_TYPE_SUFFIX;
}


void PdbMlSchema::_WriteComboKey(const string& catName, 
  const vector<string>& keyItems, const string& append)
{
    bool emptyKeyItems = true;

    for (unsigned int i = 0; i < keyItems.size(); ++i)
    {
        if (_dataInfo.IsItemDefined(keyItems[i]))
        {
            emptyKeyItems = false;
            break;
        }
    }

    if (emptyKeyItems)
    {
        return;
    }

    vector<string> sortedKeyItems = keyItems;
    sort(sortedKeyItems.begin(), sortedKeyItems.end());

    _xsdWriter.Indent();
    _xsdWriter.WriteKeyOpeningTag();
    _xsdWriter.WriteNameAttribute(catName + "Key" + "_" + append);
    _xsdWriter.WriteClosingBracket();

    _xsdWriter.IncrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteSelectorOpeningTag();

    string catElemName;
    PdbMlSchema::MakeCategoryElementName(catElemName, catName);
    _xsdWriter.WriteXpathAttribute(catElemName + string("/") + catName);
    _xsdWriter.WriteClosingTag();

    for (unsigned int i = 0; i < sortedKeyItems.size(); ++i)
    {
        if (!_dataInfo.IsItemDefined(sortedKeyItems[i]))
        {
            // VLAD - Log the undefined/invalid key item
            continue;
        }

        string attribName;
        CifString::GetItemFromCifItem(attribName, sortedKeyItems[i]);

        _xsdWriter.Indent();
        _xsdWriter.WriteFieldOpeningTag();

        _xsdWriter.WriteXpathAttribute(attribName, true);
        _xsdWriter.WriteClosingTag();
    }

    _xsdWriter.DecrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteKeyClosingTag();
}


void PdbMlSchema::_WriteKeyRef(const string& keyRefName, const string& keyName,
  const string& xPath, const vector<string>& childKeys,
  const string& childCatName)
{
    if (childKeys.empty())
    {
        throw EmptyContainerException("No child keys for keyref \"" +
          keyRefName + "\"", "PdbMlSchema::_WriteKeyRef()");
    }

    _xsdWriter.Indent();
    _xsdWriter.WriteKeyrefOpeningTag();
    _xsdWriter.WriteNameAttribute(keyRefName);

    _xsdWriter.WriteReferAttribute(keyName);
    _xsdWriter.WriteClosingBracket();

    _xsdWriter.IncrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteSelectorOpeningTag();
    _xsdWriter.WriteXpathAttribute(xPath);
    _xsdWriter.WriteClosingTag();

    for (unsigned int keyI = 0; keyI < childKeys.size(); ++keyI)
    {
        _xsdWriter.Indent();
        _xsdWriter.WriteFieldOpeningTag();

        if (_dataInfo.IsKeyItem(childCatName, childKeys[keyI]))
        {
            _xsdWriter.WriteXpathAttribute(childKeys[keyI], true);
        }
        else
        {
            _xsdWriter.WriteXpathAttribute(childKeys[keyI]);
        }

        _xsdWriter.WriteClosingTag();
    }

    _xsdWriter.DecrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteKeyrefClosingTag();
}
