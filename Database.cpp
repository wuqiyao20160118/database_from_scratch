//
//  Database.cpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#include <string>
#include <iostream>
#include <map>
#include "Storage.hpp"
#include "Database.hpp"
#include "Config.hpp"
#include "Entity.hpp"
#include "Index.hpp"

namespace ECE141
{
  Database::Database(const std::string aName, CreateDB, bool useCache)
      : name(aName), storage(stream, useCache), changed(true)
  {
    std::string thePath = Config::getDBPath(name);
    stream.clear(); // Clear Flag, then create file...
    stream.open(thePath.c_str(), std::fstream::binary | std::fstream::in | std::fstream::out | std::fstream::trunc);
    stream.close();
    stream.open(thePath.c_str(), std::fstream::binary | std::fstream::binary | std::fstream::in | std::fstream::out);

    //STUDENT: Write meta info to block 0...
    std::stringstream ss;
    ss << 0 << " ";
    ss << 0 << " ";
    ss << 0 << " ";
    StorageInfo storageinfo(0, (size_t)ss.tellp(), 0, BlockType::meta_block);
    storage.save(ss, storageinfo);

    if (mUseCache["Row"])
      rowCache = new LFUCache<std::string, std::vector<Row>>(10240, "", std::vector<Row>());
    if (mUseCache["View"])
      viewCache = new LFUCache<std::string, std::string>(10, "", "");
  }

  Database::Database(const std::string aName, OpenDB, bool useCache) : name(aName), changed(false), storage(stream, useCache)
  {

    std::string thePath = Config::getDBPath(name);
    stream.open(thePath.c_str(), std::fstream::binary | std::fstream::in | std::fstream::out);

    //STUDENT: Load meta info from block 0
    std::stringstream ss;
    storage.load(ss, 0);
    size_t theCount;

    ss >> theCount;
    for (size_t i = 0; i < theCount; i++)
    {
      std::string key;
      uint32_t value;
      ss >> key >> value;
      entityindex[key] = value;
    }

    ss >> theCount;
    for (size_t i = 0; i < theCount; i++)
    {
      std::string key;
      uint32_t value;
      ss >> key >> value;
      indexindex[key] = value;
    }

    ss >> theCount;
    for (size_t i = 0; i < theCount; i++)
    {
      uint32_t value;
      ss >> value;
      storage.addavailable(value);
    }

    if (mUseCache["Row"])
      rowCache = new LFUCache<std::string, std::vector<Row>>(10240, "", std::vector<Row>());
    if (mUseCache["View"])
      viewCache = new LFUCache<std::string, std::string>(10, "", "");
  }

  Database::~Database()
  {
    if (changed)
    {
      //save db meta information to block 0?
      //save anything else in memory that changed...
      std::stringstream ss;
      ss << entityindex.size() << ' ';
      for (auto thePair : entityindex)
      {
        ss << thePair.first << ' ';
        ss << thePair.second << ' ';
      }
      ss << indexindex.size() << ' ';
      for (auto thePair : indexindex)
      {
        ss << thePair.first << ' ';
        ss << thePair.second << ' ';
      }
      storage.encodeavailable(ss);
      storage.releaseBlocks(0, true);
      StorageInfo storageinfo(0, (size_t)ss.tellp(), 0, BlockType::meta_block);
      storage.save(ss, storageinfo);
    }
    if (mUseCache["Row"])
      delete rowCache;
    if (mUseCache["View"])
      delete viewCache;
  }

  StatusResult Database::encode(std::ostream &anOutput)
  {
    return StatusResult{Errors::noError};
  }

  StatusResult Database::decode(std::istream &anInput)
  {
    return StatusResult{Errors::noError};
  }

  StatusResult Database::addEntity(Entity &anEntity)
  {
    anEntity.blockNum = storage.getFreeBlock();
    Block entityblock(BlockType::entity_block);
    std::stringstream ss1;
    anEntity.encode(ss1);
    ss1.read(entityblock.payload, kPayloadSize);
    storage.writeBlock(anEntity.blockNum, entityblock);

    changed = true;
    entityindex[anEntity.name] = anEntity.blockNum;

    std::string index_name = anEntity.primaryKeyName;
    Index index(storage, storage.getFreeBlock(), index_name, IndexType::intKey, Entity::hashString(anEntity.name));
    std::stringstream ss2;
    index.encode(ss2);
    StorageInfo storageinfo = index.getStorageInfo((size_t)ss2.tellp());
    storage.save(ss2, storageinfo);

    indexindex[anEntity.name] = index.getBlockNum();

    return StatusResult{noError};
  }

