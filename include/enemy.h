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
        ~genericEnemy() override{}
        int ID;
        virtual int updateMovement(float playerX, float playerY);
        int updateBossMovement(float playerX, float playerY);
        void slimeBossAI();
        int onHit() override;
        void onDeath() override;


        //serialisation function
        template<class Archive> void serialize(Archive & ar, const unsigned int version)
        {
            //serialise base class
            ar & boost::serialization::base_object<entity>(*this);
            ar & isSolid;
            ar & isTouchDamage;
            ar & alignment;
        }
};

class slime : public genericEnemy
{
    public:
        ~slime() override{}
        // void onHit() override;
        // void onDeath() override;
        //serialisation function
        template<class Archive> void serialize(Archive & ar, const unsigned int version)
        {
            //serialise base class
            ar & boost::serialization::base_object<genericEnemy>(*this);
        }
};

class bossSlime : public genericEnemy
{
    public:
        ~bossSlime() override{}
        int updateMovement(float playerX, float playerY) override;

        //serialisation function
        template<class Archive> void serialize(Archive & ar, const unsigned int version)
        {
            //serialise base class
            ar & boost::serialization::base_object<genericEnemy>(*this);
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
        genericEnemy spawnSlimeBoss();
};

#endif