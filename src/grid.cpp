#include "include/grid.h"

//function to take y position and find the row of the point
int getCurrentRow(int yPos)
{
    return (static_cast<int>(yPos*rowFactor));
}


//function to take x position and find the column of the point
int getCurrentCol(int xPos)
{
    return (static_cast<int>(xPos*colFactor));
}

void clearGrid(std::unordered_map<std::pair<int, int>, gridCell>& grid)
{
    //clear hitboxes from the grid
    for(auto& cell : grid)
    {
        //clear contents of each cell (second)
        std::cout<<cell.second.size()<<" Pre\n";
        cell.second.clear();
        std::cout<<cell.second.size()<<" Post\n\n";
    }
}