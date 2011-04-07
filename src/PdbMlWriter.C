//$$FILE$$
//$$VERSION$$
//$$DATE$$
//$$LICENSE$$


#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "rcsb_types.h"

#include "GenCont.h"
#include "CifString.h"
#include "CifExcept.h"
#include "PdbMlSchema.h"
#include "PdbMlWriter.h"


using std::exception;
using std::string;
using std::vector;
using std::map;
using std::sort;
using std::cerr;
using std::endl;
using std::setw;
using std::ostream;


PdbMlWriter::PdbMlWriter(ostream& io, const string& ns,
  DataInfo& dataInfo) : XmlWriter(io, ns), _dataInfo(dataInfo)
{

}


PdbMlWriter::~PdbMlWriter()
{

}

void PdbMlWriter::WriteDatablockOpeningTag()
{
    WriteOpeningTag(PdbMlSchema::DATABLOCK_ELEMENT);
}

void PdbMlWriter::WriteDatablockClosingTag()
{
    WriteClosingTag(PdbMlSchema::DATABLOCK_ELEMENT);
}


void PdbMlWriter::WriteDatablockAttribute(const string& value)
{
    WriteAttribute(PdbMlSchema::DATABLOCK_NAME_ATTRIBUTE, value,
      value.size());
}


void PdbMlWriter::WriteCategoryOpeningTag(const string& catName)
{
    string catElemName;
    PdbMlSchema::MakeCategoryElementName(catElemName, catName);

    WriteQualifiedOpeningTag(catElemName, true);
}


void PdbMlWriter::WriteCategoryClosingTag(const string& catName)
{
    string catElemName;
    PdbMlSchema::MakeCategoryElementName(catElemName, catName);

    WriteQualifiedClosingTag(catElemName);
}


