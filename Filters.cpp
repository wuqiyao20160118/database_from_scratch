//
//  Filters.cpp
//  Datatabase5
//
//  Created by rick gessner on 3/5/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#include "Filters.hpp"
#include <string>
#include <limits>
#include "keywords.hpp"
#include "Helpers.hpp"
#include "Entity.hpp"
#include "Attribute.hpp"
#include "Compare.hpp"
#include "Row.hpp"

namespace ECE141 {
  
  using Comparitor = bool (*)(Value &aLHS, Value &aRHS);

  bool equals(Value &aLHS, Value &aRHS) {
    bool theResult=false;
    
    std::visit([&](auto const &aLeft) {
      std::visit([&](auto const &aRight) {
        theResult=isEqual(aLeft,aRight);
      },aRHS);
    },aLHS);
    return theResult;
  }

  bool notequals(Value &aLHS, Value &aRHS) {
    bool theResult=false;
    
    std::visit([&](auto const &aLeft) {
      std::visit([&](auto const &aRight) {
        theResult=!isEqual(aLeft,aRight);
      },aRHS);
    },aLHS);
    return theResult;
  }

  bool lessthan(Value &aLHS, Value &aRHS) {
    bool theResult=false;
    
    std::visit([&](auto const &aLeft) {
      std::visit([&](auto const &aRight) {
        theResult=isLess(aLeft,aRight);
      },aRHS);
    },aLHS);
    return theResult;
  }

  bool lessequal(Value &aLHS, Value &aRHS) {
    bool theResult=false;
    
    std::visit([&](auto const &aLeft) {
      std::visit([&](auto const &aRight) {
        theResult=isLess(aLeft,aRight)||isEqual(aLeft,aRight);
      },aRHS);
    },aLHS);
    return theResult;
  }

  bool greaterthan(Value &aLHS, Value &aRHS) {
    bool theResult=false;
    
    std::visit([&](auto const &aLeft) {
      std::visit([&](auto const &aRight) {
        theResult=isLess(aRight,aLeft);
      },aRHS);
    },aLHS);
    return theResult;
  }

  bool greaterequal(Value &aLHS, Value &aRHS) {
    bool theResult=false;
    
    std::visit([&](auto const &aLeft) {
      std::visit([&](auto const &aRight) {
        theResult=isLess(aRight,aLeft)||isEqual(aLeft,aRight);
      },aRHS);
    },aLHS);
    return theResult;
  }

  static std::map<Operators, Comparitor> comparitors {
    {Operators::equal_op, equals},
    {Operators::notequal_op, notequals},
    {Operators::lt_op, lessthan},
    {Operators::lte_op, lessequal},
    {Operators::gt_op, greaterthan},
    {Operators::gte_op, greaterequal},
    //STUDENT: Add more for other operators...
  };

  bool Expression::operator()(DataProvider& aDataProvider) {
    Value theLHS{lhs.value};
    Value theRHS{rhs.value};

    if(TokenType::identifier==lhs.ttype) {
      theLHS=aDataProvider.getValue(lhs); //get row value
    }

    if(TokenType::identifier==rhs.ttype) {
      theRHS=aDataProvider.getValue(rhs); //get row value
    }

    return comparitors.count(op)
      ? comparitors[op](theLHS, theRHS) : false;
  }
  
  //--------------------------------------------------------------
  
  Filters::Filters()  {}
  
  Filters::Filters(const Filters &aCopy)  {
  }
  
  Filters::~Filters() {
    //no need to delete expressions, they're unique_ptrs!
  }

  Filters& Filters::add(Expression *anExpression) {
    expressions.push_back(std::unique_ptr<Expression>(anExpression));
    return *this;
  }
    
