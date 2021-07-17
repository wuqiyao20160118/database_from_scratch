//
//  Query.cpp
//  Assignment5
//
//  Created by rick gessner on 4/26/21.
//

#include "Query.hpp"
namespace ECE141 {

  struct RowComparator {
    explicit RowComparator(std::vector<std::pair<const std::string, bool>> theOrders) : orderfields(theOrders) {}

    bool operator()(std::unique_ptr<Row> &a, std::unique_ptr<Row> &b) const {
      for (auto theOrder : orderfields) {
        std::string orderName = theOrder.first;
        bool ascending = theOrder.second;
        if (a->getData()[orderName] != b->getData()[orderName]) {
          return ascending ? (a->getData()[orderName] < b->getData()[orderName]) : (a->getData()[orderName] > b->getData()[orderName]);
        }
      }

            // if rows equals at all fields
      return true;
    }

    std::vector<std::pair<const std::string, bool>> orderfields;
  };

  StatusResult orderRows(RowCollection &rowCollection, std::vector<std::pair<const std::string, bool>> theOrders) {
    std::sort(rowCollection.begin(), rowCollection.end(), RowComparator(theOrders));
    return StatusResult{noError};
  }

  //implement your query class here...
  Query& Query::setSelectAll(bool aState) {
    all = aState; 
    return *this; 
  }

  Query& Query::setSelect(const std::vector<std::string>& aStrings) { 
    selects = aStrings;
    return *this;
  }

  Query& Query::setFrom(Entity *anEntity){ 
    _from = anEntity; 
    return *this; 
  }

  Query& Query::setOffset(int anOffset) { 
    offset = anOffset; 
    return *this; 
  }

  Query& Query::setLimit(int aLimit) { 
    limit = aLimit; 
    return *this; 
  }

  Query& Query::setUpdate(const std::vector<std::string> &keys, const std::vector<Value> &values) {
    updatekeys = keys;
    updatevalues = values;
    return *this;
  }

  Query& Query::orderBy(const std::string &aField, bool ascending) { 
    orders.push_back(std::make_pair(aField, ascending)); 
    return *this; 
  }


  Query &Query::groupBy(const std::vector<std::string> &aFields) {
    for (auto theField : aFields) {
      groups.push_back(theField);
    }
    return *this;
  }

  Query &Query::run(RowCollection &aRows) {
    // filtering, we need to execute "where" first
    if (theFilter.getCount() > 0) {
      for (size_t i = 0; i < aRows.size(); i++) {
        if (!theFilter.matches(*aRows[i])) {
          aRows.erase(aRows.begin() + i);
          i--; // need to decrement 1 because indexes after erasion changes
        }
      }
    }

    // order by
    if (orders.size() > 0) {
      StatusResult result = orderRows(aRows, orders);
      if (result != StatusResult{noError})
        throw "order in query::run() error.";
    }

    // offset and limit

    // consider some corner cases
    if (offset >= aRows.size()) {
      aRows.erase(aRows.begin(), aRows.end());
      return *this;
    }
    if (offset + limit < aRows.size() && limit > 0) {
      aRows.erase(aRows.begin() + offset + limit, aRows.end());
    }
    if (offset > 0) {
      aRows.erase(aRows.begin(), aRows.begin() + offset);
    }

    // may need more operations...

    return *this;
  }

  StatusResult Query::parsewhere(Tokenizer &aTokenizer) {
    return theFilter.parse(aTokenizer, *_from);
  }

  bool Query::matches(Row& aRow) const {
    return theFilter.matches(aRow);
  }

    //------------------------------------------------------------------------
    // handles

  bool Query::handleLimit(Tokenizer &aTokenizer) {
    if (2 < aTokenizer.remaining()) {
      aTokenizer.next();
      int theValue = std::stoi(aTokenizer.current().data);
      setLimit(theValue);
      aTokenizer.next();
      return true;
    }
    return false;
  }

  bool Query::handleOffset(Tokenizer &aTokenizer) {
    if (2 < aTokenizer.remaining()) {
      aTokenizer.next();
      int theValue = std::stoi(aTokenizer.current().data);
      setOffset(theValue);
      aTokenizer.next();
      return true;
    }
    return false;
  }

  bool Query::handleOrderBy(Tokenizer &aTokenizer) {
    if (2 < aTokenizer.remaining()) {
      aTokenizer.next(2);
      // iterate until next keyword
      while (1 < aTokenizer.remaining() && aTokenizer.current().type != TokenType::keyword) {
        std::string theName = aTokenizer.current().data;
        aTokenizer.next();
        Keywords cur_keyword = aTokenizer.current().keyword;

        // check whether has keyword asc/desc
        if (!theOrders.count(cur_keyword)) {
          orderBy(theName, true);
        }
        else {
          bool theParse = (this->*theOrders[cur_keyword])(aTokenizer, theName);
          if (!theParse)
            return false;
        }
        aTokenizer.skipIf(',');
      }
      return true;
    }
    return false;
  }

  bool Query::handleGroupBy(Tokenizer &aTokenizer) {
    if (2 < aTokenizer.remaining()) {
      aTokenizer.next(2);
      // iterate until next keyword
      std::vector<std::string> groupNames;
      while (aTokenizer.remaining() > 1 && aTokenizer.current().type != TokenType::keyword) {
        std::string theName = aTokenizer.current().data;
        groupNames.push_back(theName);
        aTokenizer.next();
        aTokenizer.skipIf(',');
      }
      groupBy(groupNames);
      return true;
    }
    return false;
  }

  bool Query::handleWhere(Tokenizer &aTokenizer) {
    if (4 < aTokenizer.remaining()) {
      aTokenizer.next();
      parsewhere(aTokenizer);
      return true;
    }
    return false;
  }

  bool Query::handleDesc(Tokenizer &aTokenizer, std::string &aName) {
    if (1 < aTokenizer.remaining()) {
      orderBy(aName, false);
      aTokenizer.next();
      return true;
    }
    return false;
  }

  bool Query::handleAsc(Tokenizer &aTokenizer, std::string &aName) {
    if (1 < aTokenizer.remaining()) {
      orderBy(aName, true);
      aTokenizer.next();
      return true;
    }
    return false;
  }

}

