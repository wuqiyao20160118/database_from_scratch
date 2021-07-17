//
//  Row.hpp
//  Assignment4
//
//  Created by rick gessner on 4/19/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Row_hpp
#define Row_hpp

#include <stdio.h>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <memory>
#include "Storage.hpp"
#include "Attribute.hpp"
#include "BasicTypes.hpp"
#include "Entity.hpp"
#include "Helpers.hpp"
#include "Filters.hpp"

class Database;

namespace ECE141
{

  class DataProvider
  {
  public:
    virtual Value &getValue(const Operand &anInfo) = 0;
  };

  class Row : public Storable, public DataProvider
  {
  public:
    //STUDENT declare OCF methods...
    Row(std::string atableName) : blockNumber(0), tableName(atableName) {}
    Row(const Row &aRow) : data(aRow.data), blockNumber(aRow.blockNumber), tableName(aRow.tableName) {}
    Row(Entity &entity, std::vector<std::string> &keys, std::vector<Value> &theValues);
    ~Row() {}

    //storable api just in case...
    StatusResult encode(std::ostream &aWriter) override;
    StatusResult decode(std::istream &aReader) override;
    Value &getValue(const Operand &anInfo) override { return data[anInfo.name]; }
    void initBlock(Block &aBlock);

    Row &setKV(std::string &aKey, Value aValue);
    void setblockNumber(uint32_t ablockNum) { blockNumber = ablockNum; }
    uint32_t getblockNumber() { return blockNumber; }
    KeyValues getData() { return data; }
    std::string getTableName() { return tableName; }
    StatusResult getSelectColumns(std::vector<std::string> &values, std::vector<std::string> theCols);

  protected:
    KeyValues data;
    uint32_t blockNumber;
    //do you need any other data members?
    std::string tableName;
  };

  struct RowPair : public DataProvider
  {
    RowPair(Row &aLeftRow, Row &aRightRow) : left(aLeftRow), right(aRightRow) {}
    Value &getValue(const Operand &anInfo) override
    {
      if (Entity::hashString(left.getTableName()) == anInfo.entityId)
        return left.getValue(anInfo);
      return right.getValue(anInfo);
    }

    StatusResult getSelectColumns(StringList &values, StringList theCols, StringList theTables);
    void assemebleValue(KeyValues &theData, StringList &theValue, std::string col);

    Row left;
    Row right;
  };

  //-------------------------------------------

  using RowCollection = std::vector<std::unique_ptr<Row>>;

  using RowPairCollection = std::vector<std::unique_ptr<RowPair>>;

}

#endif /* Row_hpp */
