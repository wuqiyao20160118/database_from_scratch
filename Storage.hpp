//
//  Storage.hpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#ifndef Storage_hpp
#define Storage_hpp

#include <string>
#include <fstream>
#include <iostream>
#include <deque>
#include <functional>
#include "BlockIO.hpp"
#include "Errors.hpp"

namespace ECE141
{

  struct CreateDB
  {
  }; //tags for db-open modes...
  struct OpenDB
  {
  };

  const int32_t kNewBlock = -1;

  class Storable
  {
  public:
    virtual StatusResult encode(std::ostream &anOutput) = 0;
    virtual StatusResult decode(std::istream &anInput) = 0;
  };

  struct StorageInfo
  {

    StorageInfo(size_t aRefId, size_t theSize, int32_t aStartPos = kNewBlock, BlockType aType = BlockType::data_block)
        : type(aType), start(aStartPos), refId(aRefId), size(theSize) {}

    BlockType type;
    int32_t start; //block#
    size_t refId;
    size_t size;
  };

  using BlockVisitor = std::function<bool(const Block &, uint32_t)>;
  using BlockList = std::deque<uint32_t>;

  struct BlockIterator
  {
    virtual bool each(const BlockVisitor &aVisitor) = 0;
  };

  // USE: Our storage class (for stream IO)
  class Storage : public BlockIO, public BlockIterator
  {
  public:
    Storage(std::iostream &aStream, bool useCache = false);
    ~Storage();

    StatusResult save(std::iostream &aStream, StorageInfo &anInfo);
    StatusResult load(std::iostream &aStream, uint32_t aStartBlockNum);

    bool each(const BlockVisitor &aVisitor) override;
    uint32_t getFreeBlock(); //pos of next free (or new)...

    StatusResult markBlockAsFree(uint32_t aPos);
    StatusResult releaseBlocks(uint32_t aPosm, bool keepfirst);
    StatusResult encodeavailable(std::iostream &aStream);
    StatusResult addavailable(uint32_t aPos);

  protected:
    BlockList available;
  };

}

#endif /* Storage_hpp */
