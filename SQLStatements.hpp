#ifndef SQLStatement_hpp
#define SQLStatement_hpp

#include "Statement.hpp"
#include "SQLProcessor.hpp"
#include "TokenSequence.hpp"
#include "Row.hpp"
#include "Query.hpp"
#include "Helpers.hpp"

namespace ECE141
{

  class CreateSQLStatement : public Statement
  {
  public:
    CreateSQLStatement(SQLProcessor *aProc) : Statement(Keywords::create_kw), Proc(aProc), name("") {}

    StatusResult parse(Tokenizer &aTokenizer)
    {
      name = aTokenizer.peek(2).data;
      aTokenizer.next(4);
      while (aTokenizer.current().data != ";")
      {
        Attribute anAttribute;
        anAttribute.field_name = aTokenizer.current().data;
        aTokenizer.next();
        while (aTokenizer.current().data != "," && aTokenizer.current().data != ")")
        {

          if (aTokenizer.current().data == "default")
          {
            anAttribute.hasdefault = true;
            anAttribute.nullable = true;
            aTokenizer.next();
            anAttribute.defaultvalue = aTokenizer.current().data;
            aTokenizer.next();
            continue;
          }

          StatusResult result = Helpers::parseAttributeProp(aTokenizer, anAttribute);
          if (!result)
            return result;
        }
        attributes.push_back(anAttribute);
        aTokenizer.next();
      }
      return StatusResult{noError};
    }

    StatusResult dispatch() { return Proc->createTable(name, attributes); }

    static bool recognizes(Tokenizer &aTokenizer)
    {
      if (aTokenizer.remaining() > 4)
      {
        TokenSequence theSeq(aTokenizer);
        return theSeq.is(Keywords::create_kw).then(Keywords::table_kw).thenID().matches();
      }
      return false;
    }

  protected:
    SQLProcessor *Proc;
    std::string name;
    AttributeList attributes;
  };

  class DropSQLStatement : public Statement
  {
  public:
    DropSQLStatement(SQLProcessor *aProc) : Statement(Keywords::drop_kw), Proc(aProc), name("") {}

    StatusResult parse(Tokenizer &aTokenizer)
    {
      name = aTokenizer.peek(2).data;
      aTokenizer.next(3);
      return StatusResult{noError};
    }

    StatusResult dispatch() { return Proc->dropTable(name); }

    static bool recognizes(Tokenizer &aTokenizer)
    {
      if (aTokenizer.remaining() > 2)
      {
        TokenSequence theSeq(aTokenizer);
        return theSeq.is(Keywords::drop_kw).then(Keywords::table_kw).thenID().matches();
      }
      return false;
    }

  protected:
    SQLProcessor *Proc;
    std::string name;
  };

  class ShowSQLStatement : public Statement
  {
  public:
    ShowSQLStatement(SQLProcessor *aProc) : Statement(Keywords::show_kw), Proc(aProc) {}

    StatusResult parse(Tokenizer &aTokenizer)
    {
      aTokenizer.next(2);
      return StatusResult{noError};
    }

    StatusResult dispatch() { return Proc->showTables(); }

    static bool recognizes(Tokenizer &aTokenizer)
    {
      if (aTokenizer.remaining() > 1)
      {
        TokenSequence theSeq(aTokenizer);
        return theSeq.is(Keywords::show_kw).then(Keywords::tables_kw).matches();
      }
      return false;
    }

  protected:
    SQLProcessor *Proc;
  };

  class DescribeSQLStatement : public Statement
  {
  public:
    DescribeSQLStatement(SQLProcessor *aProc) : Statement(Keywords::describe_kw), Proc(aProc), name("") {}

    StatusResult parse(Tokenizer &aTokenizer)
    {
      name = aTokenizer.peek(1).data;
      aTokenizer.next(2);
      return StatusResult{noError};
    }

    StatusResult dispatch() { return Proc->describeTable(name); }