  //compare expressions to row; return true if matches
  bool Filters::matches(DataProvider& aDataProvider) const {
    
    //STUDENT: You'll need to add code here to deal with
    //         logical combinations (AND, OR, NOT):
    //         like:  WHERE zipcode=92127 AND age>20
    bool match = true;
    
    for(auto &theExpr : expressions) {
      switch(theExpr->logic) {
        case Logical::not_op :
          match = match && !(*theExpr)(aDataProvider);
          break; 
        case Logical::or_op :
          match = match || (*theExpr)(aDataProvider);
          break;
        default:
          match = match && (*theExpr)(aDataProvider);
          break;
      }
    }
    return match;
  }
 

  //where operand is field, number, string...
  StatusResult parseOperand(Tokenizer &aTokenizer,
                            Entity &anEntity, Operand &anOperand) {
    StatusResult theResult{noError};
    Token &theToken = aTokenizer.current();
    if(TokenType::identifier==theToken.type) {
      if(auto *theAttr=anEntity.getAttribute(theToken.data)) {
        anOperand.ttype=theToken.type;
        anOperand.name=theToken.data; //hang on to name...
        // we need to specify the entity for the operand if it is an attribute
        anOperand.entityId=Entity::hashString(theToken.data);
        anOperand.dtype=theAttr->field_type;
      }
      else {
        anOperand.ttype=TokenType::string;
        anOperand.dtype=DataTypes::varchar_type;
        anOperand.value=theToken.data;
      }
    }
    else if(TokenType::number==theToken.type) {
      anOperand.ttype=TokenType::number;
      anOperand.dtype=DataTypes::int_type;
      if (theToken.data.find('.')!=std::string::npos) {
        anOperand.dtype=DataTypes::float_type;
        anOperand.value=std::stof(theToken.data);
      }
      else anOperand.value=std::stoi(theToken.data);
    }
    else theResult.error=syntaxError;

    if (theResult) aTokenizer.next();
    return theResult;
  }
    
  //STUDENT: Add validation here...
  bool validateOperands(Operand &aLHS, Operand &aRHS, Entity &anEntity) {
    if(TokenType::identifier==aLHS.ttype) { //most common case...
      //STUDENT: Add code for validation as necessary
      return true;
    }
    else if(TokenType::identifier==aRHS.ttype) {
      //STUDENT: Add code for validation as necessary
      return true;
    }
    return false;
  }

  //STUDENT: This starting point code may need adaptation...
  StatusResult Filters::parse(Tokenizer &aTokenizer,Entity &anEntity) {
    static std::map<std::string, Logical> logicMap = {
        {"AND", Logical::and_op},
        {"OR", Logical::or_op},
        {"NOT", Logical::not_op},
    };
    StatusResult  theResult{noError};

    while(theResult && (2<aTokenizer.remaining())) {
      Operand theLHS,theRHS;
      Logical theLogic{Logical::no_op};
      Token &theToken=aTokenizer.current();
      if(theToken.type!=TokenType::identifier) {
        if (theToken.type==TokenType::operators && logicMap.count(theToken.data)) {
          theLogic = logicMap[theToken.data];
        }
        return theResult;
      }
      if((theResult=parseOperand(aTokenizer,anEntity,theLHS))) {
        Token &theToken1=aTokenizer.current();
        Token &theToken2=aTokenizer.peek(1);
        if(theToken1.type==TokenType::operators) {
          Operators theOp;
          if(theToken2.type==TokenType::operators) {
            theOp=Helpers::toOperator(theToken1.data+theToken2.data);
            aTokenizer.next(2);
          }
          else {
            theOp=Helpers::toOperator(theToken1.data);
            aTokenizer.next();
          }          
          if((theResult=parseOperand(aTokenizer,anEntity,theRHS))) {
            if(validateOperands(theLHS, theRHS, anEntity)) {
              add(new Expression(theLHS, theOp, theRHS, theLogic));
              if(aTokenizer.skipIf(semicolon)) {
                break;
              }
            }
            else theResult.error=syntaxError;
          }
        }
      }
      else theResult.error=syntaxError;
    }
    return theResult;
  }

}

