//$$FILE$$
//$$VERSION$$
//$$DATE$$
//$$LICENSE$$


// For localtime, time, tm on SGI platform. Investigate and remove this.
#include "time.h"

#include <stdexcept>
#include <vector>
#include <set>
#include <map>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "rcsb_types.h"
#include "GenString.h"
#include "CifString.h"
#include "GenCont.h"
#include "CifFile.h"
#include "CifFileUtil.h"
#include "CifExcept.h"
#include "ParentChild.h"
#include "XsdWriter.h"
#include "PdbMlWriter.h"
#include "PdbMlSchema.h"


using std::exception;
using std::pair;
using std::make_pair;
using std::string;
using std::vector;
using std::set;
using std::map;
using std::multimap;
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

        vector<string> keys = _dataInfo.GetCatKeys(categories[i]);
        sort(keys.begin(), keys.end());

        // Figure out the non-key category items
        vector<string> nonKeyItems;
        for (unsigned int j = 0; j < items.size(); ++j)
        {
            string categoryName;
            CifString::GetCategoryFromCifItem(categoryName, items[j]);

            if (categoryName.empty())
                continue;

            if (!String::IsCiEqual(categoryName, categories[i]))
                continue;

            if (GenCont::IsInVector(items[j], keys))
                continue;

            // Non-key item of this cateogory detected
            nonKeyItems.push_back(items[j]);
        }

        if (!nonKeyItems.empty())
        {
            _xsdWriter.Indent();
            _xsdWriter.WriteAllOpeningTag();

            _xsdWriter.IncrementIndent();
            for (unsigned int j = 0; j < nonKeyItems.size(); ++j)
            {
                _WriteNonKeyItem(nonKeyItems[j]);
            }

            _xsdWriter.DecrementIndent();

            _xsdWriter.Indent();
            _xsdWriter.WriteAllClosingTag();
        }

        for (unsigned int j = 0; j < keys.size(); ++j)
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
        _WriteCategoryKeys(categories[i]);
        _WriteCategoryKeysAndKeyrefs(categories[i]); 
    }

    _xsdWriter.DecrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteElementClosingTag();
}


