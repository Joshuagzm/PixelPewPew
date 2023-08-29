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

//serialisation
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>

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
        //physical properties
        enum directionX {left = -1, right = 1}; //facing direction
        float prevX {0};                        //previous position X
        float prevY {0};                        //previous position Y
        int width {0};                          //UNUSED width
        int height {0};                         //UNUSED height
        int halfheight {0};                     //precalculated value half of the height
        int halfWidth {0};                      //precalculated value half of width
        int mass {10};                          //"mass"
        Rectangle hitbox;                       //hitbox, used for appearance and collisions
        Color entColor{WHITE};                  //entity color
        int faceDirectionX {left};              //integer to indicate facing direction in the X direction
        int faceDirectionY {0};                 //integer to indicate facing direction in the Y direction (unused maybe)

        //movement and status
        bool isAlive = true;    //when false, delete the object
        int jumpMax {1};        //max jumps
        int jumpStock {0};      //jumps available
        int speedX {0};         //internal movement speed in x direction
        int speedY {0};         //internal movement speed in y direction
        int inertiaX {0};       //externally caused decaying movement speed in x direction
        int inertiaY {0};       //externally caused decaying movement speed in y direction
        int inertiaDecay {1};   //inertia decay
        int baseSpeed {5};      //internal base movement speed
        bool yLock {false};     //locks y movement
        bool xLock {false};     //locks x movemnet
        int gravity {2};        //gravitational acceleration

        int hp {0};                     //current hp
        int maxHp{0};                   //max hp
        int invulnerableTimer{0};       //invulnerable time remaining
        bool isInvulnerable {false};    //invulnerable status

        //collision status
        bool isSolid {false};            //can entities pass through
        bool isTouchDamage {false};     //will the player take touch damage 

        //alignment
        enum entityAlignment {PLAYER = 0, MONSTER = 1, NEUTRAL = 2, OBJECT = 3};
        entityAlignment alignment {NEUTRAL};
        
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
        bool checkInSquare(int* squareWidth, int* squareHeight);
        bool checkInTopless(int* squareWidth, int* squareHeight);

        //check current grid occupation
        int updateGridOccupation();
        pairSetType gridCellsCurrent{&comparePairs};
        std::vector<entity *> checkCloseEntities();
        int removeGridOccupation(pairSetType cellsToRemove);
        int addGridOccupation(pairSetType cellsToAdd);
        void killEntity();
        
        //serialisation
        template <class Archive>
        void serialize(Archive &a, const unsigned version){
            std::string strEntityID{boost::uuids::to_string(entityID)};
            a & strEntityID; 
            a & hitbox.x;
            a & hitbox.y;
            a & hitbox.width;
            a & hitbox.height;
        }
        std::string getSerialisedEntity();
        std::string getSerialisedEntityHeader(uint32_t bodySize);
};

#endif