
//
//  RecordsView.hpp
//
//  Created by rick gessner on 4/26/20.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#ifndef TabularView_h
#define TabularView_h

#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include <vector>
#include <string>
#include "Timer.hpp"
#include "View.hpp"
#include "Row.hpp"
#include "Cache.hpp"

//IGNORE THIS CLASS if you already have a class that does this...

namespace ECE141
{

  // USE: general tabular view (with columns)
  class TabularView : public View
  {
  public:
    TabularView(Timer *aTimer) : timer(aTimer){};
    ~TabularView(){};

    bool show(std::ostream &anOutput);

    StatusResult setcolumns(std::vector<std::string> theCols);
    StatusResult setRowCollection(RowCollection &theRows);
    void setCache(LFUCache<std::string, std::string> *theCache) { viewCache = theCache; }

    std::string viewCommand;

  protected:
    //what data members do you require?
    std::vector<std::string> columns;
    RowCollection rows;
    Timer *timer;
    LFUCache<std::string, std::string> *viewCache;

    std::string buildBar();
  };

  // // USE: general tabular view (with columns)
  // class JoinView : public View {
  // public:
  //                 JoinView(Timer* aTimer): timer(aTimer) {};
  //                 ~JoinView() {};

  //   bool          show(std::ostream &anOutput);

  //   StatusResult  settables(std::vector<std::string> theTables);
  //   StatusResult  setcols(std::vector<std::string> theCols);
  //   StatusResult  setRowPairCollection(RowPairCollection &theRowPairs);

  // protected:
  //   //what data members do you require?
  //   std::vector<std::string>  columns;
  //   std::vector<std::string>  tables;
  //   RowPairCollection         rowpairs;
  //   Timer*                    timer;

  //   std::string    buildBar();
  // };

}

#endif /* TabularView_h */
