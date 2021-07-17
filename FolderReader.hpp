#ifndef FolderReader_h
#define FolderReader_h

#include <string>
#include <functional>
#include <filesystem>

namespace fs = std::filesystem;

namespace ECE141 {

  using FileVisitor = std::function<bool(const std::string &)>;

  class FolderReader {
  public:
            FolderReader(const char *aPath): path(aPath) {}
    virtual ~FolderReader() {}

    virtual void each(const std::string &anExt, const FileVisitor &aVisitor) {
      for (auto & theItem : fs::directory_iterator(path)) {
        if (!theItem.is_directory()) {
          fs::path temp(theItem.path());
          std::string theExt(temp.extension());
          if (0==anExt.size() || 0==anExt.compare(theExt)) {
            if (!aVisitor(temp.stem())) break;
          }
        }
      }
    }
    
    std::string path;
  };
}

#endif
