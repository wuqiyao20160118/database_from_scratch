#include <iostream>
#include "DBProcessor.hpp"
#include "FolderView.hpp"
#include "Config.hpp"
#include "DBStatements.hpp"
#include "Index.hpp"

namespace ECE141
{

  using StatementFactory = Statement *(*)(DBProcessor *aProc);

  Statement *DBcreateStatementFactory(DBProcessor *aProc)
  {
    return new CreateDBStatement(aProc);
  }

  Statement *DBdropStatementFactory(DBProcessor *aProc)
  {
    return new DropDBStatement(aProc);
  }

  Statement *DBdumpStatementFactory(DBProcessor *aProc)
  {
    return new DumpDBStatement(aProc);
  }

  Statement *DBuseStatementFactory(DBProcessor *aProc)
  {
    return new UseDBStatement(aProc);
  }

  Statement *DBshowStatementFactory(DBProcessor *aProc)
  {
    return new ShowDBStatement(aProc);
  }

  DBProcessor::DBProcessor(std::ostream &anOutput) : CmdProcessor(anOutput), activeDB(nullptr) {}

  DBProcessor::~DBProcessor()
  {
    delete activeDB;
  }

  CmdProcessor *DBProcessor::recognizes(Tokenizer &aTokenizer)
  {
    return CreateDBStatement::recognizes(aTokenizer) ||
                   DumpDBStatement::recognizes(aTokenizer) ||
                   DropDBStatement::recognizes(aTokenizer) ||
                   ShowDBStatement::recognizes(aTokenizer) ||
                   UseDBStatement::recognizes(aTokenizer) ||
                   ShowIndexesStatement::recognizes(aTokenizer) ||
                   ShowIndexStatement::recognizes(aTokenizer)
               ? this
               : nullptr;
  }

  StatusResult DBProcessor::run(Statement *aStmt, Timer *aTimer)
  {
    timer = aTimer;
    return aStmt->dispatch();
  }

