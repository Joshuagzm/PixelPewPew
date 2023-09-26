#ifndef ENEMY_H
#define ENEMY_H

#include <iostream>
#include <raylib.h>
#include <vector>
#include <cmath>
#include <thread>
#include <list>
#include "include/functs.h"
#include "include/entity.h"

enum enemyType {ET_SLIME, ET_SLIMEBOSS, ET_ACCELPROJ_H, ET_MISC};
class enemyDirector;

class genericEnemy: public entity
{
    public:
        genericEnemy(enemyDirector* enemyDir) :
            enemyDir(enemyDir){};
        genericEnemy(){};
        ~genericEnemy() override{}
        int ID;
        virtual int updateMovement(float playerX, float playerY);
        void slimeMovement(float playerX, float playerY);
        void slimeBossMovement(float playerX, float playerY);
        void accelProjHMovement();
        void slimeBossAI();
        int onHit() override;
        void onDeath() override;
        enemyType eType{ET_SLIME};
        //probably private
        int moveState{0};//big state
        int intState{0};//intermediate state
        int timerGoal{0};//a timer goal

        //serialisation function
        template<class Archive> void serialize(Archive & ar, const unsigned int version)
        {
            //serialise base class
            ar & boost::serialization::base_object<entity>(*this);
            ar & isSolid;
            ar & isTouchDamage;
            ar & alignment;
        }

    protected:
        int dirX = 0;
        int dirY = 0;
        int jumpTimer = 0;
        int jumpTimerThresh = 90;
        int jumpHeight = -15;
        enemyDirector* enemyDir;
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
        bossSlime(enemyDirector* enemyDir) :
        genericEnemy(enemyDir)
        {
            jumpTimerThresh = 90;//jump every 1.5 seconds give or take a second
            eType = ET_SLIMEBOSS;
        }

        int updateMovement(float playerX, float playerY) override;
        genericEnemy createShockwave();
        


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
        enemyDirector(std::list<genericEnemy>* enemyList) : 
        enemyList(enemyList)
        {
            if (!enemyList) {
                throw std::invalid_argument("enemy list cannot be nullptr");
            }
        };
        //variables
        int spawnTimer;
        int spawnThresh;
        int currentID;
        bool isRunning {false};
        int spawnableWidth {1};
        int spawnableHeight {1};

        //methods
        genericEnemy spawnCommand();
        void tickUpdate();
        bossSlime spawnSlimeBoss();
        void spawnGenericEnemy(genericEnemy enemy);
    
    private:
        std::list<genericEnemy>* enemyList;

};

#endif