void PdbMlWriter::WriteTable(ISTable* tIn, vector<unsigned int>& widths,
  const bool reCalcWidth, const vector<eTypeCode>& typeCodes)
{
    if (!_io || !tIn)
        return;

    if (reCalcWidth && widths.empty())
        // VLAD - Throw some exception here
        return;

    const string& tableName = tIn->GetName();
    const vector<string>& allColumnNames = tIn->GetColumnNames();

    vector<unsigned int> allColumnIndices;
    vector<string> columnNames;
    map<string, unsigned int> columnIndicesMap;
    map<string, eTypeCode> typeCodesMap;

    for (unsigned int i = 0; i < allColumnNames.size(); ++i)
    {
        allColumnIndices.push_back(i);

        string cifItem;
        CifString::MakeCifItem(cifItem, tableName, allColumnNames[i]);

        if (_dataInfo.IsItemDefined(cifItem))
        {
            columnNames.push_back(allColumnNames[i]);

            map<string, unsigned int>::value_type
              valuePairIndex(allColumnNames[i], i);
            columnIndicesMap.insert(valuePairIndex);

            if (!typeCodes.empty())
            {
                map<string, eTypeCode>::value_type
                  valuePairType(allColumnNames[i], typeCodes[i]);
                typeCodesMap.insert(valuePairType);
            }
        }
        else
        {
            cerr << "Skipping conversion to XML of non-defined item \"" <<
              cifItem << "\"" << endl;
        }
    }

    if (columnNames.empty())
    {
        cerr << endl;
        cerr << "Warning: Skipping conversion to XML of table \"" <<
          tableName << "\", since no items are specified." << endl;
        cerr << endl;

        return;
    }

    sort(columnNames.begin(), columnNames.end());

    vector<unsigned int> columnIndices;
    for (unsigned int i = 0; i < columnNames.size(); ++i)
    {
        columnIndices.push_back(columnIndicesMap[columnNames[i]]);
    }

    vector<bool> keyColumns;
    for (unsigned int j = 0; j < columnNames.size(); ++j)
    {
        keyColumns.push_back(_dataInfo.IsKeyItem(tableName,
          columnNames[j], tIn->GetColCaseSense()));
    }

    bool haveKeys = false;
    for (unsigned int keyI = 0; keyI < keyColumns.size(); ++keyI)
    {
        if (keyColumns[keyI])
        {
            haveKeys = true;
            break;
        }
    }

    if (!haveKeys)
    {
        cerr << "Warning: Skipping conversion to XML of table \"" <<
          tableName << "\", since no keys values are specified." << endl;

        return;
    }

    vector<bool> allowedNullColumns;
    for (unsigned int j = 0; j < columnNames.size(); ++j)
    {
        allowedNullColumns.push_back(_dataInfo.IsUnknownValueAllowed(tableName,
          columnNames[j]));
    }

    vector<eTypeCode> usedItemsTypes;
    if (typeCodes.empty())
    {
        _dataInfo.GetItemsTypes(usedItemsTypes, tableName, columnNames);
    }
    else
    {
        for (unsigned int i = 0; i < columnNames.size(); ++i)
        {
            usedItemsTypes.push_back(typeCodesMap[columnNames[i]]);
        }
    }

    bool isAllKey = _dataInfo.AreAllKeyItems(tableName, columnNames);

    bool openCategoryTagSet = false;

    for (unsigned int i = 0; i < tIn->GetNumRows(); ++i)
    {
        vector<string> row = tIn->GetRow(i);

        if (!_dataInfo.AreItemsValuesValid(tableName, columnNames,
          columnIndices, allowedNullColumns, row))
        {
#ifndef VLAD_ATOM_SITES_ALT_ID_IGNORE
            if (CIF_ITEM != "_atom_sites_alt.id")
#endif
            {
                cerr << "Warning: Skipping conversion to XML of row # " <<
                  i + 1 << " in table \"" << tableName << "\"." << endl;
            }
#ifndef VLAD_ATOM_SITES_ALT_ID_IGNORE
            else
            {
                CIF_ITEM.clear();
            }
#endif
            continue;
        }

        if (!openCategoryTagSet)
        {
            openCategoryTagSet = true;

            Indent();
            WriteCategoryOpeningTag(tableName);

            IncrementIndent();
        } 

        Indent();
        WriteQualifiedOpeningTag(tableName);
 
        for (unsigned int j = 0; j < columnNames.size(); ++j)
        {
            if (usedItemsTypes[j] == eTYPE_CODE_NONE)
            {
                // Skipping the item that is not defined
                continue;
            }

            if (!keyColumns[j])
            {
                continue;
            }

            _dataInfo.StandardizeEnumItem(row[columnIndices[j]], tableName,
              columnNames[j]);

            if (reCalcWidth)
            {
                if (row[columnIndices[j]].size() > widths[columnIndices[j]])
                    widths[columnIndices[j]] = row[columnIndices[j]].size();
            }

            unsigned int width = row[columnIndices[j]].size();

            if (!widths.empty())
                width = widths[columnIndices[j]];

            try
            {
                WriteAttributeValue(columnNames[j], row[columnIndices[j]],
                  usedItemsTypes[j], width);
            }
            catch (const exception& exc)
            {
                cerr << "Warning - In category \"" << tableName <<
                  "\" and attribute \"" << columnNames[j] <<
                  "\", value \"" << row[columnIndices[j]] << "\" could"\
                  " not be formatted." << endl;
            }
        }

        WriteClosingBracket(true);

        if (!isAllKey)
            _io << endl;

        IncrementIndent();

        for (unsigned int j = 0; j < columnNames.size(); ++j)
        {
            if (usedItemsTypes[j] == eTYPE_CODE_NONE)
            {
                // Skipping the item that is not defined
                continue;
            }

            if (keyColumns[j])
            {
                continue;
            }

            if ((row[columnIndices[j]] == CifString::UnknownValue)
              || ((row[columnIndices[j]]).empty()))
            {
                // Skip unknown or empty values.
                continue;
            }

#ifdef VLAD_DONT_IGNORE_MANDATORY
            if (row[columnIndices[j]] == CifString::UnknownValue)
            {
                string itemName;

                CifString::MakeCifItem(itemName, tableName, columnNames[j]);

                if (CifExcept::CanBeUnknown(itemName))
                    continue;
            }
#endif

            Indent();
            WriteQualifiedOpeningTag(columnNames[j]);

            if (row[columnIndices[j]] == CifString::InapplicableValue)
            {
                WriteNilAttribute("true");
                WriteClosingTag();
                continue;
            }

            WriteClosingBracket(true);

            _dataInfo.StandardizeEnumItem(row[columnIndices[j]], tableName,
              columnNames[j]);

            if (reCalcWidth)
            {
                if (row[columnIndices[j]].size() > widths[columnIndices[j]])
                    widths[columnIndices[j]] = row[columnIndices[j]].size();
            }

            unsigned int width = row[columnIndices[j]].size();

            if (!widths.empty())
                width = widths[columnIndices[j]];

            if (row[columnIndices[j]] != CifString::InapplicableValue)
            {
                try
                {
                    _FormatData(row[columnIndices[j]], usedItemsTypes[j],
                      width);
                }
                catch (const exception& exc)
                {
                    cerr << "Warning - In category \"" << tableName <<
                      "\" and attribute \"" << columnNames[j] <<
                      "\", value \"" << row[columnIndices[j]] << "\" could"\
                      " not be formatted." << endl;
                }
            }

            WriteQualifiedClosingTag(columnNames[j]);
        }

        DecrementIndent();
      
        if (!isAllKey)
            Indent();

        WriteQualifiedClosingTag(tableName);
    }

    if (openCategoryTagSet)
    {
        DecrementIndent();

        Indent();
        WriteCategoryClosingTag(tableName);
    }
    else
    {
        cerr << endl;
        cerr << "Warning: Skipping conversion to XML of table \"" <<
          tableName << "\"." << endl;
        cerr << endl;
    }
}


