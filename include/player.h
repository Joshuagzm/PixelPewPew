#ifndef PLAYER_H
#define PLAYER_H

#include <iostream>
#include <raylib.h>
#include <vector>
#include <cmath>
#include <thread>
#include <list>
#include "include/entity.h"
#include "include/functs.h"



/**
 * Entity derived class representing the player character(s)
 * 
 * Adds additional methods for specific movement patterns
*/
class projectileAttack;
class player;

class projectileAttack: public entity {
    public:
    projectileAttack();
    ~projectileAttack() override{}
    int damage {10};

    //methods
    int moveProjectile();

    //serialisation function
    template<class Archive> void serialize(Archive & ar, const unsigned int version)
    {
        //serialise base class
        ar & boost::serialization::base_object<entity>(*this);
    }
};

class player: public entity {
    public:
    ~player() override{}
    int initPlayer();
    //variables
    int abilityCooldown {0};
    bool animationLock {false};
    int playerIndex {-1};
    

    //class methods
    int checkMoveInput(); //checks user inputs
    int checkAttackInput(std::list<projectileAttack>* attackVector);
    int moveX(int tickCounter);
    int moveY();
    int screenBorder(int screenHeight, int screenWidth);//prevents movement outside of the screen
    int initLoop();//initialises values at beginning of loop
    int fireProjectile(std::list<projectileAttack>* attackVector);
    int spawnProjectile();

    template<class Archive> void serialize(Archive & ar, const unsigned int version)
    {
        //serialise base class
        ar & boost::serialization::base_object<entity>(*this);
    }
};



#endif 