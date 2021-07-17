#include "TabularView.hpp"
#include <sstream>
#include "Config.hpp"

namespace ECE141
{

    StatusResult TabularView::setcolumns(StringList theCols)
    {
        for (std::string theCol : theCols)
        {
            if (theCol != "_id")
                columns.push_back(theCol);
        }
        // columns.assign(theCols.begin(), theCols.end());
        return StatusResult{noError};
    }

    StatusResult TabularView::setRowCollection(RowCollection &theRows)
    {
        for (auto &theRow : theRows)
        {
            rows.push_back(std::move(theRow));
        }
        return StatusResult{noError};
    }

    std::string TabularView::buildBar()
    {
        std::stringstream ss;
        for (size_t i = 0; i < columns.size(); i++)
        {
            ss << "+";
            for (int j = 0; j < 31; j++)
            {
                ss << "-";
            }
        }
        ss << "+\n";
        return ss.str();
    }

    bool TabularView::show(std::ostream &anOutput)
    {
        int theCount = 0;
        std::stringstream theOut;
        std::string theBar = buildBar();
        theOut << theBar;
        // show the selected columns
        for (size_t i = 0; i < columns.size(); i++)
        {
            theOut << "| " << std::left << std::setw(30) << columns[i];
        }
        theOut << "|\n";

        theOut << theBar;
        // iterate through rows
        for (auto &theRow : rows)
        {
            std::vector<std::string> output_value;
            // collect the value
            theRow->getSelectColumns(output_value, columns);
            // validate the extracted value
            if (output_value.size() != columns.size())
                return false;
            // show the value
            for (size_t i = 0; i < columns.size(); i++)
            {
                theOut << "| " << std::left << std::setw(30) << output_value[i];
            }
            theOut << "|\n";
            theCount += 1;
        }
        theOut << theBar;

        theOut << theCount << " rows in set ";
        anOutput << theOut.str();
        if (viewCache != nullptr)
            viewCache->put(viewCommand, theOut.str());
        // timer->stop();
        //anOutput << theCount << " rows in set (" << timer->elapsed() << " secs)\n";

        return true;
    }

}
