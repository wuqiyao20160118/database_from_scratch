#ifndef TokenSequence_h
#define TokenSequence_h

#include "Tokenizer.hpp"

namespace ECE141
{

  class TokenSequence
  {
  public:
    TokenSequence(Tokenizer &aTokenizer) : tokenizer(aTokenizer), match(false), offset(0) {}

    TokenSequence &is(Keywords aKW)
    {
      match = tokenizer.current().keyword == aKW;
      return *this;
    }

    TokenSequence &then(Keywords aKW)
    {
      if (match)
      {
        Token &theNext = tokenizer.peek(++offset);
        match = theNext.keyword == aKW;
      }
      return *this;
    }

    TokenSequence &then(std::vector<Keywords> KWs)
    {
      if (match)
      {
        Token &theNext = tokenizer.peek(++offset);
        match = false;
        if (std::find(KWs.begin(), KWs.end(), theNext.keyword) != KWs.end())
          match = true;
      }
      return *this;
    }

    TokenSequence &thenID()
    {
      if (match)
      {
        Token &theNext = tokenizer.peek(++offset);
        match = theNext.type == TokenType::identifier;
      }
      return *this;
    }

    TokenSequence &thenPunc(const std::string &aString)
    {
      if (match)
      {
        Token &theNext = tokenizer.peek(++offset);
        match = aString == theNext.data;
      }
      return *this;
    }

    TokenSequence &until(Keywords aKW)
    {
      if (match)
      {
        while (offset + 1 < tokenizer.remaining())
        {
          if (aKW == tokenizer.peek(++offset).keyword)
          {
            return *this;
          }
        }
        match = false;
      }
      return *this;
    }

    void clear()
    {
      match = false;
      offset = 0;
    }

    bool matches() { return match; }

  protected:
    Tokenizer &tokenizer;
    int offset;
    bool match;
  };

}

#endif