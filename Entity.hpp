//
//  Entity.hpp
//
//  Created by rick gessner on 4/03/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#ifndef Entity_hpp
#define Entity_hpp

#include <stdio.h>
#include <vector>
#include <optional>
#include <memory>
#include <string>

#include "Attribute.hpp"
#include "BasicTypes.hpp"
#include "Errors.hpp"
#include "Storage.hpp"

namespace ECE141
{

  using AttributeOpt = std::optional<Attribute>;
  using AttributeList = std::vector<Attribute>;

  //------------------------------------------------

  class Entity : public Storable
  {
  public:
    static uint32_t hashString(const char *str);
    static uint32_t hashString(const std::string &aStr);

    //declare ocf methods...
    Entity(std::string aname = "");
    ~Entity();

    //declare methods you think are useful for entity...
    bool addAttribute(Attribute &anAttribute);
    bool removeAttribute(Attribute &anAttribute);
    bool modifyAttribute(Attribute &anAttribute);
    bool validateAttributes(std::vector<std::string> &keys, std::vector<DataTypes> &valuetypes);
    bool validatesubAttributes(std::vector<std::string> &keys);
    bool validatesubAttributes(std::vector<std::string> &keys, std::vector<DataTypes> &valuetypes);

    int getid()
    {
      return id;
    }

    void incrementid()
    {
      id++;
    }

    Attribute *getAttribute(std::string aName);
    bool getallcols(std::vector<std::string> &keys);

    //this is the storable interface...
    StatusResult encode(std::ostream &aWriter) override;
    StatusResult decode(std::istream &aReader) override;

    AttributeList attributes;
    std::string name;
    uint32_t blockNum;
    int id;
    std::string primaryKeyName;

    //surely there must be other data members?
  };

}
#endif /* Entity_hpp */
