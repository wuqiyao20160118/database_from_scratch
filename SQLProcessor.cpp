//
//  SQLProcessor.cpp
//  RGAssignment3
//
//  Created by rick gessner on 4/1/21.
//

#include "SQLProcessor.hpp"
#include "Application.hpp"
#include "Database.hpp"
#include "Helpers.hpp"
#include "TabularView.hpp"
#include "Entity.hpp"
#include "BlockIO.hpp"
#include "EntityView.hpp"
#include "SQLStatements.hpp"
#include "JoinView.hpp"
#include "Config.hpp"
// #include "DataProvider.hpp"

namespace ECE141
{

  using StatementFactory = Statement *(*)(SQLProcessor *aProc);

  Statement *SQLcreateStatementFactory(SQLProcessor *aProc)
  {
    return new CreateSQLStatement(aProc);
  }

  Statement *SQLdropStatementFactory(SQLProcessor *aProc)
  {
    return new DropSQLStatement(aProc);
  }

  Statement *SQLshowStatementFactory(SQLProcessor *aProc)
  {
    return new ShowSQLStatement(aProc);
  }

  Statement *SQLdescribeStatementFactory(SQLProcessor *aProc)
  {
    return new DescribeSQLStatement(aProc);
  }

  Statement *SQLinsertStatementFactory(SQLProcessor *aProc)
  {
    return new InsertSQLStatement(aProc);
  }

  Statement *SQLselectStatementFactory(SQLProcessor *aProc)
  {
    return new SelectSQLStatement(aProc);
  }

  Statement *SQLdeleteStatementFactory(SQLProcessor *aProc)
  {
    return new DeleteSQLStatement(aProc);
  }

  Statement *SQLupdateStatementFactory(SQLProcessor *aProc)
  {
    return new UpdateSQLStatement(aProc);
  }

  Statement *SQLalterStatementFactory(SQLProcessor *aProc)
  {
    return new AlterTableStatement(aProc);
  }

  SQLProcessor::SQLProcessor(std::ostream &anOutput) : CmdProcessor(anOutput), dbproc(nullptr) {}

  SQLProcessor::~SQLProcessor()
  {
  }

  CmdProcessor *SQLProcessor::recognizes(Tokenizer &aTokenizer)
  {
    return CreateSQLStatement::recognizes(aTokenizer) ||
                   DropSQLStatement::recognizes(aTokenizer) ||
                   ShowSQLStatement::recognizes(aTokenizer) ||
                   DescribeSQLStatement::recognizes(aTokenizer) ||
                   InsertSQLStatement::recognizes(aTokenizer) ||
                   SelectSQLStatement::recognizes(aTokenizer) ||
                   DeleteSQLStatement::recognizes(aTokenizer) ||
                   UpdateSQLStatement::recognizes(aTokenizer) ||
                   AlterTableStatement::recognizes(aTokenizer)
               ? this
               : nullptr;
  }

  StatusResult SQLProcessor::run(Statement *aStmt, Timer *aTimer)
  {
    timer = aTimer;
    return aStmt->dispatch();
  }

  // USE: retrieve a statement based on given text input...
  Statement *SQLProcessor::makeStatement(Tokenizer &aTokenizer)
  {
    static std::map<Keywords, StatementFactory> factories = {
        {Keywords::create_kw, SQLcreateStatementFactory},
        {Keywords::drop_kw, SQLdropStatementFactory},
        {Keywords::describe_kw, SQLdescribeStatementFactory},
        {Keywords::show_kw, SQLshowStatementFactory},
        {Keywords::insert_kw, SQLinsertStatementFactory},
        {Keywords::select_kw, SQLselectStatementFactory},
        {Keywords::delete_kw, SQLdeleteStatementFactory},
        {Keywords::update_kw, SQLupdateStatementFactory},
        {Keywords::alter_kw, SQLalterStatementFactory},
    };

    Token &theToken = aTokenizer.current();
    if (factories.count(theToken.keyword))
    {
      if (Statement *theStatement = factories[theToken.keyword](this))
      {
        if (StatusResult theResult = theStatement->parse(aTokenizer))
        {
          return theStatement;
        }
      }
    }
    return nullptr;
  }

  StatusResult SQLProcessor::createTable(const std::string &aName, AttributeList &attributes)
  {
    Entity entity(aName);
    for (int i = 0; i < attributes.size(); i++)
    {
      Attribute theAttribute = attributes[i];
      entity.addAttribute(attributes[i]);
    }

    // add a auto generated primary key names '_id'
    Attribute theID;
    theID.field_name = "_id";
    theID.primary_key = false; // self generated key
    theID.auto_increment = true;
    theID.nullable = false;
    theID.field_type = DataTypes::int_type;
    entity.addAttribute(theID);

    (dbproc->getactiveDB())->addEntity(entity);

    timer->stop();
    output << "Query OK, 0 rows affected (" << timer->elapsed() << " sec)" << std::endl;
    return StatusResult{noError};
  }

