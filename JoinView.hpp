#ifndef JoinView_h
#define JoinView_h

#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include <vector>
#include <string>
#include "Timer.hpp"
#include "View.hpp"
#include "Row.hpp"

namespace ECE141
{
  class JoinView : public View
  {
  public:
    JoinView(Timer *aTimer) : timer(aTimer){};
    ~JoinView(){};

    bool show(std::ostream &anOutput);

    StatusResult settables(StringList theTables);
    StatusResult setcols(StringList theCols);
    StatusResult setRowPairCollection(RowPairCollection &theRowPairs);
    void setCache(LFUCache<std::string, std::string> *theCache) { viewCache = theCache; }

    std::string viewCommand;

  protected:
    //what data members do you require?
    StringList columns;
    StringList tables;
    RowPairCollection rowpairs;
    Timer *timer;
    LFUCache<std::string, std::string> *viewCache;

    std::string buildBar();
  };

}

#endif /* JoinView_h */
