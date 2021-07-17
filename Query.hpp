//
//  Query.hpp
//  Assignment5
//
//  Created by rick gessner on 4/26/21.
//

#ifndef Query_hpp
#define Query_hpp

#include <stdio.h>
#include <string>
#include "Attribute.hpp"
#include "Row.hpp"
#include "Entity.hpp"
#include "Tokenizer.hpp"
#include "Filters.hpp"

namespace ECE141
{

  class Query
  {
  public:
    Query() : _from(nullptr), all(false), offset(0), limit(0), theFilter() {}
    ~Query() { delete _from; }

    bool selectAll() const { return all; }
    Query &setSelectAll(bool aState);
    Query &setSelect(const std::vector<std::string> &aStrings);
    std::vector<std::string> &getSelect() { return selects; }

    Entity *getFrom() const { return _from; }
    Query &setFrom(Entity *anEntity);

    Query &setOffset(int anOffset);
    Query &setLimit(int aLimit);
    Query &setUpdate(const std::vector<std::string> &keys, const std::vector<Value> &values);
    std::vector<std::string> &getUpdatekeys() { return updatekeys; }
    std::vector<Value> &getUpdatevalues() { return updatevalues; }

    Query &orderBy(const std::string &aField, bool ascending = false);
    Query &groupBy(const std::vector<std::string> &aFields);
    Query &run(RowCollection &aRows);

    StatusResult parsewhere(Tokenizer &aTokenizer);
    bool matches(Row &aRow) const;

    bool handleLimit(Tokenizer &aTokenizer);
    bool handleOffset(Tokenizer &aTokenizer);
    bool handleOrderBy(Tokenizer &aTokenizer);
    bool handleGroupBy(Tokenizer &aTokenizer);
    bool handleWhere(Tokenizer &aTokenizer);
    bool handleDesc(Tokenizer &aTokenizer, std::string &aName);
    bool handleAsc(Tokenizer &aTokenizer, std::string &aName);

    std::string commandStr;

  protected:
    Entity *_from;
    bool all;
    int offset;
    int limit;
    std::vector<std::string> updatekeys;
    std::vector<Value> updatevalues;
    std::vector<std::string> selects;
    std::vector<std::string> groups;
    std::vector<std::pair<const std::string, bool>> orders;
    Filters theFilter; // filter for "where"
  };

  using QueryPtr = bool (Query::*)(Tokenizer &aTokenizer);
  using OrderPtr = bool (Query::*)(Tokenizer &aTokenizer, std::string &aName);

  static std::map<Keywords, QueryPtr> theQueries = {
      {Keywords::order_kw, &Query::handleOrderBy},
      {Keywords::offset_kw, &Query::handleOffset},
      {Keywords::limit_kw, &Query::handleLimit},
      {Keywords::where_kw, &Query::handleWhere},
      {Keywords::group_kw, &Query::handleGroupBy}};

  static std::map<Keywords, OrderPtr> theOrders = {
      {Keywords::desc_kw, &Query::handleDesc},
      {Keywords::asc_kw, &Query::handleAsc}};

}

#endif /* Query_hpp */