  // USE: retrieve a statement based on given text input...
  Statement *DBProcessor::makeStatement(Tokenizer &aTokenizer)
  {
    TokenSequence theSeq(aTokenizer);
    if (theSeq.is(Keywords::show_kw).then(Keywords::indexes_kw).matches())
    {
      if (Statement *theStatement = new ShowIndexesStatement(this))
      {
        if (StatusResult theResult = theStatement->parse(aTokenizer))
        {
          return theStatement;
        }
      }
      return nullptr;
    }

    theSeq.clear();
    if (theSeq.is(Keywords::show_kw).then(Keywords::index_kw).matches())
    {
      if (Statement *theStatement = new ShowIndexStatement(this))
      {
        if (StatusResult theResult = theStatement->parse(aTokenizer))
        {
          return theStatement;
        }
      }
      return nullptr;
    }

    static std::map<Keywords, StatementFactory> factories = {
        {Keywords::create_kw, DBcreateStatementFactory},
        {Keywords::drop_kw, DBdropStatementFactory},
        {Keywords::dump_kw, DBdumpStatementFactory},
        {Keywords::use_kw, DBuseStatementFactory},
        {Keywords::show_kw, DBshowStatementFactory},
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

  StatusResult DBProcessor::showDatabases()
  {
    FolderView theView(Config::getStoragePath(), Config::getDBExtension(), timer);
    theView.show(output);
    return StatusResult();
  }

  StatusResult DBProcessor::useDatabase(const std::string &aName)
  {
    if (dbExists(aName))
    {
      releaseDB();
      activeDB = new Database(aName, OpenDB{}, mUseCache["Block"]);
      output << "Database changed\n";
      return StatusResult{noError};
    }
    return StatusResult{unknownDatabase};
  }

  StatusResult DBProcessor::createDatabase(const std::string &aName)
  {
    if (!dbExists(aName))
    {
      Database *aDB;
      if (aDB = new Database(aName, CreateDB{}, mUseCache["Block"]))
      {
        timer->stop();
        output << "Query OK, 1 row affected (" << timer->elapsed() << " secs)\n";
        return StatusResult{noError};
      }
      delete aDB;
      return StatusResult{databaseCreationError};
    }
    return StatusResult{databaseExists};
  }

  StatusResult DBProcessor::dropDatabase(const std::string &aName)
  {
    if (dbExists(aName))
    {
      if (iscurrentDB(aName))
      {
        releaseDB();
      }
      Database *thedbptr = new Database(aName, OpenDB(), mUseCache["Block"]);
      int tablenum = thedbptr->getIndexIndex().size();
      delete thedbptr;
      std::remove(Config::getDBPath(aName).c_str());
      timer->stop();
      output << "Query OK, " << tablenum << " row affected (" << timer->elapsed() << " secs)\n";
      return StatusResult{noError};
    }
    return StatusResult{unknownDatabase};
  }

  StatusResult DBProcessor::dumpDatabase(const std::string &aName)
  {
    Database thedb(aName, OpenDB(), mUseCache["Block"]);
    std::string theBar = "+----------------+--------+---------------+\n";
    size_t theCount = 0;
    output << theBar;
    output << "| Type           | Id     | Extra         |\n";
    output << theBar;
    thedb.getStorage().each([&](const Block &block, uint32_t id)
                            {
                              output << "| " << std::left << std::setw(15) << block.header.type;
                              output << "| " << std::left << std::setw(7) << id;
                              output << "| " << std::left << std::setw(14) << block.header.refId << "|\n";
                              output << theBar;
                              theCount++;
                              return true;
                            });
    timer->stop();
    output << theCount << " rows in set (" << timer->elapsed() << " secs)\n";
    return StatusResult();
  }

  StatusResult DBProcessor::showIndexes()
  {
    std::map<std::string, uint32_t> &indexindex = activeDB->getIndexIndex();
    std::string theBar = "+-----------------+-----------------+\n";
    size_t theCount = 0;
    output << theBar;
    output << "| table           | field(s)        |\n";
    output << theBar;
    for (const auto &indexpair : indexindex)
    {
      output << "| " << std::left << std::setw(16) << indexpair.first;
      output << "| id              |\n";
      output << theBar;
      theCount++;
    }
    timer->stop();
    output << theCount << " rows in set (" << timer->elapsed() << " secs)\n";
    return StatusResult();
  }

  StatusResult DBProcessor::showIndex(const std::string &aName)
  {

    std::map<std::string, uint32_t> &indexindex = activeDB->getIndexIndex();
    if (!indexindex.count(aName))
    {
      return StatusResult{unknownTable};
    }

    Index index(activeDB->getStorage(), indexindex[aName], "id", IndexType::intKey, Entity::hashString(aName));
    std::stringstream ss;
    activeDB->getStorage().load(ss, indexindex[aName]);
    index.decode(ss);

    std::string theBar = "+-----------------+-----------------+\n";
    size_t theCount = 0;
    output << theBar;
    output << "| key             | block#          |\n";
    output << theBar;

    index.eachKV([&](const IndexKey &key, uint32_t value)
                 {
                   output << "| " << std::left << std::setw(16) << std::get<uint32_t>(key);
                   output << "| " << std::left << std::setw(16) << value << "|\n";
                   output << theBar;
                   theCount++;
                   return true;
                 });

    timer->stop();
    output << theCount << " rows in set (" << timer->elapsed() << " secs)\n";
    return StatusResult();
  }

  bool DBProcessor::dbExists(const std::string &theName)
  {
    FolderReader reader(Config::getStoragePath());
    bool ifexist = false;
    reader.each(Config::getDBExtension(), [&](const std::string &aName)
                {
                  if (theName == aName)
                  {
                    ifexist = true;
                    return false;
                  }
                  return true;
                });
    return ifexist;
  }

  bool DBProcessor::iscurrentDB(const std::string aName)
  {
    if (activeDB)
    {
      return activeDB->is(aName);
    }
    return false;
  }

  DBProcessor &DBProcessor::releaseDB()
  {
    delete activeDB;
    activeDB = nullptr;
    return *this;
  }

}
