#ifndef ENTITY_H
#define ENTITY_H

#include <iostream>
#include <raylib.h>
#include <vector>
#include <cmath>
#include <thread>
#include "include/functs.h"
#include "include/grid.h"
#include <set>
#include <algorithm>
#include <string>

//uuid generation
#include <boost/uuid/uuid.hpp>              //uuid class
#include <boost/uuid/uuid_generators.hpp>   //uuid generator
#include <boost/uuid/uuid_io.hpp>

/**
 * Entity super class
 * 
 * Provides attributes for movement and positional properties
 * Provides methods for movement and collision events
 * 
*/

//custom comparator
bool comparePairs(const std::pair<int,int>& setA, const std::pair<int,int>& setB);
//grid sets
using pairSetType = std::set<std::pair<int,int>, decltype(comparePairs)*>;

class entity {
    protected:
        boost::uuids::uuid entityID {};

    public:
        entity();
        enum directionX {left = -1, right = 1};
        //geometry and position
        float prevX {0};
        float prevY {0};
        int width {0};
        int height {0};
        int halfheight {0};
        int mass {10};
        Rectangle hitbox;

        //movement and status
        bool isAlive = true;//when false, delete the object
        int jumpMax {1};
        int jumpStock {0};
        int speedX {0};
        int speedY {0};
        int baseSpeed {5};        
        bool yLock {false};
        bool xLock {false};
        int gravity {2};

        //collision status
        bool isSolid {false};
        bool isTouchDamage {false};

        int faceDirectionX {left};//integer to indicate facing direction in the X direction
        int faceDirectionY {0};//integer to indicate facing direction in the Y direction (unused maybe)

        //collision utilities
        Rectangle boxCollision; //represents the collision intersect area
        int collisionInt;

        //class methods
        int applyGravity(int gravity);
        int collisionHandler(entity * plat);
        int collisionOrder(float * x, float * y);

        //set speed methods
        int setSpeedY(int modifier);
        int setSpeedX(int modifier);

        //check entity methods
        bool checkInSquare();

        //check current grid occupation
        int updateGridOccupation();
        pairSetType gridCellsCurrent{&comparePairs};
        std::vector<entity *> checkCloseEntities();
        //related hitboxes

};

#endif