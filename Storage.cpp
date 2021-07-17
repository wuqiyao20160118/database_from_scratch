//
//  Storage.cpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#include <sstream>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <cstring>
#include "Storage.hpp"
#include "Config.hpp"

namespace ECE141
{

  // USE: ctor ---------------------------------------
  Storage::Storage(std::iostream &aStream, bool useCache) : BlockIO(aStream, useCache)
  {
  }

  // USE: dtor ---------------------------------------
  Storage::~Storage()
  {
  }

  bool Storage::each(const BlockVisitor &aVisitor)
  {
    int blocknum = getBlockCount();
    for (int blockid = 0; blockid < blocknum; blockid++)
    {
      Block aBlock;
      StatusResult result = readBlock(blockid, aBlock);
      if (!result)
        return false;
      if (!aVisitor(aBlock, blockid))
        return false;
    }
    return true;
  }

  //write logic to get the next available free block
  uint32_t Storage::getFreeBlock()
  {
    if (available.size() == 0)
      return getBlockCount();

    uint32_t ret = available.front();
    available.pop_front();
    return ret;
  }

  //write logic to mark a block as free...
  StatusResult Storage::markBlockAsFree(uint32_t aPos)
  {
    Block theBlock;
    StatusResult result = readBlock(aPos, theBlock);
    if (!result)
      return result;
    theBlock.header.type = static_cast<char>(BlockType::free_block);

    result = writeBlock(aPos, theBlock);
    if (!result)
      return result;
    available.push_back(aPos);
    return StatusResult{Errors::noError};
  }

  // USE: for use with storable API...
  //   Write logic to mark a sequence of blocks as free)
  //   starting at given block number, following block sequence
  //   defined in the block headers...
  StatusResult Storage::releaseBlocks(uint32_t aPos, bool keepfirst)
  {
    std::vector<uint32_t> freelist;
    do
    {
      Block block;
      writeBlock(aPos, block);
      freelist.push_back(aPos);
      // markBlockAsFree(aPos);
      aPos = block.header.next;
    } while (aPos != 0);

    if (keepfirst)
    {
      for (int i = 1; i < freelist.size(); i++)
      {
        markBlockAsFree(freelist[i]);
      }
    }
    else
    {
      for (int i = 0; i < freelist.size(); i++)
      {
        markBlockAsFree(freelist[i]);
      }
    }

    return StatusResult{noError};
  }

  StatusResult Storage::encodeavailable(std::iostream &aStream)
  {
    aStream << available.size() << ' ';
    for (int i = 0; i < available.size(); i++)
    {
      aStream << available[i] << ' ';
    }
    return StatusResult{noError};
  }

  StatusResult Storage::addavailable(uint32_t aPos)
  {
    available.push_back(aPos);
    return StatusResult{noError};
  }

  //Write logic to break stream into N parts, that fit into
  //a series of blocks. Save each block, and link them together
  //logically using data in the header...
  StatusResult Storage::save(std::iostream &aStream, StorageInfo &anInfo)
  {
    aStream.seekg(0, std::ios::beg);
    uint8_t blockCount = (anInfo.size - 1) / kPayloadSize + 1;
    uint32_t now = anInfo.start;

    for (uint8_t i = 0; i < blockCount; i++)
    {
      Block block(anInfo.type);
      block.header.count = blockCount;
      block.header.pos = i + 1;
      if (block.header.pos < block.header.count)
      {
        block.header.next = getFreeBlock();
      }
      block.header.refId = anInfo.refId;
      aStream.read(block.payload, kPayloadSize);
      writeBlock(now, block);
      now = block.header.next;
    }

    return StatusResult{noError};
  }

  //Write logic to read an ordered sequence of N blocks, back into
  //a stream for your caller
  StatusResult Storage::load(std::iostream &anOut, uint32_t aBlockNum)
  {
    anOut.clear();
    do
    {
      Block block;
      readBlock(aBlockNum, block);
      anOut.write(block.payload, kPayloadSize);
      aBlockNum = block.header.next;
    } while (aBlockNum != 0);

    return StatusResult{noError};
  }

}
