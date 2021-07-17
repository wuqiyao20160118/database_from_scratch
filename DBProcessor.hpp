#ifndef DBProcessor_hpp
#define DBProcessor_hpp

#include <stdio.h>
#include "CmdProcessor.hpp"
#include "Database.hpp"

namespace ECE141 {

  class DBProcessor : public CmdProcessor {
  public:
    
    DBProcessor(std::ostream &anOutput);
    virtual ~DBProcessor();

    CmdProcessor* recognizes(Tokenizer &aTokenizer) override;
    Statement*    makeStatement(Tokenizer &aTokenizer) override;
    StatusResult  run(Statement *aStmt, Timer *aTimer) override;

    StatusResult  showDatabases();
    StatusResult  useDatabase(const std::string &aName);
    StatusResult  createDatabase(const std::string &aName);
    StatusResult  dropDatabase(const std::string &aName);

    StatusResult  dumpDatabase(const std::string &aName);
    StatusResult  showIndexes();
    StatusResult  showIndex(const std::string &aName);

    Database*     getactiveDB() {return activeDB;}

  protected:

    bool         dbExists(const std::string &theName); 
    bool         iscurrentDB(const std::string aName);
    DBProcessor& releaseDB();

    Database*    activeDB;
  };
  
}

#endif