  StatusResult SQLProcessor::alterTable(Keywords &alterType, Attribute &theAttribute, Entity *theEntity)
  {
    if (alterType == Keywords::add_kw)
    {
      if (theEntity->addAttribute(theAttribute))
      {
        StatusResult result = (dbproc->getactiveDB())->updateEntity(*theEntity);
        if (!result)
          return result;
      }
      else
        StatusResult{syntaxError};
    }
    else if (alterType == Keywords::drop_kw)
    {
      if (theEntity->removeAttribute(theAttribute))
      {
        StatusResult result = (dbproc->getactiveDB())->updateEntity(*theEntity);
        if (!result)
          return result;
      }
      else
        StatusResult{syntaxError};
    }
    else
    {
      // Keywords::modify_kw
      if (theEntity->modifyAttribute(theAttribute))
      {
        StatusResult result = (dbproc->getactiveDB())->updateEntity(*theEntity);
        if (!result)
          return result;
      }
      else
        StatusResult{syntaxError};
    }

    timer->stop();
    output << "Query OK, 0 rows affected (" << timer->elapsed() << " sec)" << std::endl;
    return StatusResult{noError};
  }

  StatusResult SQLProcessor::showTables()
  {
    EntityView theview(dbproc->getactiveDB()->getStorage(), timer);
    theview.show(output);
    return StatusResult{noError};
  }

  StatusResult SQLProcessor::dropTable(const std::string &aName)
  {
    int rownum;
    StatusResult result = dbproc->getactiveDB()->dropTable(aName, rownum);
    if (!result)
    {
      return result;
    }

    timer->stop();
    output << "Query OK, " << 0 << " rows affected (" << timer->elapsed() << " sec)" << std::endl;
    return StatusResult{noError};
  }

  StatusResult SQLProcessor::describeTable(const std::string &aName)
  {
    Database *theDB = dbproc->getactiveDB();

    if (theDB)
    {
      Entity *theEntity = theDB->getEntity(aName);
      if (theEntity)
      {
        DescribeView theView(theEntity, timer);
        theView.show(output);
        delete theEntity;
        return StatusResult{noError};
      }
      return StatusResult{unknownTable};
    }

    return StatusResult{unknownDatabase};
  }

  StatusResult SQLProcessor::insertRows(const RowCollection &aRows)
  {
    Database *activeDB = dbproc->getactiveDB();
    activeDB->insertRows(aRows);

    timer->stop();
    output << "Query OK, " << aRows.size() << " rows affected (" << timer->elapsed() << " sec"
           << ")" << '\n';
    return StatusResult{noError};
  }

  StatusResult SQLProcessor::showQuery(Query &aQuery)
  {
    if (mUseCache["View"] && dbproc->getactiveDB()->getViewCache()->contains(aQuery.commandStr))
    {
      timer->stop();
      output << dbproc->getactiveDB()->getViewCache()->get(aQuery.commandStr) << "(" << timer->elapsed() << " secs)\n";
      return StatusResult{noError};
    }
    RowCollection rows;
    dbproc->getactiveDB()->selectRows(aQuery.getFrom()->name, rows);

    // we still need to order the rows based on query
    aQuery.run(rows);

    // construct and show the view
    TabularView theView(timer);
    StatusResult result = theView.setcolumns(aQuery.getSelect());
    if (result != StatusResult{noError})
      return result;

    result = theView.setRowCollection(rows); // construct the required rows
    if (result != StatusResult{noError})
      return result;

    theView.viewCommand = aQuery.commandStr;
    if (mUseCache["View"])
      theView.setCache(dbproc->getactiveDB()->getViewCache());
    theView.show(output);
    timer->stop();
    output << "(" << timer->elapsed() << " secs)\n";

    return StatusResult{noError};
  }

  void SQLProcessor::assembleRowPair(Join &aJoin, RowCollection &leftrows, RowCollection &rightrows, RowPairCollection &rowpairs, bool inner)
  {
    for (auto &leftrow : leftrows)
    {
      int count = 0;
      for (auto &rightrow : rightrows)
      {
        RowPair *rowpair = new RowPair(*leftrow, *rightrow);
        if ((*aJoin.expr)(*rowpair))
        {
          rowpairs.push_back(std::unique_ptr<RowPair>(rowpair));
          count++;
        }
      }
      if (count == 0 && !inner)
      {
        Row *aRowPtr = new Row(aJoin.right->name);
        RowPair *rowpair = new RowPair(*leftrow, *aRowPtr);
        rowpairs.push_back(std::unique_ptr<RowPair>(rowpair));
        delete aRowPtr;
      }
    }
  }

