//
//  FolderView.hpp
//  Assignment2
//
//  Created by rick gessner on 2/15/21.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef FolderView_h
#define FolderView_h


#include "View.hpp"
#include "FolderReader.hpp"
#include "iomanip"

namespace ECE141 {

  class FolderView : public View {
  public:
    FolderView(const char *aPath, const char *anExtension="db", Timer* aTimer=nullptr): reader(aPath), extension(anExtension), timer(aTimer) {}
   
    bool show(std::ostream &anOutput) override {
      std::string theBar = "+-----------------+\n";
      size_t theCount = 0;
      anOutput << theBar;
      anOutput << "+ Databases       +\n" << theBar;
      reader.each(extension,[&](const std::string &aName) {
        anOutput << "| " << std::left << std::setw(16) << aName << "|\n";
        anOutput << theBar;
        theCount++;
        return true;
      });
      timer->stop();
      anOutput << theCount << " rows in set (" << timer->elapsed() << " sec)\n";
      return true;
    }
    
    FolderReader  reader;
    const char    *extension;
    Timer         *timer;
  };

}

#endif /* FolderView_h */
