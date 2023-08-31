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


        //serialisation function
        template<class Archive> void serialize(Archive & ar, const unsigned int version)
        {
            //serialise base class
            ar & boost::serialization::base_object<entity>(*this);
        }
};

//enemy spawning handler
class enemyDirector: public entity
{
    public:
        //variables
        int spawnTimer;
        int spawnThresh;
        int currentID;
        bool isRunning {false};

        //methods
        genericEnemy spawnCommand();
        void tickUpdate(std::deque<genericEnemy>& enemyList);

};

#endif