  StatusResult Database::updateEntity(Entity &anEntity)
  {
    Block entityblock(BlockType::entity_block);
    std::stringstream ss;
    anEntity.encode(ss);
    ss.read(entityblock.payload, kPayloadSize);
    return storage.writeBlock(anEntity.blockNum, entityblock);
  }

  StatusResult Database::dropTable(const std::string &aName, int &rownum)
  {
    if (entityindex.count(aName))
    {
      Index index(storage, indexindex[aName], "", IndexType::intKey, Entity::hashString(aName));
      std::stringstream ss;
      storage.load(ss, indexindex[aName]);
      index.decode(ss);

      storage.markBlockAsFree(entityindex[aName]);
      storage.releaseBlocks(indexindex[aName], true);
      rownum = 0;
      index.eachKV([&](const IndexKey &key, uint32_t value)
                   {
                     storage.markBlockAsFree(value);
                     rownum++;
                     return true;
                   });

      changed = true;
      entityindex.erase(aName);
      indexindex.erase(aName);
      return StatusResult{noError};
    }
    return StatusResult{unknownTable};
  }

  bool Database::insertRows(const RowCollection &aRows)
  {
    Index index(storage, indexindex[aRows[0]->getTableName()], "", IndexType::intKey, Entity::hashString(aRows[0]->getTableName()));
    std::stringstream ss1;
    storage.load(ss1, indexindex[aRows[0]->getTableName()]);
    index.decode(ss1);

    for (int i = 0; i < aRows.size(); i++)
    {
      aRows[i]->setblockNumber(storage.getFreeBlock());
      Block rowblock;
      rowblock.header.refId = Entity::hashString(aRows[i]->getTableName());
      std::stringstream ss;
      aRows[i]->encode(ss);
      ss.read(rowblock.payload, kPayloadSize);
      storage.writeBlock(aRows[i]->getblockNumber(), rowblock);

      std::string index_name = index.getName();
      IndexKey key = std::get<int>((aRows[i]->getData())[index_name]);
      index.setKeyValue(key, aRows[i]->getblockNumber());
    }

    storage.releaseBlocks(indexindex[aRows[0]->getTableName()], true);
    std::stringstream ss2;
    index.encode(ss2);
    StorageInfo storageinfo = index.getStorageInfo((size_t)ss2.tellp());
    storage.save(ss2, storageinfo);

    return true;
  }

  bool Database::selectRows(const std::string &aName, RowCollection &aRows)
  {
    if (rowCache != nullptr && rowCache->contains(aName))
    {
      std::vector<Row> theRows = rowCache->get(aName);
      for (auto theRow : theRows)
      {
        aRows.push_back(std::unique_ptr<Row>(new Row(theRow)));
      }
      return true;
    }
    if (!indexindex.count(aName))
      return false;
    Index index(storage, indexindex[aName], "", IndexType::intKey, Entity::hashString(aName));
    std::stringstream ss;
    storage.load(ss, indexindex[aName]);
    index.decode(ss);

    std::vector<Row> theRows;

    index.each([&](const Block &theBlock, uint32_t blockNum)
               {
                 Row *aRowPtr = new Row(aName);
                 std::stringstream ss;
                 ss.write(theBlock.payload, kPayloadSize);
                 aRowPtr->decode(ss);
                 if (mUseCache["Row"])
                   theRows.push_back(*aRowPtr);
                 aRows.push_back(std::unique_ptr<Row>(aRowPtr));
                 return true;
               });
    if (mUseCache["Row"])
    {
      rowCache->put(aName, theRows);
    }
    return true;
  }

