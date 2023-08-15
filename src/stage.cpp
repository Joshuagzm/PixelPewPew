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

int resetWorld(std::deque<entity>* platformVector, std::deque<genericEnemy>* enemyVector, std::deque<projectileAttack>* attackVector)
{
    platformVector->clear();
    enemyVector->clear();
    attackVector->clear();
    return 0;
}