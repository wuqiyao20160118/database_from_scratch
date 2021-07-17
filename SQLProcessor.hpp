//
//  SQLProcessor.hpp
//  RGAssignment3
//
//  Created by rick gessner on 4/1/21.
//

#ifndef SQLProcessor_hpp
#define SQLProcessor_hpp

#include <stdio.h>
#include "CmdProcessor.hpp"
#include "DBProcessor.hpp"
#include "Tokenizer.hpp"
#include "Attribute.hpp"
#include "Row.hpp"
#include "Query.hpp"
#include "Join.hpp"

namespace ECE141
{

  using AttributeOpt = std::optional<Attribute>;
  using AttributeList = std::vector<Attribute>;

  class Statement;

  class SQLProcessor : public CmdProcessor
  {
  public:
    //STUDENT: Declare OCF methods...
    SQLProcessor(std::ostream &anOutput);
    virtual ~SQLProcessor();

    CmdProcessor *recognizes(Tokenizer &aTokenizer) override;
    Statement *makeStatement(Tokenizer &aTokenizer) override;
    StatusResult run(Statement *aStmt, Timer *aTimer) override;

    //STUDENT add any other methods your processor needs...
    StatusResult createTable(const std::string &aName, AttributeList &attributes);
    StatusResult showTables();
    StatusResult dropTable(const std::string &aName);
    StatusResult describeTable(const std::string &aName);
    StatusResult alterTable(Keywords &alterType, Attribute &theAttribute, Entity *theEntity);

    StatusResult insertRows(const RowCollection &aRows);
    StatusResult showQuery(Query &aQuery);
    StatusResult deleteRows(const Query &aQuery);
    StatusResult updateRows(Query &aQuery);
    StatusResult showJoin(Join &aJoin);

    DBProcessor *dbproc;
    //do you need other data members?
  private:
    void assembleRowPair(Join &aJoin, RowCollection &leftrows, RowCollection &rightrows, RowPairCollection &rowpairs, bool inner = false);
    void assembleFullRowPair(Join &aJoin, RowCollection &leftrows, RowCollection &rightrows, RowPairCollection &rowpairs);
    void assembleCrossRowPair(Join &aJoin, RowCollection &leftrows, RowCollection &rightrows, RowPairCollection &rowpairs);
  };

}

#endif /* SQLProcessor_hpp */
