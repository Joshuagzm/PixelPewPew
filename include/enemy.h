#ifndef ENEMY_H
#define ENEMY_H

#include <iostream>
#include <raylib.h>
#include <vector>
#include <cmath>
#include <thread>
#include "include/functs.h"
#include "include/entity.h"

class genericEnemy: public entity
{
    protected:
        int dirX = 0;
        int dirY = 0;
        int jumpTimer = 0;
        int jumpTimerThresh = 90;
        int jumpHeight = -15;

    public:
        int ID;
        int updateMovement(float playerX, float playerY);
};

//enemy spawning handler
class enemyDirector: public entity
{
    public:
        //variables
        int spawnTimer;
        int spawnThresh;
        int currentID;

        //methods
        genericEnemy spawnCommand();

};

#endif