    static bool recognizes(Tokenizer &aTokenizer)
    {
      if (aTokenizer.remaining() > 1)
      {
        TokenSequence theSeq(aTokenizer);
        return theSeq.is(Keywords::describe_kw).thenID().matches();
      }
      return false;
    }

  protected:
    SQLProcessor *Proc;
    std::string name;
  };

  class InsertSQLStatement : public Statement
  {
  public:
    InsertSQLStatement(SQLProcessor *aProc) : Statement(Keywords::insert_kw), Proc(aProc), entity(nullptr) {}

    ~InsertSQLStatement()
    {
      Proc->dbproc->getactiveDB()->updateEntity(*entity);
      delete entity;
    }

    StatusResult parse(Tokenizer &aTokenizer)
    {
      std::string tablename = aTokenizer.peek(2).data;
      aTokenizer.next(4);
      Database *activeDB = Proc->dbproc->getactiveDB();
      if (activeDB)
      {
        entity = activeDB->getEntity(tablename);
      }
      else
      {
        return StatusResult{unknownDatabase};
      }
      if (entity)
      {
        parseKeys(aTokenizer);
        if (entity->validateAttributes(keys, valuetypes))
        {
          aTokenizer.next(1);
          if (aTokenizer.current().keyword == Keywords::values_kw)
          {
            aTokenizer.next(1);
            if (parseValueList(aTokenizer))
            {
              return StatusResult{noError};
            }
            return StatusResult{keyValueMismatch};
          }
          return StatusResult{unexpectedKeyword};
        }
        return StatusResult{unknownAttribute};
      }
      return StatusResult{unknownEntity};
    }

    int parseKeys(Tokenizer &aTokenizer)
    {
      while (aTokenizer.more() && aTokenizer.current().data != ")")
      {
        if (aTokenizer.current().type == TokenType::identifier)
        {
          keys.push_back(aTokenizer.current().data);
        }
        aTokenizer.next(1);
      }
      return keys.size();
    }

    bool parseValueList(Tokenizer &aTokenizer)
    {
      while (aTokenizer.more() && aTokenizer.current().data != ";")
      {
        std::vector<Value> theValues;
        std::vector<Token> theValueTokens;
        if (parseValues(aTokenizer, theValueTokens) != keys.size())
          return false;
        if (!Helpers::validateValues(theValues, theValueTokens, valuetypes))
          return false;
        Row *theRow = new Row(*entity, keys, theValues);
        rows.push_back(std::make_unique<Row>(*theRow));
        aTokenizer.next(1);
      }
      return true;
    }

    int parseValues(Tokenizer &aTokenizer, std::vector<Token> &aValues)
    {
      while (aTokenizer.more() && (aTokenizer.current().data != ")"))
      {
        if (aTokenizer.current().type != TokenType::punctuation)
        {
          aValues.push_back(aTokenizer.current());
        }
        aTokenizer.next(1);
      }
      return aValues.size();
    }

    StatusResult dispatch() { return Proc->insertRows(rows); }

    static bool recognizes(Tokenizer &aTokenizer)
    {
      if (aTokenizer.remaining() > 7)
      {
        TokenSequence theSeq(aTokenizer);
        return theSeq.is(Keywords::insert_kw).then(Keywords::into_kw).thenID().thenPunc("(").matches();
      }
      return false;
    }

  protected:
    SQLProcessor *Proc;
    Entity *entity;
    RowCollection rows;
    std::vector<std::string> keys;
    std::vector<DataTypes> valuetypes;
  };

  class SelectSQLStatement : public Statement
  {
  public:
    SelectSQLStatement(SQLProcessor *aProc) : Statement(Keywords::select_kw), Proc(aProc), dbquery(), join(), ifJoin(false) {}

