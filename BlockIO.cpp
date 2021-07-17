//
//  BlockIO.cpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#include "BlockIO.hpp"
#include "string.h"

namespace ECE141
{

  Block::Block(BlockType aType) : header(aType)
  {
  }

  Block::Block(const Block &aCopy)
  {
    this->header = aCopy.header;
    memcpy(this->payload, aCopy.payload, kPayloadSize);
  }

  Block &Block::operator=(const Block &aCopy)
  {
    this->header = aCopy.header;
    memcpy(this->payload, aCopy.payload, kPayloadSize);
    return *this;
  }

  StatusResult Block::write(std::ostream &aStream)
  {
    return StatusResult{Errors::noError};
  }

  //---------------------------------------------------

  BlockIO::BlockIO(std::iostream &aStream, bool useCache) : stream(aStream), useCache(useCache)
  {
    cache = new LFUCache<int, Block>(1024, -1, Block(BlockType::unknown_block));
  }

  BlockIO::~BlockIO()
  {
    delete cache;
  }

  StatusResult write(Block &aBlock, std::iostream &aStream, size_t aBlockSize)
  {
    if (aStream.write((char *)&aBlock, aBlockSize))
    {
      aStream.flush();
      return StatusResult{noError};
    }
    return StatusResult{writeError};
  }

  // USE: write data a given block (after seek) ---------------------------------------
  StatusResult BlockIO::writeBlock(uint32_t aBlockNum, Block &aBlock)
  {
    static size_t theSize = sizeof(aBlock);
    stream.seekg(stream.tellg(), std::ios::beg); //sync buffers...
    stream.seekp(aBlockNum * theSize);
    StatusResult theResult = write(aBlock, stream, theSize);
    if (theResult && useCache)
    {
      cache->put(aBlockNum, aBlock);
    }
    return theResult;
  }

  // USE: write data a given block (after seek) ---------------------------------------
  StatusResult BlockIO::readBlock(uint32_t aBlockNumber, Block &aBlock)
  {
    if (!useCache || !cache->contains(aBlockNumber))
    {
      static size_t theSize = sizeof(aBlock);
      stream.seekg(aBlockNumber * theSize);
      //size_t thePos=stream.tellg();
      if (!stream.read((char *)&aBlock, theSize))
      {
        return StatusResult(readError);
      }
      if (useCache)
      {
        cache->put(aBlockNumber, aBlock);
      }
    }
    else
    {
      aBlock = cache->get(aBlockNumber);
    }

    return StatusResult{noError};
  }

  // USE: count blocks in file ---------------------------------------
  uint32_t BlockIO::getBlockCount()
  {
    stream.seekg(stream.tellg(), std::ios::beg); //force read mode; dumb c++ issue...
    stream.seekg(0, std::ios::end);
    int thePos = (int)stream.tellg();
    return thePos / sizeof(Block);
  }

}
