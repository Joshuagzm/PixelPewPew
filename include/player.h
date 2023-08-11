#ifndef PLAYER_H
#define PLAYER_H

#include <iostream>
#include <raylib.h>
#include <vector>
#include <cmath>
#include <thread>
#include "include/entity.h"

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
    int damage {10};

    //methods
    int moveProjectile();
};

class player: public entity {
    public:
    int initPlayer();
    //variables
    int abilityCooldown {0};
    bool animationLock {false};
    

    //class methods
    int moveX();
    int moveY();
    int screenBorder(int screenHeight, int screenWidth);//prevents movement outside of the screen
    int initLoop();//initialises values at beginning of loop
    int fireProjectile(std::vector<projectileAttack>* attackVector);
    int spawnProjectile();
};



#endif 