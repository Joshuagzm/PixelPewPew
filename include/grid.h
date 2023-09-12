#ifndef GRID_H
#define GRID_H

#include <raylib.h>
#include <unordered_map>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <vector>
#include <string>
#include "functs.h"
#include <boost/functional/hash.hpp>
#include <iostream>


//forward declarations
class entity;

namespace std {
    template <>
    struct hash<boost::uuids::uuid> {
        size_t operator()(const boost::uuids::uuid& uuid) const {
            std::size_t seed = 0;
            boost::hash_combine(seed, uuid);
            return seed;
        }
    };
}

//CUSTOM HASH FUNCTION
namespace std {
    template <>
    struct hash<std::pair<int,int>> {
        size_t operator()(const std::pair<int,int>& p) const {
            return std::hash<int>()(p.first) ^ std::hash<int>()(p.second);
        }
    };
}

//header file containing the functions and structure declarations for the grid layout of the screen
//key: entity ID, value: hitbox of entity
typedef std::unordered_map<boost::uuids::uuid, entity*> gridCell;
//key: pair representing the coordinates, value: map of ID'd entities within the cell
extern std::unordered_map<std::pair<int, int>, gridCell> gridContainer;

//What do we need, the grid dimensions and spacing
const int gridRows {5};
const int gridCols {5};

const float rowFactor {static_cast<float>(gridRows)/static_cast<float>(screenWidth)};        
const float colFactor {static_cast<float>(gridCols)/static_cast<float>(screenHeight)};

int getCurrentRow(int yPos);
int getCurrentCol(int xPos); 

void clearGrid(std::unordered_map<std::pair<int, int>, gridCell>& grid);

#endif