//
//  Attribute.hpp
//
//  Created by rick gessner on 4/02/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#include <iostream>
#include "Attribute.hpp"

namespace ECE141
{

  Attribute::Attribute() : field_type(DataTypes::no_type), field_length(0), auto_increment(false), primary_key(false), nullable(true), hasdefault(false) {}
  Attribute::Attribute(const Attribute &anAttribute)
  {
    field_name = anAttribute.field_name;
    field_type = anAttribute.field_type;

    field_length = anAttribute.field_length;
    auto_increment = anAttribute.auto_increment;
    primary_key = anAttribute.primary_key;
    nullable = anAttribute.nullable;
    hasdefault = anAttribute.hasdefault;
    defaultvalue = anAttribute.defaultvalue;
  }

  StatusResult Attribute::encode(std::ostream &aWriter)
  {
    // name    type    length    if auto increment    if primary key    if nullable
    aWriter << field_name << " " << static_cast<char>(field_type) << " " << field_length << " " << auto_increment << " " << primary_key << " " << nullable << " ";
    return StatusResult{noError};
  }

  StatusResult Attribute::decode(std::istream &aReader)
  {
    // name    type    length    if auto increment    if primary key    if nullable
    char aType;
    aReader >> field_name >> aType >> field_length >> auto_increment >> primary_key >> nullable;
    field_type = static_cast<DataTypes>(aType);
    return StatusResult{noError};
  }

  std::string Attribute::getExtra()
  {
    std::string extra = "";
    if (auto_increment)
    {
      extra += "auto increment ";
    }
    if (primary_key)
    {
      extra += "primary key ";
    }
    return extra;
  }

}