    StatusResult parse(Tokenizer &aTokenizer)
    {
      // save the command
      dbquery.commandStr = aTokenizer.extractCommand();
      join.commandStr = aTokenizer.extractCommand();
      // EXAMPLE: SELECT * from Accounts order by first_name;  SELECT first_name, last_name from Users order by last_name;
      aTokenizer.next();
      if (aTokenizer.current().data == "*")
      {
        dbquery.setSelectAll(true);
        aTokenizer.next(2);
      }
      else
      {
        // extract attribuite tokens
        StatusResult result = parseAttributes(aTokenizer);
        if (result != StatusResult{noError})
          return result;
      }
      // set the entity for the target query
      join.left = Proc->dbproc->getactiveDB()->getEntity(aTokenizer.current().data);
      if (!join.left)
        return StatusResult{unknownTable};

      dbquery.setFrom(Proc->dbproc->getactiveDB()->getEntity(aTokenizer.current().data));
      if (dbquery.getFrom() == nullptr)
      {
        return StatusResult{unknownTable};
      }

      aTokenizer.next();
      StatusResult theResult = parseJoin(aTokenizer);
      if (!theResult)
        return theResult;

      if (ifJoin)
      {
        for (auto col : selectCols)
        {
          std::string table_name;
          std::string attribute_name;
          // Operand : table.field
          if (col.find('.') != std::string::npos)
          {
            size_t found = col.find('.');
            table_name = col.substr(0, found);
            attribute_name = col.substr(found + 1);
            if (table_name == join.left->name)
            {
              if (!join.left->getAttribute(attribute_name))
                return StatusResult{syntaxError};
            }
            else if (table_name == join.right->name)
            {
              if (!join.right->getAttribute(attribute_name))
                return StatusResult{syntaxError};
            }
            else
              return StatusResult{syntaxError};
          }
          // Operand : field
          else
          {
            attribute_name = col;
            if (join.left->getAttribute(attribute_name))
            {
              table_name = join.left->name;
            }
            else if (join.right->getAttribute(attribute_name))
            {
              table_name = join.right->name;
            }
            else
              return StatusResult{syntaxError};
          }
          join.attribute_names.push_back(attribute_name);
          join.table_names.push_back(table_name);
        }
        return StatusResult{noError};
      }

      // validate attributes and set columns into Query
      if (!dbquery.selectAll())
      {
        // if the query needs to pick some attributes, we should fetch and validate them
        if (!dbquery.getFrom()->validatesubAttributes(selectCols))
          return StatusResult{invalidAttribute};
        dbquery.setSelect(selectCols);
      }
      else
      {
        !dbquery.getFrom()->getallcols(selectCols);
        dbquery.setSelect(selectCols);
      }

      // parse the constraints: where, order by, group by, limit, offset
      while (aTokenizer.more() && aTokenizer.current().data != ";")
      {
        Keywords cur_keyword = aTokenizer.current().keyword;
        if (!theQueries.count(cur_keyword))
        {
          return StatusResult{syntaxError};
        }
        if (!(dbquery.*theQueries[cur_keyword])(aTokenizer))
          return StatusResult{syntaxError};
      }

      return StatusResult{noError};
    }

    // select (tablename.)fieldname, ... from
    StatusResult parseAttributes(Tokenizer &aTokenizer)
    {
      bool valid = false;
      while (aTokenizer.more())
      {
        while (aTokenizer.more() && aTokenizer.current().data != "," && aTokenizer.current().keyword != Keywords::from_kw)
        {
          std::string theCol;
          StatusResult result = parseField(aTokenizer, theCol);
          if (!result)
            return result;
          selectCols.push_back(theCol);
        }

        if (aTokenizer.current().keyword == Keywords::from_kw)
        {
          aTokenizer.next();
          valid = true;
          break;
        }
        // tokenizer moves forward
        aTokenizer.next();
      }

      if (!valid)
        return StatusResult{syntaxError};
      return StatusResult{noError};
    }

    StatusResult parseField(Tokenizer &aTokenizer, std::string &theField)
    {
      if (TokenType::identifier == aTokenizer.current().type)
      {
        std::string table_name = aTokenizer.current().data;
        aTokenizer.next();
        if (aTokenizer.current().data != ".")
        {
          theField = table_name;
        }
        else
        {
          aTokenizer.next();
          theField = aTokenizer.current().data;
          aTokenizer.next();
        }
        return StatusResult{noError};
      }
      return StatusResult{syntaxError};
    }

