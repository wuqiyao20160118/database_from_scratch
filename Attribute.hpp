//
//  Attribute.hpp
//
//  Created by rick gessner on 4/02/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#ifndef Attribute_hpp
#define Attribute_hpp

#include <stdio.h>
#include <string>
#include "Storage.hpp"
#include "keywords.hpp"
#include "BasicTypes.hpp"

namespace ECE141
{

  class Attribute : public Storable
  {
  public:
    std::string field_name;
    DataTypes field_type;

    int field_length;
    bool auto_increment;
    bool primary_key;
    bool nullable;
    bool hasdefault;
    Value defaultvalue;

    //STUDENT: declare ocf methods...
    Attribute();
    Attribute(const Attribute &anAttribute);
    ~Attribute(){};

    //What methods do you need to interact with Attributes?

    //Added so that the attribute is a storable...
    StatusResult encode(std::ostream &aWriter) override;
    StatusResult decode(std::istream &aReader) override;

    std::string getExtra();
  };

}

#endif /* Attribute_hpp */
