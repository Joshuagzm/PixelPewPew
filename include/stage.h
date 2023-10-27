#ifndef STAGE_H
#define STAGE_H

#include <iostream>
#include <raylib.h>
#include <vector>
#include <cmath>
#include "include/enemy.h"
#include "include/player.h"
#include "include/functs.h"
#include "include/grid.h"

//register platform
int registerPlatform(std::list<entity>* platformVector, float platX, float platY, float platWidth, float platHeight);
//clear platforms
int resetWorld(std::list<entity>* platformVector, std::list<genericEnemy>* enemyVector, std::list<projectileAttack>* attackVector, std::unordered_map<std::pair<int, int>, gridCell>& grid);

//stage manager - to track progression and transitions
//progression - kill counts, boss alive
//current stage
//stage transition protocol

class stageManager {
    private:
        screen currentStage;
        
    public:
};

#endif