  void SQLProcessor::assembleFullRowPair(Join &aJoin, RowCollection &leftrows, RowCollection &rightrows, RowPairCollection &rowpairs)
  {
    // left side
    for (auto &leftrow : leftrows)
    {
      int count = 0;
      for (auto &rightrow : rightrows)
      {
        RowPair *rowpair = new RowPair(*leftrow, *rightrow);
        if ((*aJoin.expr)(*rowpair))
        {
          count++;
        }
      }
      if (count == 0)
      {
        Row *aRowPtr = new Row(aJoin.right->name);
        RowPair *rowpair = new RowPair(*leftrow, *aRowPtr);
        rowpairs.push_back(std::unique_ptr<RowPair>(rowpair));
        delete aRowPtr;
      }
    }

    // right side
    for (auto &rightrow : rightrows)
    {
      int count = 0;
      for (auto &leftrow : leftrows)
      {
        RowPair *rowpair = new RowPair(*leftrow, *rightrow);
        if ((*aJoin.expr)(*rowpair))
        {
          count++;
        }
      }
      if (count == 0)
      {
        Row *aRowPtr = new Row(aJoin.left->name);
        RowPair *rowpair = new RowPair(*aRowPtr, *rightrow);
        rowpairs.push_back(std::unique_ptr<RowPair>(rowpair));
        delete aRowPtr;
      }
    }
    // inner part
    assembleRowPair(aJoin, rightrows, leftrows, rowpairs, true);
  }

  void SQLProcessor::assembleCrossRowPair(Join &aJoin, RowCollection &leftrows, RowCollection &rightrows, RowPairCollection &rowpairs)
  {
    for (auto &leftrow : leftrows)
    {
      for (auto &rightrow : rightrows)
      {
        RowPair *rowpair = new RowPair(*leftrow, *rightrow);
        rowpairs.push_back(std::unique_ptr<RowPair>(rowpair));
      }
    }
  }

  StatusResult SQLProcessor::showJoin(Join &aJoin)
  {
    RowPairCollection rowpairs;
    RowCollection leftrows, rightrows;
    dbproc->getactiveDB()->selectRows(aJoin.left->name, leftrows);
    dbproc->getactiveDB()->selectRows(aJoin.right->name, rightrows);

    if (aJoin.joinType == Keywords::left_kw)
    {
      assembleRowPair(aJoin, leftrows, rightrows, rowpairs);
    }
    else if (aJoin.joinType == Keywords::right_kw)
    {
      assembleRowPair(aJoin, rightrows, leftrows, rowpairs);
    }
    else if (aJoin.joinType == Keywords::inner_kw)
    {
      assembleRowPair(aJoin, leftrows, rightrows, rowpairs, true);
    }
    else if (aJoin.joinType == Keywords::full_kw)
    {
      assembleFullRowPair(aJoin, leftrows, rightrows, rowpairs);
    }
    else
    {
      // cross join has no on key
      assembleCrossRowPair(aJoin, leftrows, rightrows, rowpairs);
    }

    // construct and show the view
    JoinView theView(timer);
    theView.viewCommand = aJoin.commandStr;
    if (mUseCache["View"])
      theView.setCache(dbproc->getactiveDB()->getViewCache());

    StatusResult result = theView.settables(aJoin.table_names);
    if (result != StatusResult{noError})
      return result;
    result = theView.setcols(aJoin.attribute_names);
    if (result != StatusResult{noError})
      return result;
    result = theView.setRowPairCollection(rowpairs);
    if (result != StatusResult{noError})
      return result;

    theView.show(output);
    timer->stop();
    output << "(" << timer->elapsed() << " secs)\n";

    return StatusResult{noError};
  }

  StatusResult SQLProcessor::deleteRows(const Query &aQuery)
  {
    int rownum = dbproc->getactiveDB()->deleteRows(aQuery);
    timer->stop();
    output << "Query OK, " << rownum << " rows affected (" << timer->elapsed() << " sec"
           << ")" << '\n';
    return StatusResult{noError};
  }

  StatusResult SQLProcessor::updateRows(Query &aQuery)
  {
    int rownum = dbproc->getactiveDB()->updateRows(aQuery);
    timer->stop();
    output << "Query OK, " << rownum << " rows affected (" << timer->elapsed() << " sec"
           << ")" << '\n';
    return StatusResult{noError};
  }

}
