//
//  Row.cpp
//  Assignment4
//
//  Created by rick gessner on 4/19/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#include "Row.hpp"
#include "Database.hpp"
#include <sstream>

namespace ECE141
{

  Row::Row(Entity &entity, std::vector<std::string> &keys, std::vector<Value> &values) : blockNumber(0), tableName(entity.name)
  {
    tableName = entity.name;
    bool hasAutoInrement = false;
    for (int i = 0; i < keys.size(); i++)
    {
      data[keys[i]] = values[i];
    }
    for (int i = 0; i < entity.attributes.size(); i++)
    {
      if (entity.attributes[i].auto_increment)
      {
        hasAutoInrement = true;
        if (std::find(keys.begin(), keys.end(), entity.attributes[i].field_name) == keys.end())
          data[entity.attributes[i].field_name] = entity.getid();
        continue;
      }
      if (entity.attributes[i].hasdefault)
      {
        if (data.count(entity.attributes[i].field_name) == 0)
        {
          data[entity.attributes[i].field_name] = entity.attributes[i].defaultvalue;
        }
      }
    }
    if (hasAutoInrement)
      entity.incrementid();
  }

  //STUDENT: You need to fully implement the ROW methods...

  std::ostream &operator<<(std::ostream &out, const Value &aValue)
  {
    static char vtypes[] = {'b', 'i', 'd', 's'};
    char theType = vtypes[aValue.index()];
    std::visit([&theType, &out](auto const &var)
               {
                 std::stringstream ss;
                 ss << var;
                 std::string thevar = ss.str();
                 std::replace(thevar.begin(), thevar.end(), ' ', '~');
                 out << theType << ' ' << thevar;
               },
               aValue);
    return out;
  }

  StatusResult Row::encode(std::ostream &aWriter)
  {
    // block number      key value pair size      key1      value1      key2      value2 ...
    aWriter << blockNumber << " ";
    aWriter << data.size() << " ";
    for (auto &thePair : data)
    {
      aWriter << thePair.first << " " << thePair.second << " ";
    }
    return StatusResult{noError};
  }

  StatusResult Row::decode(std::istream &aReader)
  {
    // block number      key value pair size      key1      value1      key2      value2 ...
    aReader >> blockNumber;
    int mapsize;
    aReader >> mapsize;
    for (int i = 0; i < mapsize; i++)
    {
      std::string key, svalue;
      char type;
      aReader >> key >> type >> svalue;
      Value value;
      switch (type)
      {
      case 'b':
        value = (std::stoi(svalue)) ? true : false;
        break;
      case 'i':
        value = std::stoi(svalue);
        break;
      case 'd':
        value = std::stof(svalue);
        break;
      case 's':
        std::replace(svalue.begin(), svalue.end(), '~', ' ');
        value = svalue;
        break;
      default:
        return StatusResult{unknownType};
      }
      data[key] = value;
    }
    return StatusResult{noError};
  }

  void Row::initBlock(Block &aBlock)
  {
    aBlock.header.refId = Entity::hashString(tableName);
  }

  Row &Row::setKV(std::string &aKey, Value aValue)
  {
    data[aKey] = aValue;
    return *this;
  }

  StatusResult Row::getSelectColumns(std::vector<std::string> &values, std::vector<std::string> theCols)
  {
    for (auto col : theCols)
    {
      if (data.count(col))
      {
        std::string outStr = Helpers::valueToString(data[col]);
        values.push_back(outStr);
      }
      else
      {
        // null value, print blanks
        values.push_back("");
      }
    }
    return StatusResult{noError};
  }

  StatusResult RowPair::getSelectColumns(StringList &values, StringList theCols, StringList theTables)
  {
    if (theCols.size() != theTables.size())
      return StatusResult{syntaxError};
    for (size_t i = 0; i < theCols.size(); i++)
    {
      std::string theCol = theCols[i];
      std::string theTable = theTables[i];
      if (theTable == left.getTableName())
      {
        KeyValues theData = left.getData();
        assemebleValue(theData, values, theCol);
      }
      else
      {
        KeyValues theData = right.getData();
        assemebleValue(theData, values, theCol);
      }
    }
    return StatusResult{noError};
  }

  void RowPair::assemebleValue(KeyValues &theData, StringList &theValue, std::string col)
  {
    if (theData.count(col))
    {
      std::string outStr = Helpers::valueToString(theData[col]);
      theValue.push_back(outStr);
    }
    else
    {
      // null value, print blanks
      theValue.push_back("NULL");
    }
  }

}
