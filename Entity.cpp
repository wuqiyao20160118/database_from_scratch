//
//  Entity.cpp
//
//  Created by rick gessner on 4/03/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#include <sstream>
#include "Entity.hpp"

namespace ECE141
{

  //STUDENT: Implement the Entity class here...

  const int gMultiplier = 37;

  //hash given string to numeric quantity...
  uint32_t Entity::hashString(const char *str)
  {
    uint32_t h{0};
    unsigned char *p;
    for (p = (unsigned char *)str; *p != '\0'; p++)
      h = gMultiplier * h + *p;
    return h;
  }

  uint32_t Entity::hashString(const std::string &aStr)
  {
    return hashString(aStr.c_str());
  }

  //OCF...
  Entity::Entity(std::string aname) : name(aname), blockNum(0), id(1), primaryKeyName("_id") {}
  Entity::~Entity() {}

  //other entity methods...
  bool Entity::addAttribute(Attribute &aAttribute)
  {
    if (aAttribute.primary_key)
      primaryKeyName = aAttribute.field_name;
    attributes.push_back(aAttribute);
    return true;
  }

  bool Entity::removeAttribute(Attribute &anAttribute)
  {
    if (anAttribute.primary_key)
      return false;
    // remove the attribute
    int idx = -1;
    for (int i = 0; i < attributes.size(); i++)
    {
      auto &attribute = attributes[i];
      if (attribute.field_name == anAttribute.field_name)
      {
        idx = i;
        break;
      }
    }
    if (idx < 0)
      return false;
    for (int i = idx; i < attributes.size() - 1; i++)
    {
      attributes[i] = attributes[i + 1];
    }
    attributes.pop_back();
    return true;
  }

  bool Entity::modifyAttribute(Attribute &anAttribute)
  {
    for (size_t i = 0; i < attributes.size(); i++)
    {
      if (attributes[i].field_name == anAttribute.field_name)
      {
        attributes[i] = Attribute(anAttribute);
        return true;
      }
    }
    return false;
  }

  bool Entity::validateAttributes(std::vector<std::string> &keys, std::vector<DataTypes> &valuetypes)
  {
    std::vector<std::string> nonnullables;
    std::unordered_map<std::string, DataTypes> nullable_map, nonnullable_map;
    std::unordered_map<std::string, bool> key_map;
    for (size_t i = 0; i < attributes.size(); i++)
    {
      if (attributes[i].nullable)
      {
        nullable_map[attributes[i].field_name] = attributes[i].field_type;
      }
      else
      {
        nonnullable_map[attributes[i].field_name] = attributes[i].field_type;
        nonnullables.push_back(attributes[i].field_name);
      }
    }
    for (auto key : keys)
    {
      key_map[key] = true;
    }

    for (auto key : keys)
    {
      if ((nullable_map.count(key) > 0))
      {
        valuetypes.push_back(nullable_map[key]);
        continue;
      }
      if (nonnullable_map.count(key) > 0)
      {
        valuetypes.push_back(nonnullable_map[key]);
        continue;
      }
      return false;
    }
    for (auto nonnullable : nonnullables)
    {
      if (key_map.count(nonnullable))
      {
        continue;
      }
      if (getAttribute(nonnullable)->auto_increment)
        continue;
      return false;
    }

    return true;
  }

  bool Entity::validatesubAttributes(std::vector<std::string> &keys)
  {
    std::unordered_map<std::string, bool> attribute_map;
    for (size_t i = 0; i < attributes.size(); i++)
    {
      attribute_map[attributes[i].field_name] = true;
    }

    for (auto key : keys)
    {
      if ((attribute_map.count(key) > 0))
      {
        continue;
      }
      return false;
    }

    return true;
  }

  bool Entity::validatesubAttributes(std::vector<std::string> &keys, std::vector<DataTypes> &valuetypes)
  {
    std::unordered_map<std::string, DataTypes> attribute_map;
    for (size_t i = 0; i < attributes.size(); i++)
    {
      attribute_map[attributes[i].field_name] = attributes[i].field_type;
    }

    for (auto key : keys)
    {
      if ((attribute_map.count(key) > 0))
      {
        valuetypes.push_back(attribute_map[key]);
        continue;
      }
      return false;
    }

    return true;
  }

  Attribute *Entity::getAttribute(std::string aName)
  {
    for (auto &attribute : attributes)
    {
      if (attribute.field_name == aName)
      {
        return &attribute;
      }
    }
    return nullptr;
  }

  bool Entity::getallcols(std::vector<std::string> &keys)
  {
    for (size_t i = 0; i < attributes.size(); i++)
    {
      keys.push_back(attributes[i].field_name);
    }
    return true;
  }

  StatusResult Entity::encode(std::ostream &aWriter)
  {
    // name    how many attributes    the highest id this entity give to a row     block number    attribute1    attribute2 ....

    aWriter.clear();
    aWriter << name << " " << attributes.size() << " " << id << " " << blockNum << " ";
    for (auto attribute : attributes)
    {
      attribute.encode(aWriter);
    }
    return StatusResult{noError};
  }

  StatusResult Entity::decode(std::istream &aReader)
  {
    // name    how many attributes    the highest id this entity give to a row     block number    attribute1    attribute2 ....
    int num;
    aReader >> name;
    aReader >> num;
    aReader >> id;
    aReader >> blockNum;
    Attribute it;
    for (int i = 0; i < num; i++)
    {
      it.decode(aReader);
      attributes.push_back(it);
    }
    return StatusResult{noError};
  }

}
