//$$FILE$$
//$$VERSION$$
//$$DATE$$
//$$LICENSE$$


/**
** \file PdbMlSchema.h
**
** PDB ML schema converter class.
*/


#ifndef PDBMLSCHEMA_H
#define PDBMLSCHEMA_H


#include <string>
#include <vector>
#include <set>

#include "DataInfo.h"
#include "ParentChild.h"
#include "XsdWriter.h"


class PdbMlSchema
{
  public:
    static const std::string DATABLOCK_ELEMENT;
    static const std::string DATABLOCK_NAME_ATTRIBUTE;

    PdbMlSchema(XsdWriter& xsdWriter, ParentChild& parentChild,
      DataInfo& dataInfo, const std::string& ns, const std::string& prefix);

    void Convert();

    static void MakeSchemaFileName(std::string& schemaFileName,
      const std::string& prefix,
      const std::string& dictVer = std::string());
    static void MakeFullSchemaFileName(std::string& schemaFileName,
      const std::string& prefix, const std::string& dictVer = std::string());

    static void MakeCategoryElementName(std::string& catElemName,
      const std::string& catName);
    static void MakeCategoryTypeName(std::string& catTypeName,
      const std::string& catName);

  private:
    static const std::string CATEGORY_ELEMENT_SUFFIX;
    static const std::string CATEGORY_TYPE_SUFFIX;

    static const std::string SCHEMA_LOCATION_DIRECTORY;

    XsdWriter& _xsdWriter;
    ParentChild& _parentChild;
    DataInfo& _dataInfo;
    std::string _ns;
    std::string _nsPrefix;
    std::string _prefix;

    void _WriteSchemaAttributes(const std::string& ns);
    void _WriteSchemaComment(const std::string& dictVer);

    void _WriteCategoriesTypes(const std::vector<std::string>& categories,
      const std::vector<std::string>& items);
    void _WriteDatablockType(const std::vector<std::string>& categories);
    void _WriteDatablockElement(const std::vector<std::string>& categories);

    void _WriteCategoryDocumentation(const std::string& catName);
    void _WriteNonKeyItem(const std::string& itemName);
    void _WriteKeyItem(const std::string& itemName);

    void _WriteCategoryKeys(const std::string& catName);

    void _WriteCategoryKeysAndKeyrefs(const std::string& catName);

    void _WriteComboKey(const std::string& catName,
      const std::vector<std::string>& keyItems, const std::string& append);

    void _WriteKeyRef(const std::string& keyRefName,
      const std::string& keyName, const std::string& xPath,
      const std::vector<std::string>& childKeys,
      const std::string& childCatName);

    void _WriteItemAttributes(const std::string& itemName, const bool isKey);

    void _WriteItemDocumentation(const std::string& itemName); 

    void _WriteDataTypeAsElement(const std::string& itemName);
    void _WriteDataTypeAsAttribute(const std::string& itemName);

    void _FindNonMandItemsIndices(std::set<unsigned int>& nonMandIndices,
      const std::vector<std::string>& itemsNames);

    void _FindToSkipItemsIndices(std::set<unsigned int>& indices,
      const std::vector<std::string>& itemsNames);

    void _RemoveNonMandItems(std::vector<std::string>& itemsNames,
      std::set<unsigned int>& nonMandIndices);

    bool _IsSkipParentItem(const std::string& itemName);
};

#endif

