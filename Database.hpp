//
//  Database.hpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#ifndef Database_hpp
#define Database_hpp

#include <stdio.h>
#include <fstream>
#include "Storage.hpp"
#include "Row.hpp"
#include "Query.hpp"
#include "Entity.hpp"

namespace ECE141
{
  class Database : public Storable
  {
  public:
    Database(const std::string aPath, CreateDB, bool useCache = false);
    Database(const std::string aPath, OpenDB, bool useCache = false);
    virtual ~Database();

    //storable...
    StatusResult encode(std::ostream &anOutput) override;
    StatusResult decode(std::istream &anInput) override;

    bool is(const std::string aName) { return name == aName; }
    StatusResult addEntity(Entity &anEntity);
    StatusResult updateEntity(Entity &anEntity);
    StatusResult dropTable(const std::string &aName, int &rownum);
    bool insertRows(const RowCollection &aRows);
    bool selectRows(const std::string &aName, RowCollection &aRows);
    int deleteRows(const Query &aQuery);
    int updateRows(Query &aQuery);

    bool hasEntity(const std::string &aName);
    Entity *getEntity(const std::string &aName);
    Storage &getStorage() { return storage; }
    std::map<std::string, uint32_t> &getIndexIndex() { return indexindex; }
    LFUCache<std::string, std::string> *getViewCache() { return viewCache; }

  protected:
    Storage storage;
    std::string name;
    bool changed;
    std::fstream stream; //stream storage uses for IO
    std::map<std::string, uint32_t> entityindex;
    std::map<std::string, uint32_t> indexindex;
    LFUCache<std::string, std::vector<Row>> *rowCache;
    LFUCache<std::string, std::string> *viewCache;
  };

}
#endif /* Database_hpp */