    //jointype JOIN tablename ON table1.field=table2.field
    StatusResult parseJoin(Tokenizer &aTokenizer)
    {

      if (in_array<Keywords>(gJoinTypes, aTokenizer.current().keyword))
      {
        ifJoin = true;
        join.joinType = aTokenizer.current().keyword;
        aTokenizer.next(1); //yank the 'join-type' token (e.g. left, right)
        if (aTokenizer.skipIf(Keywords::join_kw))
        {
          join.right = Proc->dbproc->getactiveDB()->getEntity(aTokenizer.current().data);
          if (!join.right)
            return StatusResult{unknownTable};
          aTokenizer.next(1);
          if (aTokenizer.skipIf(Keywords::on_kw))
          { //LHS field = RHS field
            StatusResult theResult{noError};
            Operand theLHS, theRHS;
            Logical theLogic{Logical::no_op};
            Operators theOp{Operators::equal_op};
            if (theResult = join.parseOperand(aTokenizer, theLHS))
            {
              if (aTokenizer.current().data != "=")
                return StatusResult{syntaxError};
              aTokenizer.next();
              if (theResult = join.parseOperand(aTokenizer, theRHS))
              {
                join.expr = new Expression(theLHS, theOp, theRHS, theLogic);
                aTokenizer.next();
                return StatusResult{noError};
              }
              return theResult;
            }
            return theResult;
          }
          else if (join.joinType == Keywords::cross_kw)
          {
            return StatusResult{noError};
          }
        }
        return StatusResult{unexpectedKeyword};
      }
      return StatusResult{noError};
    }

    StatusResult dispatch()
    {
      if (ifJoin)
      {
        return Proc->showJoin(join);
      }
      return Proc->showQuery(dbquery);
    }

    static bool recognizes(Tokenizer &aTokenizer)
    {
      if (aTokenizer.remaining() > 3)
      {
        TokenSequence theSeq(aTokenizer);
        return theSeq.is(Keywords::select_kw).until(Keywords::from_kw).thenID().matches();
      }
      return false;
    }

  protected:
    std::vector<std::string> selectCols;
    SQLProcessor *Proc;
    Query dbquery;
    Join join;
    bool ifJoin;
  };

  class UpdateSQLStatement : public Statement
  {
  public:
    UpdateSQLStatement(SQLProcessor *aProc) : Statement(Keywords::update_kw), Proc(aProc), dbquery() {}

    StatusResult parse(Tokenizer &aTokenizer)
    {
      aTokenizer.next();
      dbquery.setFrom(Proc->dbproc->getactiveDB()->getEntity(aTokenizer.current().data));
      if (dbquery.getFrom() == nullptr)
      {
        return StatusResult{unknownTable};
      }
      aTokenizer.next(2);

      std::vector<std::string> keys;
      std::vector<Value> values;
      StatusResult result = parseUpdate(aTokenizer, keys, values);
      if (!result)
        return result;

      // set the updated columns for the query
      dbquery.setUpdate(keys, values);

      if (!dbquery.handleWhere(aTokenizer))
        return StatusResult{syntaxError};
      return StatusResult{noError};
    }

    StatusResult parseUpdate(Tokenizer &aTokenizer, std::vector<std::string> &keys, std::vector<Value> &values)
    {
      std::vector<Token> valuetokens;
      std::vector<DataTypes> valuetypes;
      while (aTokenizer.remaining() >= 3 && aTokenizer.current().type != TokenType::keyword)
      {
        keys.push_back(aTokenizer.current().data);
        aTokenizer.next();

        if (aTokenizer.current().data != "=")
          return StatusResult{syntaxError};
        aTokenizer.next();

        valuetokens.push_back(aTokenizer.current());
        aTokenizer.next();

        aTokenizer.skipIf(',');
      }

      if (!dbquery.getFrom()->validatesubAttributes(keys, valuetypes))
        return StatusResult{invalidAttribute};

      if (!Helpers::validateValues(values, valuetokens, valuetypes))
        return StatusResult{keyValueMismatch};

      return StatusResult{noError};
    }

