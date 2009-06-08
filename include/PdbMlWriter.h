//$$FILE$$
//$$VERSION$$
//$$DATE$$
//$$LICENSE$$


/**
** \file PdbMlWriter.h
**
** PDBML Writer.
*/


#ifndef PDBMLWRITER_H
#define PDBMLWRITER_H


#include <string>
#include <ostream>

#include "ISTable.h"
#include "XmlWriter.h"
#include "DataInfo.h"


class PdbMlWriter : public XmlWriter
{
  public:
    PdbMlWriter(std::ostream& io, const std::string& ns, DataInfo& dataInfo);

    ~PdbMlWriter();

    // PDBML related API
    void WriteTable(ISTable* tIn,
      vector<unsigned int>& widths,
      const bool reCalcWidth = false,
      const vector<eTypeCode>& typeCodes = vector<eTypeCode>());

    void _writeAlternateAtomSiteTable(ISTable *tIn);

    void WriteDatablockOpeningTag();
    void WriteDatablockClosingTag();

    void WriteDatablockAttribute(const std::string& value);

    void WriteCategoryOpeningTag(const std::string& catName);
    void WriteCategoryClosingTag(const std::string& catName);

  private:
    static std::string DATABLOCK_TAG;
    DataInfo& _dataInfo;
};


#endif
