//
//  Config.hpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#ifndef Config_h
#define Config_h
#include <sstream>
#include <map>
#include "Cache.hpp"

struct Config
{

  static const char *getDBExtension() { return ".db"; }

  static const char *getStoragePath()
  {

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

    //STUDENT: If you're on windows, return a path to folder on your machine...
    return std::filesystem::temp_directory_path();

#elif __APPLE__ || defined __linux__ || defined __unix__

    return "/tmp"; //MAC, UNIX, LINUX here...

#endif
  }

  static std::string getDBPath(const std::string &aDBName)
  {
    std::ostringstream theStream;
    theStream << Config::getStoragePath() << "/" << aDBName << ".db";
    return theStream.str();
  }
};

#endif /* Config_h */
