#include "include/stage.h"

//function to initialise a platform object
int registerPlatform(std::deque<entity>* platformVector, float platX, float platY, float platWidth, float platHeight)
{
    entity dummyPlat;
    dummyPlat.hitbox = {platX, platY, platWidth,platHeight};
    dummyPlat.alignment = entity::OBJECT;
    dummyPlat.isSolid = true;
    platformVector->push_back(dummyPlat);
    platformVector->back().updateGridOccupation();
    return 0;
}

int resetWorld(std::deque<entity>* platformVector, std::deque<genericEnemy>* enemyVector, std::deque<projectileAttack>* attackVector, std::unordered_map<std::pair<int, int>, gridCell>& grid)
{
    platformVector->clear();
    for (auto& e : *enemyVector){
        //place out of range
        e.hitbox.x = -50;
        e.hitbox.y = -50;
        //kill
        e.killEntity();
    }
    //kill all projectiles
    for (auto& a : *attackVector){
        //place out of range
        a.hitbox.x = -50;
        a.hitbox.y = -50;
        //kill
        a.killEntity();
    }
    return 0;
}