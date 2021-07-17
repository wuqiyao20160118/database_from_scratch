//
//  CommandProcessor.cpp
//  ECEDatabase
//
//  Created by rick gessner on 3/30/18.
//  Copyright © 2018 rick gessner. All rights reserved.
//

#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include "Application.hpp"
#include "Tokenizer.hpp"


namespace ECE141 {
  
  Application::Application(std::ostream &anOutput)
    : CmdProcessor(anOutput), ddl(anOutput), sdl(anOutput) {
    sdl.dbproc = &ddl;
  }
  
  Application::~Application() {
    //if(database) delete database;
  }

  // USE: -----------------------------------------------------
  
  bool isKnown(Keywords aKeyword) {
    static Keywords theKnown[]={Keywords::quit_kw, Keywords::help_kw, Keywords::version_kw};
    auto theIt = std::find(std::begin(theKnown),
                           std::end(theKnown), aKeyword);
    return theIt!=std::end(theKnown);
  }

  CmdProcessor* Application::recognizes(Tokenizer &aTokenizer) {
    if(isKnown(aTokenizer.current().keyword)) {
      return this;
    }
    if (auto *DBproc = ddl.recognizes(aTokenizer)) {
        return DBproc;
    }
    if (auto* SQLproc = sdl.recognizes(aTokenizer)) { 
        return SQLproc;
    }
    return nullptr;
  }

  StatusResult shutdown(std::ostream &anOutput) {
    anOutput << "DB::141 is shutting down.\n";
    return StatusResult(ECE141::userTerminated);
  }

  StatusResult help(std::ostream &anOutput) {
    anOutput << "Help system ready.\n";
    return StatusResult();
  }

  StatusResult version(std::ostream &anOutput) {
    anOutput << "Version 0.9.\n";
    return StatusResult();
  }

  StatusResult Application::run(Statement *aStmt, Timer *aTimer) {
    //STUDENT: Add support for HELP and VERSION commands...

    switch(aStmt->getType()) {
      case Keywords::quit_kw: return shutdown(output);
      case Keywords::help_kw: return help(output);
      case Keywords::version_kw: return version(output);
      default: break;
    }
    return StatusResult{Errors::noError};
  }
    
  // USE: retrieve a statement based on given text input...
  Statement* Application::makeStatement(Tokenizer &aTokenizer) {
    Token theToken=aTokenizer.current();
    if (isKnown(theToken.keyword)) {
      aTokenizer.next(); //skip ahead...
      return new Statement(theToken.keyword);
    }

    return nullptr;
  }

  //build a tokenizer, tokenize input, ask processors to handle...
  StatusResult Application::handleInput(std::istream &anInput) {
    Tokenizer theTokenizer(anInput);
    ECE141::StatusResult theResult=theTokenizer.tokenize();
    while (theResult && theTokenizer.more()) {
      Timer theTimer;
      theTimer.start();    
      if(auto *theProc=recognizes(theTokenizer)) {
        if(auto *theCmd=theProc->makeStatement(theTokenizer)) {
          theResult=theProc->run(theCmd, &theTimer);
          if(theResult) theTokenizer.skipIf(';');
          delete theCmd;
        }
      }
      else theTokenizer.next(); //force skip ahead...
    }
    return theResult;
  }
}