    StatusResult dispatch() { return Proc->updateRows(dbquery); }

    static bool recognizes(Tokenizer &aTokenizer)
    {
      if (aTokenizer.remaining() > 3)
      {
        TokenSequence theSeq(aTokenizer);
        return theSeq.is(Keywords::update_kw).thenID().then(Keywords::set_kw).matches();
      }
      return false;
    }

  protected:
    SQLProcessor *Proc;
    Query dbquery;
  };

  class DeleteSQLStatement : public Statement
  {
  public:
    DeleteSQLStatement(SQLProcessor *aProc) : Statement(Keywords::delete_kw), Proc(aProc), dbquery() {}

    StatusResult parse(Tokenizer &aTokenizer)
    {
      aTokenizer.next(2);
      dbquery.setFrom(Proc->dbproc->getactiveDB()->getEntity(aTokenizer.current().data));
      if (dbquery.getFrom() == nullptr)
      {
        return StatusResult{unknownTable};
      }
      aTokenizer.next();
      if (!dbquery.handleWhere(aTokenizer))
        return StatusResult{syntaxError};
      return StatusResult{noError};
    }

    StatusResult dispatch() { return Proc->deleteRows(dbquery); }

    static bool recognizes(Tokenizer &aTokenizer)
    {
      if (aTokenizer.remaining() > 3)
      {
        TokenSequence theSeq(aTokenizer);
        return theSeq.is(Keywords::delete_kw).then(Keywords::from_kw).thenID().matches();
      }
      return false;
    }

  protected:
    SQLProcessor *Proc;
    Query dbquery;
  };

  class AlterTableStatement : public Statement
  {
  public:
    AlterTableStatement(SQLProcessor *aProc) : Statement(Keywords::alter_kw), Proc(aProc), alterType(Keywords::unknown_kw), attribute() {}

    StatusResult parse(Tokenizer &aTokenizer)
    {
      aTokenizer.next(2);
      entity = Proc->dbproc->getactiveDB()->getEntity(aTokenizer.current().data);
      if (entity == nullptr)
      {
        return StatusResult{unknownTable};
      }
      aTokenizer.next();

      if (in_array<Keywords>(gAlterTypes, aTokenizer.current().keyword))
      {
        alterType = aTokenizer.current().keyword;
        aTokenizer.next();

        attribute.field_name = aTokenizer.current().data;
        if (alterType != Keywords::drop_kw)
        {
          aTokenizer.next();
          while (aTokenizer.more() && aTokenizer.current().data != ";")
          {
            if (aTokenizer.current().data == "default")
            {
              attribute.hasdefault = true;
              attribute.nullable = true;
              aTokenizer.next();
              attribute.defaultvalue = aTokenizer.current().data;
              aTokenizer.next();
              continue;
            }

            StatusResult result = Helpers::parseAttributeProp(aTokenizer, attribute);
            if (!result)
              return result;
          }
        }
        return StatusResult{noError};
      }
      return StatusResult{syntaxError};
    }

    StatusResult dispatch() { return Proc->alterTable(alterType, attribute, entity); }

    static bool recognizes(Tokenizer &aTokenizer)
    {
      std::vector<Keywords> alterKeywords{Keywords::add_kw, Keywords::drop_kw, Keywords::modify_kw};
      if (aTokenizer.remaining() > 5)
      {
        TokenSequence theSeq(aTokenizer);
        return theSeq.is(Keywords::alter_kw).then(Keywords::table_kw).thenID().then(alterKeywords).thenID().matches();
      }
      return false;
    }

  protected:
    SQLProcessor *Proc;
    Keywords alterType;
    Attribute attribute;
    Entity *entity;
  };

}

#endif