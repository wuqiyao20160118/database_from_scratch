#ifndef EntityView_h
#define EntityView_h

#include <sstream>
#include <iomanip>
#include "View.hpp"
#include "Storage.hpp"

namespace ECE141
{

  class EntityView : public View
  {
  public:
    EntityView(Storage &aStorage, Timer *aTimer) : storage(aStorage), timer(aTimer) {}

    bool show(std::ostream &anOutput) override
    {
      std::string theBar = "+-----------------+\n";
      size_t theCount = 0;
      anOutput << theBar;
      anOutput << "+ Tables          +\n"
               << theBar;
      storage.each([&](const Block &theBlock, uint32_t blockNum)
                   {
                     if (theBlock.header.type == 'E')
                     {
                       Entity theEntity;
                       std::stringstream ss;
                       ss.write(theBlock.payload, kPayloadSize);
                       theEntity.decode(ss);
                       anOutput << "| " << std::left << std::setw(16) << theEntity.name << "|\n";
                       anOutput << theBar;
                       theCount++;
                     }
                     return true;
                   });
      timer->stop();
      anOutput << theCount << " rows in set (" << timer->elapsed() << "sec)\n";
      return true;
    }

    Storage &storage;
    Timer *timer;
  };

  class DescribeView : public View
  {
  public:
    DescribeView(Entity *anEntityPtr, Timer *aTimer) : entityptr(anEntityPtr), timer(aTimer) {}

    bool show(std::ostream &anOutput) override
    {
      std::string theBar = "+---------------------+--------------+------+-----+---------+-----------------------------+\n";
      size_t theCount = 0;
      anOutput << theBar;
      anOutput << "| Field               | Type         | Null | Key | Default | Extra                       |\n";
      anOutput << theBar;

      for (auto attribute : entityptr->attributes)
      {
        if (attribute.field_name == "_id")
          continue;
        anOutput << "| " << std::left << std::setw(20) << attribute.field_name;
        anOutput << "| " << std::left << std::setw(13) << Helpers::dataTypeToString(attribute.field_type);
        anOutput << "| " << std::left << std::setw(5) << (attribute.nullable ? "YES" : "NO");
        anOutput << "| " << std::left << std::setw(4) << (attribute.primary_key ? "YES" : "   ");
        if (attribute.hasdefault)
        {
          anOutput << "| " << std::left << std::setw(8) << Helpers::valueToString(attribute.defaultvalue);
        }
        else
        {
          anOutput << "| " << std::left << std::setw(8) << "NULL";
        }
        anOutput << "| " << std::left << std::setw(28) << attribute.getExtra() << "|\n";
        theCount++;
      }
      anOutput << theBar;

      timer->stop();
      anOutput << theCount << " rows in set (" << std::fixed << std::setprecision(5) << timer->elapsed() << " secs)\n";
      return true;
    }

  private:
    Entity *entityptr;
    Timer *timer;
  };

}

#endif