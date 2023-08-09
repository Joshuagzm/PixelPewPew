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