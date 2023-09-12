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
int registerPlatform(std::deque<entity>* platformVector, float platX, float platY, float platWidth, float platHeight);
//clear platforms
int resetWorld(std::deque<entity>* platformVector, std::deque<genericEnemy>* enemyVector, std::deque<projectileAttack>* attackVector, std::unordered_map<std::pair<int, int>, gridCell>& grid);
#endif