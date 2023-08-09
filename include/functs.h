#ifndef FUNCTS_H
#define FUNCTS_H

#define MIN(x,y) (x < y ? x : y)
#define MAX(x,y) (x > y ? x : y)

#include <boost/uuid/uuid.hpp>              //uuid class
#include <boost/uuid/uuid_generators.hpp>   //uuid generator
#include <vector>
#include <raylib.h>

//initialise system variables
const int screenWidth = 800;
const int screenHeight = 600;
const int targetFPS = 60;

int getRelativeDir(float src, float dst);//gets the direction of incrementation to get from src to dst
bool checkInBounds(float input, float lower, float upper);//checks if input is within bounds upper and lower (inclusive)
std::vector<std::pair<int,int>> pairInterpolator(std::pair<int,int> in1, std::pair<int,int> in2);
bool isMouseInRect(Vector2 mousePos, float rectX, float rectY, Vector2 rectDimensions);


#endif