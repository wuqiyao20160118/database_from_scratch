#ifndef DBStatement_hpp
#define DBStatement_hpp

#include "Statement.hpp"
#include "DBProcessor.hpp"
#include "TokenSequence.hpp"

namespace ECE141 {
  
  class CreateDBStatement: public Statement {
  public:
    CreateDBStatement(DBProcessor *aProc): Statement(Keywords::create_kw), Proc(aProc), name("") {}
    
    StatusResult  parse(Tokenizer &aTokenizer) {
      name=aTokenizer.peek(2).data;
      aTokenizer.next(3);
      return StatusResult{noError};
    }
    
    StatusResult  dispatch() {return Proc->createDatabase(name);}

    static bool   recognizes(Tokenizer &aTokenizer) {
      if (aTokenizer.remaining()>2) {
        TokenSequence theSeq(aTokenizer);
        return theSeq.is(Keywords::create_kw).then(Keywords::database_kw).thenID().matches();
      }
      return false;
    }

  protected:
    DBProcessor *Proc;
    std::string name; 
  };

  class DropDBStatement: public Statement {
  public:
    DropDBStatement(DBProcessor *aProc): Statement(Keywords::drop_kw), Proc(aProc), name("") {}
    
    StatusResult  parse(Tokenizer &aTokenizer) {
      name=aTokenizer.peek(2).data;
      aTokenizer.next(3);
      return StatusResult{noError};
    }
    
    StatusResult  dispatch() {return Proc->dropDatabase(name);}

    static bool   recognizes(Tokenizer &aTokenizer) {
      if (aTokenizer.remaining()>2) {
        TokenSequence theSeq(aTokenizer);
        return theSeq.is(Keywords::drop_kw).then(Keywords::database_kw).thenID().matches();
      }
      return false;
    }

  protected:
    DBProcessor *Proc;
    std::string name; 
  };

  class UseDBStatement: public Statement {
  public:
    UseDBStatement(DBProcessor *aProc): Statement(Keywords::use_kw), Proc(aProc), name("") {}
    
    StatusResult  parse(Tokenizer &aTokenizer) {
      name=aTokenizer.peek(1).data;
      aTokenizer.next(2);
      return StatusResult{noError};
    }
    
    StatusResult  dispatch() {return Proc->useDatabase(name);}

    static bool   recognizes(Tokenizer &aTokenizer) {
      if (aTokenizer.remaining()>1) {
        TokenSequence theSeq(aTokenizer);
        return theSeq.is(Keywords::use_kw).thenID().matches();
      }
      return false;
    }

  protected:
    DBProcessor *Proc;
    std::string name; 
  };

  class ShowDBStatement: public Statement {
  public:
    ShowDBStatement(DBProcessor *aProc): Statement(Keywords::show_kw), Proc(aProc) {}
    
    StatusResult  parse(Tokenizer &aTokenizer) {
      aTokenizer.next(2);
      return StatusResult{noError};
    }
    
    StatusResult  dispatch() {return Proc->showDatabases();}

    static bool   recognizes(Tokenizer &aTokenizer) {
      if (aTokenizer.remaining()>1) {
        TokenSequence theSeq(aTokenizer);
        return theSeq.is(Keywords::show_kw).then(Keywords::databases_kw).matches();
      }
      return false;
    }

  protected:
    DBProcessor *Proc;
  };
  
  
  class DumpDBStatement: public Statement {
  public:
    DumpDBStatement(DBProcessor *aProc): Statement(Keywords::dump_kw), Proc(aProc), name("") {}
    
    StatusResult  parse(Tokenizer &aTokenizer) {
      name=aTokenizer.peek(2).data;
      aTokenizer.next(3);
      return StatusResult{noError};
    }
    
    StatusResult  dispatch() {return Proc->dumpDatabase(name);}

    static bool   recognizes(Tokenizer &aTokenizer) {
      if (aTokenizer.remaining()>2) {
        TokenSequence theSeq(aTokenizer);
        return theSeq.is(Keywords::dump_kw).then(Keywords::database_kw).thenID().matches();
      }
      return false;
    }

  protected:
    DBProcessor *Proc;
    std::string name; 
  };

  class ShowIndexesStatement: public Statement {
  public:
    ShowIndexesStatement(DBProcessor *aProc): Statement(Keywords::show_kw), Proc(aProc) {}
    
    StatusResult  parse(Tokenizer &aTokenizer) {
      aTokenizer.next(2);
      return StatusResult{noError};
    }
    
    StatusResult  dispatch() {return Proc->showIndexes();}

    static bool   recognizes(Tokenizer &aTokenizer) {
      if (aTokenizer.remaining()>1) {
        TokenSequence theSeq(aTokenizer);
        return theSeq.is(Keywords::show_kw).then(Keywords::indexes_kw).matches();
      }
      return false;
    }

  protected:
    DBProcessor *Proc;
  };

  class ShowIndexStatement: public Statement {
  public:
    ShowIndexStatement(DBProcessor *aProc): Statement(Keywords::show_kw), Proc(aProc), name("") {}
    
    StatusResult  parse(Tokenizer &aTokenizer) {
      name=aTokenizer.peek(4).data;
      aTokenizer.next(5);
      return StatusResult{noError};
    }
    
    StatusResult  dispatch() {return Proc->showIndex(name);}

    static bool   recognizes(Tokenizer &aTokenizer) {
      if (aTokenizer.remaining()>4) {
        TokenSequence theSeq(aTokenizer);
        return theSeq.is(Keywords::show_kw).then(Keywords::index_kw).until(Keywords::from_kw).thenID().matches();
      }
      return false;
    }

  protected:
    DBProcessor *Proc;
    std::string name; 
  };

}

#endif