void PdbMlWriter::_writeAlternateAtomSiteTable(ISTable* tIn)
{
    if (!_io || !tIn)
        return;

    int inW[20];
    string inC[20];

    inC[0]  = "group_PDB";
    inC[1]  = "pdbx_PDB_model_num";
    inC[2]  = "label_asym_id";
    inC[3]  = "auth_asym_id";
    inC[4]  = "label_seq_id";
    inC[5]  = "auth_seq_id";
    inC[6]  = "pdbx_PDB_ins_code";
    inC[7]  = "label_alt_id";
    inC[8]  = "label_comp_id";
    inC[9]  = "auth_comp_id";
    inC[10] = "type_symbol";
    inC[11] = "label_atom_id";
    inC[12] = "auth_atom_id";
    inC[13] = "Cartn_x";
    inC[14] = "Cartn_y";
    inC[15] = "Cartn_z";
    inC[16] = "occupancy";
    inC[17] = "B_iso_or_equiv";
    inC[18] = "label_entity_id";
    inC[19] = "pdbx_formal_charge";

    for (unsigned int inCind = 0; inCind < 20; ++inCind)
    {
        if (!tIn->IsColumnPresent(inC[inCind]))
          inC[inCind].clear();
    }

    inW[0] = 6;
    inW[1] = 4;
    inW[2] = 3;
    inW[3] = 4;
    inW[4] = 6;
    inW[5] = 6;
    inW[6] = 2;
    inW[7] = 2;
    inW[8] = 4;
    inW[9] = 4;
    inW[10] = 3;
    inW[11] = 6;
    inW[12] = 6;
    inW[13] = 9;
    inW[14] = 9;
    inW[15]=  9;
    inW[16] = 7;
    inW[17] = 7;
    inW[18] = 5;
    inW[19] = 4;

    Indent();
    WriteOpeningTag("category_atom_record", true);

    IncrementIndent();
 
    for (unsigned int i = 0; i < tIn->GetNumRows(); ++i)
    {
        Indent();

        WriteOpeningTag("atom_record");

        string idC = "\"" + (*tIn)(i, "id") + "\"";

        _io << " id=" << setw(9) << idC;

        WriteClosingBracket(true);

        for (unsigned int j = 0; j < 20; ++j)
        {
            if (inC[j].empty())
                continue;

            const string& cell = (*tIn)(i, inC[j]);

            if (cell.empty())
            {
                _io << CifString::UnknownValue;
            }
            else
            {
                if ((inC[j] == "label_atom_id") || (inC[j] == "auth_atom_id"))
                {
                    _io << " "; 
 
                    for (unsigned int k = 0; k < cell.size(); ++k)
                    {
                        if (cell[k] == ' ')
                        {
                            _io << "&#32;"; 
                        }
                        else
                        {
                            _io << cell[k]; 
                        }
                    }
                }
                else
                {
                    _io << setw(inW[j]) << cell;
                }

                _io << " ";
            }
        }

        WriteClosingTag("atom_record");
    }

    DecrementIndent();

    Indent();
    WriteClosingTag("category_atom_record");
}

