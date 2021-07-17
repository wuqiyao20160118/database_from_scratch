//
//  Join.hpp
//  RGAssignment8
//
//  Created by rick gessner on 5/05/21.
//

#ifndef Join_h
#define Join_h

#include <string>
#include <vector>
#include "BasicTypes.hpp"
#include "Errors.hpp"
#include "keywords.hpp"
#include "Entity.hpp"

namespace ECE141
{

  class Join
  {
  public:
    Join() : left(nullptr), right(nullptr), joinType(Keywords::unknown_kw), expr(nullptr) {}
    ~Join()
    {
      delete left;
      delete right;
      delete expr;
    }

    StatusResult parseOperand(Tokenizer &aTokenizer, Operand &anOperand)
    {
      if (TokenType::identifier == aTokenizer.current().type)
      {
        std::string table_name = aTokenizer.current().data;
        std::string attribute_name;
        aTokenizer.next();
        if (aTokenizer.current().data != ".")
        {
          attribute_name = table_name;
          auto *theAttr = left->getAttribute(attribute_name);
          if (theAttr)
            table_name = left->name;
          theAttr = right->getAttribute(attribute_name);
          if (theAttr)
            table_name = right->name;
        }
        else
        {
          aTokenizer.next();
          attribute_name = aTokenizer.current().data;
          aTokenizer.next();
        }
        // if (aTokenizer.current().data != ".") return StatusResult{notImplemented};  // TODO

        if (table_name == left->name)
        {
          if (auto *theAttr = left->getAttribute(attribute_name))
          {
            anOperand.ttype = TokenType::identifier;
            anOperand.name = attribute_name; //hang on to name...
            // we need to specify the entity for the operand if it is an attribute
            anOperand.entityId = Entity::hashString(table_name);
            anOperand.dtype = theAttr->field_type;
            return StatusResult{noError};
          }
          return StatusResult{unknownAttribute};
        }
        else if (table_name == right->name)
        {
          if (auto *theAttr = right->getAttribute(attribute_name))
          {
            anOperand.ttype = TokenType::identifier;
            anOperand.name = attribute_name; //hang on to name...
            // we need to specify the entity for the operand if it is an attribute
            anOperand.entityId = Entity::hashString(table_name);
            anOperand.dtype = theAttr->field_type;
            return StatusResult{noError};
          }
          return StatusResult{unknownAttribute};
        }

        return StatusResult{unknownTable};
      }
      return StatusResult{syntaxError};
    }

    Entity *left;
    Entity *right;
    Keywords joinType;
    Expression *expr;
    std::vector<std::string> attribute_names;
    std::vector<std::string> table_names;
    std::string commandStr;
  };

  using JoinList = std::vector<Join>;

}

#endif /* Join_h */
