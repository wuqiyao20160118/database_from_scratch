//
//  Index.hpp
//  RGAssignment3
//
//  Created by rick gessner on 4/2/21.
//

#ifndef Index_hpp
#define Index_hpp

#include <stdio.h>
#include <map>
#include <map>
#include <functional>
#include "Storage.hpp"
#include "BasicTypes.hpp"
#include "Errors.hpp"

namespace ECE141
{

  enum class IndexType
  {
    intKey = 0,
    strKey
  };

  using IndexVisitor = std::function<bool(const IndexKey &, uint32_t)>;

  struct Index : public Storable, BlockIterator
  {

    Index(Storage &aStorage, uint32_t aBlockNum = 0, std::string aName = "", IndexType aType = IndexType::intKey, uint32_t anTableId = 0) : storage(aStorage), type(aType), name(aName), blockNum(aBlockNum), changed(false), tableId(anTableId) {}

    class ValueProxy
    {
    public:
      Index &index;
      IndexKey key;
      IndexType type;

      ValueProxy(Index &anIndex, uint32_t aKey)
          : index(anIndex), key(aKey), type(IndexType::intKey) {}

      ValueProxy(Index &anIndex, const std::string &aKey)
          : index(anIndex), key(aKey), type(IndexType::strKey) {}

      ValueProxy &operator=(uint32_t aValue)
      {
        index.setKeyValue(key, aValue);
        return *this;
      }

      operator IntOpt() { return index.valueAt(key); }
    }; //value proxy

    ValueProxy operator[](const std::string &aKey)
    {
      return ValueProxy(*this, aKey);
    }

    ValueProxy operator[](uint32_t aKey)
    {
      return ValueProxy(*this, aKey);
    }

    uint32_t getBlockNum() const { return blockNum; }
    Index &setBlockNum(uint32_t aBlockNum)
    {
      blockNum = aBlockNum;
      return *this;
    }

    bool isChanged() { return changed; }
    Index &setChanged(bool aChanged)
    {
      changed = aChanged;
      return *this;
    }

    StorageInfo getStorageInfo(size_t aSize)
    {
      return StorageInfo{tableId, aSize, blockNum, BlockType::index_block};
    }

    IntOpt valueAt(IndexKey &aKey)
    {
      return exists(aKey) ? data[aKey] : (IntOpt)(std::nullopt);
    }

    bool setKeyValue(IndexKey &aKey, uint32_t aValue)
    {
      data[aKey] = aValue;
      return changed = true; //side-effect indended!
    }

    StatusResult erase(const std::string &aKey)
    {
      IndexKey key = aKey;
      if (data.count(key))
      {
        data.erase(key);
      }
      changed = true;
      return StatusResult{Errors::noError};
    }

    StatusResult erase(uint32_t aKey)
    {
      IndexKey key = aKey;
      if (data.count(key))
      {
        data.erase(key);
      }
      changed = true;
      return StatusResult{Errors::noError};
    }

    size_t getSize() { return data.size(); }

    bool exists(IndexKey &aKey)
    {
      return data.count(aKey);
    }

    std::string getName()
    {
      return name;
    }

    StatusResult encode(std::ostream &anOutput) override
    {
      // entity name     how many rows this entity has     index key 1 type     index key 1    index value 1     index key 2 type     index key 2    index value 2 ......
      anOutput << name << ' ';
      anOutput << data.size() << ' ';
      for (auto thePair : data)
      {
        switch (thePair.first.index())
        {
        case 0:
          anOutput << "i " << std::get<uint32_t>(thePair.first);
          break;
        default:
          anOutput << "s " << std::get<std::string>(thePair.first);
        }
        anOutput << ' ' << thePair.second << ' ';
      }
      return StatusResult{Errors::noError};
    }

    StatusResult decode(std::istream &anInput) override
    {
      // entity name     how many rows this entity has     index key 1 type     index key 1    index value 1     index key 2 type     index key 2    index value 2 ......
      std::string theSKey;
      char theType;
      uint32_t theIKey, theValue;
      size_t theCount;

      anInput >> name;
      anInput >> theCount;
      for (size_t i = 0; i < theCount; i++)
      {
        anInput >> theType;
        IndexKey theKey;
        switch (theType)
        {
        case 'i':
          anInput >> theIKey >> theValue;
          theKey = theIKey;
          break;
        default:
          anInput >> theSKey >> theValue;
          theKey = theSKey;
        }
        data[theKey] = theValue;
      }
      return StatusResult{Errors::noError};
    }

    bool each(const BlockVisitor &aVisitor) override
    {
      Block theBlock;
      for (auto thePair : data)
      {
        if (storage.readBlock(thePair.second, theBlock))
        {
          if (!aVisitor(theBlock, thePair.second))
          {
            return false;
          }
        }
      }
      return true;
    }

    bool eachKV(IndexVisitor aCall)
    {
      for (auto thePair : data)
      {
        if (!aCall(thePair.first, thePair.second))
        {
          return false;
        }
      }
      return true;
    }

  protected:
    Storage &storage;
    std::map<IndexKey, uint32_t> data;
    IndexType type;
    std::string name;
    bool changed;
    uint32_t blockNum;
    uint32_t tableId;
  };

  using IndexMap = std::map<std::string, std::unique_ptr<Index>>;

}

#endif /* Index_hpp */
