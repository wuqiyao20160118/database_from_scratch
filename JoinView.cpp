#include "JoinView.hpp"
#include <sstream>

namespace ECE141
{

    std::string JoinView::buildBar()
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

    StatusResult JoinView::settables(StringList theTables)
    {
        tables.assign(theTables.begin(), theTables.end());
        return StatusResult{noError};
    }

    StatusResult JoinView::setcols(StringList theCols)
    {
        for (std::string theCol : theCols)
        {
            if (theCol != "_id")
                columns.push_back(theCol);
        }
        //columns.assign(theCols.begin(), theCols.end());
        return StatusResult{noError};
    }

    StatusResult JoinView::setRowPairCollection(RowPairCollection &theRowPairs)
    {
        for (auto &theRowPair : theRowPairs)
        {
            rowpairs.push_back(std::move(theRowPair));
        }
        return StatusResult{noError};
    }

    bool JoinView::show(std::ostream &anOutput)
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

        // iterate through row pairs
        for (auto &theRowPair : rowpairs)
        {
            StringList output_value;
            // collect the value
            theRowPair->getSelectColumns(output_value, columns, tables);

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

        return true;
    }

}