  int Database::deleteRows(const Query &aQuery)
  {
    Index index(storage, indexindex[aQuery.getFrom()->name], "", IndexType::intKey, Entity::hashString(aQuery.getFrom()->name));
    std::stringstream ss1;
    storage.load(ss1, indexindex[aQuery.getFrom()->name]);
    index.decode(ss1);

    std::vector<int> todelete;
    index.each([&](const Block &theBlock, uint32_t blockNum)
               {
                 Row aRow(aQuery.getFrom()->name);
                 std::stringstream ss;
                 ss.write(theBlock.payload, kPayloadSize);
                 aRow.decode(ss);
                 if (aQuery.matches(aRow))
                 {
                   storage.markBlockAsFree(blockNum);
                   std::string index_name = index.getName();
                   todelete.push_back(std::get<int>((aRow.getData())[index_name]));
                 }
                 return true;
               });

    for (auto key : todelete)
    {
      index.erase(key);
    }

    if (index.isChanged())
    {
      storage.releaseBlocks(indexindex[aQuery.getFrom()->name], true);
      std::stringstream ss2;
      index.encode(ss2);
      StorageInfo storageinfo = index.getStorageInfo((size_t)ss2.tellp());
      storage.save(ss2, storageinfo);
    }

    // invalidate the cache (can be optimized)
    if (mUseCache["Row"])
    {
      delete rowCache;
      rowCache = new LFUCache<std::string, std::vector<Row>>(10240, "", std::vector<Row>());
    }
    if (mUseCache["View"])
    {
      viewCache = new LFUCache<std::string, std::string>(10, "", "");
      delete viewCache;
    }

    return todelete.size();
  }

  int Database::updateRows(Query &aQuery)
  {
    Index index(storage, indexindex[aQuery.getFrom()->name], "", IndexType::intKey, Entity::hashString(aQuery.getFrom()->name));
    std::stringstream ss1;
    storage.load(ss1, indexindex[aQuery.getFrom()->name]);
    index.decode(ss1);

    std::vector<std::string> &updatekeys = aQuery.getUpdatekeys();
    std::vector<Value> &updatevalues = aQuery.getUpdatevalues();

    int updaterownum = 0;

    index.each([&](const Block &theBlock, uint32_t blockNum)
               {
                 Row aRow(aQuery.getFrom()->name);
                 std::stringstream ss3;
                 ss3.write(theBlock.payload, kPayloadSize);
                 aRow.decode(ss3);
                 if (aQuery.matches(aRow))
                 {
                   updaterownum++;
                   for (int i = 0; i < updatekeys.size(); i++)
                   {
                     aRow.setKV(updatekeys[i], updatevalues[i]);
                   }
                   Block rowblock;
                   rowblock.header.refId = Entity::hashString(aRow.getTableName());
                   std::stringstream ss4;
                   aRow.encode(ss4);
                   ss4.read(rowblock.payload, kPayloadSize);
                   storage.writeBlock(aRow.getblockNumber(), rowblock);
                 }
                 return true;
               });

    if (index.isChanged())
    {
      storage.releaseBlocks(indexindex[aQuery.getFrom()->name], true);
      std::stringstream ss2;
      index.encode(ss2);
      StorageInfo storageinfo = index.getStorageInfo((size_t)ss2.tellp());
      storage.save(ss2, storageinfo);
    }

    // invalidate the cache (can be optimized)
    if (mUseCache["Row"])
    {
      delete rowCache;
      rowCache = new LFUCache<std::string, std::vector<Row>>(10240, "", std::vector<Row>());
    }
    if (mUseCache["View"])
    {
      viewCache = new LFUCache<std::string, std::string>(10, "", "");
      delete viewCache;
    }

    return updaterownum;
  }

  bool Database::hasEntity(const std::string &aName)
  {
    return entityindex.count(aName);
  }

  Entity *Database::getEntity(const std::string &aName)
  {
    Entity *aEntityPtr = nullptr;
    if (entityindex.count(aName))
    {
      Block block;
      storage.readBlock(entityindex[aName], block);
      aEntityPtr = new Entity();
      std::stringstream ss;
      ss.write(block.payload, kPayloadSize);
      aEntityPtr->decode(ss);
    }
    return aEntityPtr;
  }

}