void PdbMlSchema::_WriteCategoryDocumentation(const string& catName) 
{
    // Category level documentation
    if (catName.empty())
    {
        cerr << "ERROR: Empty category name detected in \""\
          "PdbMlSchema::_WriteCategoryDocumentation()\"" << endl;

        return;
    }

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

        try
        {
            ostringstream exampleXMLStream;

            CifFile* fobjIn = ParseCifString(exampleCIF);

            const string& parsingDiags = fobjIn->GetParsingDiags();

            if (!parsingDiags.empty())
            {
                cerr << "Diags for category " << catName << "  = " <<
                  parsingDiags << endl;
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
        catch (const exception& exc)
        {
            cerr << "Skipping conversion to XML of the bad CIF "\
              "segment \"" << exampleCase[i] << "\"" << endl;
        }
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
    sort(keys.begin(), keys.end());

    if (keys.empty())
    {
        return;
    }

    // Write category keys
    vector<string> sortedCatKeys = keys;
    sort(sortedCatKeys.begin(), sortedCatKeys.end());

    _WriteComboKey(catName, sortedCatKeys, String::IntToString(0));

#ifdef VLAD_KEYS_ONLY_REFERENCES
    return;
#endif

    // Write other keys
    vector<vector<string> > parComboKeys;
    vector<vector<vector<vector<string> > > > allChildrenKeys;

    _FilterKeys(parComboKeys, allChildrenKeys, catName);

    // Id of 0 is reserved for all category keys

    unsigned keyId = 1;

    for (unsigned int keyI = 0; keyI < parComboKeys.size(); ++keyI)
    {
        if (parComboKeys[keyI].empty())
        {
            continue;
        }

        if (allChildrenKeys[keyI].empty())
        {
            continue; 
        }

#ifdef VLAD_KEYS_WITH_MANDATORY_DET_REFERENCES
        if (_AreAllKeyItems(parComboKeys[keyI]))
        {
            continue;
        }

        if (!_AreSupersetOfAllKeyItems(parComboKeys[keyI]))
        {
            continue;
        }
#endif

#ifdef VLAD_NO_KEYS_WITH_MANDATORY_DET_REFERENCES
        if (_AreAllKeyItems(parComboKeys[keyI]))
        {
            continue;
        }
#endif

#ifdef VLAD_NO_KEYS_NO_MANDATORY_DET_REFERENCES
        if (_AreAllKeyItems(parComboKeys[keyI]))
        {
            continue;
        }
#endif

        vector<string> sortedParComboKey = parComboKeys[keyI];
        sort(sortedParComboKey.begin(), sortedParComboKey.end());

        if (_WriteComboKey(catName, sortedParComboKey,
          String::IntToString(keyId), false))
        {
            keyId++;
        }
    }
}


void PdbMlSchema::_WriteCategoryKeysAndKeyrefs(const string& catName) 
{
    //const vector<string>& catKeys = _dataInfo.GetCatKeys(catName);

    vector<vector<string> > parComboKeys;
    vector<vector<vector<vector<string> > > > allChildrenKeys;

    _FilterKeys(parComboKeys, allChildrenKeys, catName);

    if (allChildrenKeys.empty())
        return;

    // Start from 1, as keyId of 0 is reserved for category primary key.
    // Category primary key consists of items that are all defined as
    // category keys.
    unsigned int keyId = 0;

    // This is used in order to prepend keyrefs that point to
    // category primary key.
    //unsigned int altKeyId = 0; 

    for (unsigned int keyI = 0; keyI < parComboKeys.size(); ++keyI)
    {
#ifdef VLAD_KEYS_ONLY_REFERENCES
        if (!_AreAllKeyItems(parComboKeys[keyI]))
        {
            continue;
        }
#endif

#ifdef VLAD_KEYS_WITH_MANDATORY_DET_REFERENCES
        if (!_AreSupersetOfAllKeyItems(parComboKeys[keyI]))
        {
            continue;
        }
#endif

        if (allChildrenKeys[keyI].empty())
            continue;

        const vector<vector<vector<string> > >& childrenKeys =
          allChildrenKeys[keyI];

        if (!_AreAllKeyItems(parComboKeys[keyI]))
        {
            keyId++;
        }

        for (unsigned int keyItemI = 0; keyItemI < parComboKeys[keyI].size();
          ++keyItemI)
        {
            if (_HasMultipleSubRanges(parComboKeys[keyI][keyItemI]))
            {
                cerr << "WARNING: Detected multiple permitted value"\
                  " ranges for item " + parComboKeys[keyI][keyItemI] + 
                  ", which would participate in"\
                  " key/keyref relationship as a union. Xerces"\
                  " validator would flag all values of this item"\
                  " as key/keyref errors, while other validators"\
                  " would not. It is advised to remove this item from"\
                  " parent/child relationship, if value ranges are"\
                  " to be kept, or change value ranges to a single"\
                  " range if parent/child relationship is to be kept."
                  << endl;
            }
        }

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
                unsigned int usedKeyId = keyId;
                if (_AreAllKeyItems(parComboKeys[keyI]))
                {
                    usedKeyId = 0;
                }

                string keyRefPrefix = String::IntToString(keyI) + "_" +
                  String::IntToString(usedKeyId);

                string keyRefName = catName + "Keyref" + "_" +
                  keyRefPrefix + "_" +
                  String::IntToString(childI) + "_" +
                  String::IntToString(childKeyI);

                string keyName = _nsPrefix + catName + "Key" + "_" +
                  String::IntToString(usedKeyId);


                if (usedKeyId != 0)
                {
                    keyName = _nsPrefix + catName + "Unique" + "_" +
                      String::IntToString(usedKeyId);
                }

                string childCatElemName;
                PdbMlSchema::MakeCategoryElementName(childCatElemName,
                  childCatName);

                string xPath = _nsPrefix + childCatElemName + string("/") +
                  _nsPrefix + childCatName;

                multimap<string, string, StringLess> keyMap;

                for (unsigned int keyItemI = 0; keyItemI <
                  childrenKeys[childI][childKeyI].size(); ++keyItemI)
                {
                    if (_HasMultipleSubRanges(childrenKeys[childI][childKeyI][keyItemI]))
                    {
                        cerr << "WARNING: Detected multiple permitted value"\
                          " ranges for item " + childrenKeys[childI][childKeyI][keyItemI] + ", which would participate in"\
                          " key/keyref relationship as a union. Xerces"\
                          " validator would flag all values of this item"\
                          " as key/keyref errors, while other validators"\
                          " would not. It is advised to remove this item from"\
                          " parent/child relationship, if value ranges are"\
                          " to be kept, or change value ranges to a single"\
                          " range if parent/child relationship is to be kept."
                          << endl;
                    }
                    string chAttribName;
                    CifString::GetItemFromCifItem(chAttribName,
                      childrenKeys[childI][childKeyI][keyItemI]);
                    multimap<string, string, StringLess>::value_type
                      valuePairIndex(parComboKeys[keyI][keyItemI],
                      chAttribName);
                    keyMap.insert(valuePairIndex);
                }

                vector<string> chKeys;
                for (multimap<string, string, StringLess>::iterator iter =
                  keyMap.begin(); iter != keyMap.end(); ++iter)
                {
                    chKeys.push_back((*iter).second);
                }

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

        if (_IsSkipParentItem(itemName) || _IsSkipChildItem(itemName))
        {
            _xsdWriter.WriteNillableAttribute("true");
        }
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
  
  bool allEmptyBoundaries = true;
  for (unsigned int i=0; i < rangeMin.size(); i++)
  {
      if (CifString::IsEmptyValue(rangeMin[i]) &&
        CifString::IsEmptyValue(rangeMax[i]))
          continue;

      allEmptyBoundaries = false;
      break;
  }

  if (allEmptyBoundaries)
      iRange = false;
  else
      iRange = true;


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
    vector<string> incRangeMin;
    vector<string> incRangeMax;

    _AggregateInclusiveRanges(rangeMin, rangeMax, incRangeMin, incRangeMax);

    /*
    VG_DEBUG
    if (incRangeMin.size() > 1)
    {
        cerr << "INTERESTING INFO: More than one sub-range"\
          " for item " << itemName << endl;
    }
    VG_DEBUG
    */

    if (incRangeMin.size() == 1)
    {
    string newRangeMin = incRangeMin[0];
    string newRangeMax = incRangeMax[0];

    _xsdWriter.Indent();
    _xsdWriter.WriteSimpleTypeOpeningTag();

    _xsdWriter.IncrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteRestrictionOpeningTag();
    _xsdWriter.WriteBaseAttribute(iType);
    _xsdWriter.WriteClosingBracket();

    _xsdWriter.IncrementIndent();

    if ((!newRangeMin.empty()) && (newRangeMin != CifString::InapplicableValue))
    {
        _xsdWriter.Indent();
        _xsdWriter.WriteMinInclusiveOpeningTag();
        _xsdWriter.WriteValueAttribute(newRangeMin);
        _xsdWriter.WriteClosingTag();
    }

    if ((!newRangeMax.empty()) && (newRangeMax != CifString::InapplicableValue))
    {
        _xsdWriter.Indent();
        _xsdWriter.WriteMaxInclusiveOpeningTag();
        _xsdWriter.WriteValueAttribute(newRangeMax);
        _xsdWriter.WriteClosingTag();
    }

    _xsdWriter.DecrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteRestrictionClosingTag();

    _xsdWriter.DecrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteSimpleTypeClosingTag();

    }
    else
    {
    
    _xsdWriter.Indent();
    _xsdWriter.WriteSimpleTypeOpeningTag();

    _xsdWriter.IncrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteUnionOpeningTag();

    _xsdWriter.IncrementIndent();

    for (unsigned int i=0; i < rangeMin.size(); i++)
    {
       if (CifString::IsEmptyValue(rangeMin[i]) &&
          CifString::IsEmptyValue(rangeMax[i]))
            continue;

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
            catch (EmptyValueException& exc)
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


bool PdbMlSchema::_WriteComboKey(const string& catName, 
  const vector<string>& keyItems, const string& append, const bool asXsdKey)
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
        return (false);
    }

    string keyType;

    _xsdWriter.Indent();

    if (asXsdKey)
    {
        keyType = "Key";
        _xsdWriter.WriteKeyOpeningTag();
    }
    else
    {
        keyType = "Unique";
        _xsdWriter.WriteUniqueOpeningTag();
    }

    _xsdWriter.WriteNameAttribute(catName + keyType + "_" + append);
    _xsdWriter.WriteClosingBracket();

    _xsdWriter.IncrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteSelectorOpeningTag();

    string catElemName;
    PdbMlSchema::MakeCategoryElementName(catElemName, catName);
    _xsdWriter.WriteXpathAttribute(_nsPrefix + catElemName + string("/") +
      _nsPrefix + catName);
    _xsdWriter.WriteClosingTag();

    for (unsigned int i = 0; i < keyItems.size(); ++i)
    {
        if (!_dataInfo.IsItemDefined(keyItems[i]))
        {
            // VLAD - Log the undefined/invalid key item
            continue;
        }

        string attribName;
        CifString::GetItemFromCifItem(attribName, keyItems[i]);

        _xsdWriter.Indent();
        _xsdWriter.WriteFieldOpeningTag();

        if (_dataInfo.IsKeyItem(catName, attribName))
        {
            _xsdWriter.WriteXpathAttribute(attribName, true);
        }
        else
        {
            _xsdWriter.WriteXpathAttribute(_nsPrefix + attribName);
        }

        _xsdWriter.WriteClosingTag();
    }

    _xsdWriter.DecrementIndent();

    _xsdWriter.Indent();

    if (asXsdKey)
    {
        _xsdWriter.WriteKeyClosingTag();
    }
    else
    {
        _xsdWriter.WriteUniqueClosingTag();
    }

    return (true);
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
            _xsdWriter.WriteXpathAttribute(_nsPrefix + childKeys[keyI]);
        }

        _xsdWriter.WriteClosingTag();
    }

    _xsdWriter.DecrementIndent();

    _xsdWriter.Indent();
    _xsdWriter.WriteKeyrefClosingTag();
}


void PdbMlSchema::_FilterKeys(vector<vector<string> >& parComboKeys,
  vector<vector<vector<vector<string> > > >& allChildrenKeys,
  const string& catName)
{
    //const vector<string>& catKeys = _dataInfo.GetCatKeys(catName);

    // Get all combo keys participating in parent-child relationships.
    const vector<vector<string> >& origParComboKeys =
     _parentChild.GetComboKeys(catName);

    for (unsigned int keyI = 0; keyI < origParComboKeys.size(); ++keyI)
    {
        const vector<string>& currOrigParComboKey = origParComboKeys[keyI];

        set<unsigned int> nonMandInd;

        _FindToSkipParentItemsIndices(nonMandInd, currOrigParComboKey);

#ifdef VLAD_KEYS_ONLY_REFERENCES
        // If this code is eliminated, the reduced parent key may not be
        // unique any more and XSD unique validation will fail. This code
        // can only be safely removed, once all the parent keys are supersets
        // of category key.
        if (!nonMandInd.empty())
            continue;
#endif

#ifdef VLAD_KEYS_WITH_MANDATORY_DET_REFERENCES
        // If this code is eliminated, the reduced parent key may not be
        // unique any more and XSD unique validation will fail. This code
        // can only be safely removed, once all the parent keys are supersets
        // of category key.
        if (!nonMandInd.empty())
            continue;
#endif

#ifdef VLAD_NO_KEYS_WITH_MANDATORY_DET_REFERENCES
        // If this code is eliminated, the reduced parent key may not be
        // unique any more and XSD unique validation will fail. This code
        // can only be safely removed, once all the parent keys are supersets
        // of category key.
        if (!nonMandInd.empty())
            continue;
#endif

#ifdef VLAD_NO_KEYS_NO_MANDATORY_DET_REFERENCES
        // If this code is eliminated, the reduced parent key may not be
        // unique any more and XSD unique validation will fail. This code
        // can only be safely removed, once all the parent keys are supersets
        // of category key.
        if (!nonMandInd.empty())
            continue;
#endif

        vector<vector<vector<string> > >& origChildrenKeys =
          _parentChild.GetChildrenKeys(currOrigParComboKey);

        // Pair of child index, child key index is a map key. Set of
        // to be skipped indices is map value.
        map<pair<unsigned int, unsigned int>, set<unsigned int> > chSkipInd;

        for (unsigned int childI = 0; childI < origChildrenKeys.size();
          ++childI)
        {
            for (unsigned int childKeyI = 0; childKeyI <
              origChildrenKeys[childI].size(); ++childKeyI)
            {
                const vector<string>& currChKey =
                  origChildrenKeys[childI][childKeyI];

                set<unsigned int> chKeySkipInd;

                _FindToSkipChildItemsIndices(chKeySkipInd, currChKey);

                // Insert a pair of (childI, childKeyI) and chKeySkipInd
                // in the map
                map<pair<unsigned int, unsigned int>, set<unsigned int> >::value_type valuePairIndex(make_pair(childI, childKeyI), chKeySkipInd);
                chSkipInd.insert(valuePairIndex);
            }
        }

        for (unsigned int parKeyI = 0; parKeyI < currOrigParComboKey.size();
          ++parKeyI)
        {
            unsigned int totalNumKeys = 0;
            unsigned int numIndexInKeys = 0;

            for (unsigned int childI = 0; childI < origChildrenKeys.size();
              ++childI)
            {
                for (unsigned int childKeyI = 0; childKeyI <
                  origChildrenKeys[childI].size(); ++childKeyI)
                {
                    totalNumKeys += origChildrenKeys[childI].size();

                    const set<unsigned int>& chSet =
                      chSkipInd[make_pair(childI, childKeyI)];

                    if (chSet.find(parKeyI) != chSet.end())
                    {
                        numIndexInKeys++;
                    }
                }
            }

            // If counter equals the sum of all keys, it means that that
            // item index is skipped in all keys.
            if (numIndexInKeys == totalNumKeys)
            {
                nonMandInd.insert(parKeyI);

                for (unsigned int childI = 0; childI < origChildrenKeys.size();
                  ++childI)
                {
                    for (unsigned int childKeyI = 0; childKeyI <
                      origChildrenKeys[childI].size(); ++childKeyI)
                    {
                        set<unsigned int>& chSet =
                          chSkipInd[make_pair(childI, childKeyI)];

                        chSet.erase(parKeyI);
                    }
                }
            }
        }

        vector<string> newParComboKey = currOrigParComboKey;
        _RemoveNonMandItems(newParComboKey, nonMandInd);
 
        parComboKeys.push_back(newParComboKey);
 
        vector<vector<vector<string> > > childrenKeys;

        for (unsigned int childI = 0; childI < origChildrenKeys.size();
          ++childI)
        {
            // Add empty vector
            vector<vector<string> > newChKeys;

            for (unsigned int childKeyI = 0; childKeyI <
              origChildrenKeys[childI].size(); ++childKeyI)
            {
                const vector<string>& currChKey =
                  origChildrenKeys[childI][childKeyI];

                vector<string> newChKey = currChKey;
                _RemoveNonMandItems(newChKey, nonMandInd);
 
                const set<unsigned int>& chSet =
                  chSkipInd[make_pair(childI, childKeyI)];

                if (!chSet.empty())
                {
                    continue;
                }

                if (!newChKey.empty())
                {
                    newChKeys.push_back(newChKey);
                }
            }

            if (!newChKeys.empty())
            {
                childrenKeys.push_back(newChKeys);
            }
        } // for (all child categories)

        allChildrenKeys.push_back(childrenKeys);
    } // for (all original parent combo keys)
}


bool PdbMlSchema::_AreAllKeyItems(const vector<string>& itemsNames)
{
    if (itemsNames.empty())
    {
        return (false);
    }

    string catName;
    CifString::GetCategoryFromCifItem(catName, itemsNames[0]);

    const vector<string>& catKeys = _dataInfo.GetCatKeys(catName);

    if (itemsNames.size() != catKeys.size())
    {
        return (false);
    }

    unsigned int keyItemsNum = 0;

    for (unsigned int i = 0; i < itemsNames.size(); ++i)
    {
        string attribName;
        CifString::GetItemFromCifItem(attribName, itemsNames[i]);
        if (_dataInfo.IsKeyItem(catName, attribName))
        {
            keyItemsNum++;
        }
    }

    if (keyItemsNum == catKeys.size())
    {
        return (true);
    }

    return (false);
}


bool PdbMlSchema::_AreSupersetOfAllKeyItems(const vector<string>& itemsNames)
{
    if (itemsNames.empty())
    {
        return (false);
    }

    string catName;
    CifString::GetCategoryFromCifItem(catName, itemsNames[0]);

    unsigned int keyItemsNum = 0;

    for (unsigned int i = 0; i < itemsNames.size(); ++i)
    {
        string attribName;
        CifString::GetItemFromCifItem(attribName, itemsNames[i]);
        if (_dataInfo.IsKeyItem(catName, attribName))
        {
            keyItemsNum++;
        }
    }

    const vector<string>& catKeys = _dataInfo.GetCatKeys(catName);

    if (keyItemsNum == catKeys.size())
    {
        return (true);
    }

    return (false);
}


void PdbMlSchema::_FindNonKeyItemsIndices(set<unsigned int>& nonMandIndices,
  const vector<string>& itemsNames)
{
    for (unsigned int indI = 0; indI < itemsNames.size(); ++indI)
    {
        string attribName;
        CifString::GetItemFromCifItem(attribName, itemsNames[indI]);

        string catName;
        CifString::GetCategoryFromCifItem(catName, itemsNames[indI]);

        if (!_dataInfo.IsKeyItem(catName, attribName))
        {
            nonMandIndices.insert(indI);
        }
    }
}


void PdbMlSchema::_FindNonMandItemsIndices(set<unsigned int>& nonMandIndices,
  const vector<string>& itemsNames)
{
    for (unsigned int indI = 0; indI < itemsNames.size(); ++indI)
    {
        if (!_dataInfo.IsItemMandatory(itemsNames[indI]))
        {
            nonMandIndices.insert(indI);
        }
    }
}


void PdbMlSchema::_FindToSkipParentItemsIndices(set<unsigned int>& indices,
  const vector<string>& itemsNames)
{
    for (unsigned int indI = 0; indI < itemsNames.size(); ++indI)
    {
        if (_IsSkipParentItem(itemsNames[indI]))
        {
            indices.insert(indI);
        }
    }
}


void PdbMlSchema::_FindToSkipChildItemsIndices(set<unsigned int>& indices,
  const vector<string>& itemsNames)
{
    for (unsigned int indI = 0; indI < itemsNames.size(); ++indI)
    {
        if (_IsSkipChildItem(itemsNames[indI]))
        {
            indices.insert(indI);
        }
    }
}


void PdbMlSchema::_RemoveNonMandItems(vector<string>& itemsNames,
  set<unsigned int>& nonMandIndices)
{
    for (set<unsigned int>::const_reverse_iterator it =
      nonMandIndices.rbegin(); it != nonMandIndices.rend(); ++it)
    {
        itemsNames.erase(itemsNames.begin() + (*it));
    }
}


bool PdbMlSchema::_IsSkipParentItem(const string& itemName)
{
#ifdef VLAD_KEYS_ONLY_REFERENCES
    string attribName;
    CifString::GetItemFromCifItem(attribName, itemName);

    string catName;
    CifString::GetCategoryFromCifItem(catName, itemName);

    if (!_dataInfo.IsKeyItem(catName, attribName))
    {
        return (true);
    }

    return (false);
#endif

#ifdef VLAD_KEYS_WITH_MANDATORY_DET_REFERENCES
    string attribName;
    CifString::GetItemFromCifItem(attribName, itemName);

    string catName;
    CifString::GetCategoryFromCifItem(catName, itemName);

    if (!_dataInfo.IsKeyItem(catName, attribName))
    {
        if (!_dataInfo.IsItemMandatory(itemName))
        {
            return (true);
        }

        if (CifExcept::CanBeInapplicable(itemName))
        {
            return (true);
        }
    }

    return (false);
#endif

#ifdef VLAD_NO_KEYS_WITH_MANDATORY_DET_REFERENCES
    string attribName;
    CifString::GetItemFromCifItem(attribName, itemName);

    string catName;
    CifString::GetCategoryFromCifItem(catName, itemName);

    if (!_dataInfo.IsItemMandatory(itemName))
    {
        return (true);
    }

    if (CifExcept::CanBeInapplicable(itemName))
    {
        return (true);
    }

    //if (CifExcept::IsBadParentRelation(itemName))
    //{
    //    return (true);
    //}

    return (false);
#endif

#ifdef VLAD_NO_KEYS_NO_MANDATORY_DET_REFERENCES
    string attribName;
    CifString::GetItemFromCifItem(attribName, itemName);

    string catName;
    CifString::GetCategoryFromCifItem(catName, itemName);

    if (CifExcept::CanBeInapplicable(itemName))
    {
        return (true);
    }

    //if (CifExcept::IsBadParentRelation(itemName))
    //{
    //    return (true);
    //}

    return (false);
#endif

    // VLAD - INVESTIGATE THIS
    //if (itemName == "_pdbx_poly_seq_scheme.auth_seq_num")
    //    return (true);
    //else if (itemName == "_entity_poly_seq.num")
    //    return (true);
    //else if (itemName == "_pdbx_poly_seq_scheme.seq_id")
    //    return (true);
    //else if (itemName == "_atom_site.auth_comp_id")
    //    return (true);
    //else if (itemName == "_atom_site.pdbx_PDB_ins_code")
    //    return (true);
    //else if (itemName == "_pdbx_poly_seq_scheme.seq_id")
    //    return (true);
    //else if (itemName == "_pdbx_poly_seq_scheme.pdb_ins_code")
    //    return (true);
    //else if (itemName == "_atom_site.auth_seq_id")
    //    return (true);
    //else if (itemName == "_pdbx_poly_seq_scheme.mon_id")
    //    return (true);
    //else if (itemName == "_atom_site.label_seq_id")
    //    return (true);
    //else if (itemName == "_struct_biol.id")
    //    return (true);
}


bool PdbMlSchema::_IsSkipChildItem(const string& itemName)
{
#ifdef VLAD_KEYS_ONLY_REFERENCES
    string attribName;
    CifString::GetItemFromCifItem(attribName, itemName);

    string catName;
    CifString::GetCategoryFromCifItem(catName, itemName);

    if (!_dataInfo.IsKeyItem(catName, attribName))
    {
        return (true);
    }

    return (false);
#endif

#ifdef VLAD_KEYS_WITH_MANDATORY_DET_REFERENCES
    string attribName;
    CifString::GetItemFromCifItem(attribName, itemName);

    string catName;
    CifString::GetCategoryFromCifItem(catName, itemName);

    if (!_dataInfo.IsKeyItem(catName, attribName))
    {
        if (!_dataInfo.IsItemMandatory(itemName))
        {
            return (true);
        }

        if (CifExcept::CanBeInapplicable(itemName))
        {
            return (true);
        }
    }

    return (false);
#endif

#ifdef VLAD_NO_KEYS_WITH_MANDATORY_DET_REFERENCES
    string attribName;
    CifString::GetItemFromCifItem(attribName, itemName);

    string catName;
    CifString::GetCategoryFromCifItem(catName, itemName);

    if (!_dataInfo.IsItemMandatory(itemName))
    {
        return (true);
    }

    if (CifExcept::CanBeInapplicable(itemName))
    {
        return (true);
    }

    if (CifExcept::IsBadChildRelation(itemName))
    {
        return (true);
    }

    return (false);
#endif

#ifdef VLAD_NO_KEYS_NO_MANDATORY_DET_REFERENCES
    string attribName;
    CifString::GetItemFromCifItem(attribName, itemName);

    string catName;
    CifString::GetCategoryFromCifItem(catName, itemName);

    if (CifExcept::CanBeInapplicable(itemName))
    {
        return (true);
    }

    if (CifExcept::IsBadChildRelation(itemName))
    {
        return (true);
    }

    return (false);
#endif


    // VLAD - DOCUMENT: These items are child combo keys and can have
    // values of "?"

    //if (itemName == "_pdbx_refine_tls_group.beg_label_asym_id")
    //    return (true);
    //else if (itemName == "_software.citation_id")
    //    return (true);
}


void PdbMlSchema::_AggregateInclusiveRanges(const vector<string>& rangeMin,
  const vector<string>& rangeMax, vector<string>& incRangeMin,
  vector<string>& incRangeMax)
{
    vector<string> boundaries;

    vector<string> lowerRanges;
    vector<string> higherRanges;

    // Process ranges that are single values
    for (unsigned int rangeI = 0; rangeI < rangeMin.size(); ++rangeI)
    {
        if (String::IsCiEqual(rangeMin[rangeI], rangeMax[rangeI]))
        {
            boundaries.push_back(rangeMin[rangeI]);
        }
    }

    // Extend input ranges to include single values
    for (unsigned int rangeI = 0; rangeI < rangeMin.size(); ++rangeI)
    {
        bool foundLower = false;
        for (unsigned int boundI = 0; boundI < boundaries.size(); ++boundI)
        {
            if (String::IsCiEqual(rangeMin[rangeI], boundaries[boundI]))
            {
                foundLower = true;
                lowerRanges.push_back(boundaries[boundI]);
                break;
            }
        }

        if (!foundLower)
        {
            lowerRanges.push_back(string());
        }

        bool foundHigher = false;
        for (unsigned int boundI = 0; boundI < boundaries.size(); ++boundI)
        {
            if (String::IsCiEqual(rangeMax[rangeI], boundaries[boundI]))
            {
                higherRanges.push_back(boundaries[boundI]);
                foundHigher = true;
                break;
            }

        }

        if (!foundHigher)
        {
            higherRanges.push_back(string());
        }
    }

    for (unsigned int rangeI = 0; rangeI < rangeMin.size(); ++rangeI)
    {
        if (String::IsCiEqual(rangeMin[rangeI], rangeMax[rangeI]))
        {
            continue;
        }

        if (!lowerRanges[rangeI].empty())
        {
            incRangeMin.push_back(lowerRanges[rangeI]);
        }
        else
        {
            incRangeMin.push_back(rangeMin[rangeI]);
        }

        if (!higherRanges[rangeI].empty())
        {
            incRangeMax.push_back(higherRanges[rangeI]);
        }
        else
        {
            incRangeMax.push_back(rangeMax[rangeI]);
        }
    }
}


bool PdbMlSchema::_HasMultipleSubRanges(const string& itemName)
{
  const vector<string>& rangeMin = _dataInfo.GetItemAttribute(itemName,
    CifString::CIF_DDL_CATEGORY_ITEM_RANGE,
    CifString::CIF_DDL_ITEM_MINIMUM);

  const vector<string>& rangeMax = _dataInfo.GetItemAttribute(itemName,
    CifString::CIF_DDL_CATEGORY_ITEM_RANGE,
    CifString::CIF_DDL_ITEM_MAXIMUM);

    vector<string> incRangeMin;
    vector<string> incRangeMax;

    _AggregateInclusiveRanges(rangeMin, rangeMax, incRangeMin, incRangeMax);

    return ((incRangeMin.size() != 0) && (incRangeMin.size() != 1));
}

