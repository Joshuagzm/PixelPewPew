#include "include/functs.h"
#include <iostream>

int getRelativeDir(float src, float dst)
{
    if(src < dst){
        return 1;
    }else{
        return -1;
    }
}

bool checkInBounds(float input, float lower, float upper)
{
    if(input > upper){
        return false;
    }
    if(input < lower){
        return false;   
    }
    return true;
}

std::vector<std::pair<int,int>> pairInterpolator(std::pair<int,int> in1, std::pair<int,int> in2)
{
    //find direction of traversal
    int xStart {0};
    int xEnd {0};
    int yStart {0};
    int yEnd {0};

    std::vector<std::pair<int,int>> gridListCurrent;
    std::pair<int,int> gridCell;

    //to account for all orientation of inputs, sort the inputs in ascending order
    if (in1.first < in2.first){
        xStart  = in1.first; 
        xEnd    = in2.first;
    }else{
        xStart  = in2.first; 
        xEnd    = in1.first;
    }

    if (in1.second < in2.second){
        yStart  = in1.second; 
        yEnd    = in2.second;
    }else{
        yStart  = in2.second; 
        yEnd    = in1.second;
    }

    for (int i = xStart; i <= xEnd; i++){
        for (int j = yStart; j <= yEnd; j++){
            gridCell = {i,j};
            gridListCurrent.push_back(gridCell);
        }
    }
    return gridListCurrent; 
}

bool isMouseInRect(Vector2 mousePos, float rectX, float rectY, Vector2 rectDimensions)
{
    if( mousePos.x > rectX && mousePos.x < (rectX + rectDimensions.x) &&
        mousePos.y > rectY && mousePos.y < (rectY + rectDimensions.y) )
    {
        return true;
    }
    return false;

    
}