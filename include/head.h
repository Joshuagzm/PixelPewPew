#ifndef HEAD_H
#define HEAD_H

#include <iostream>
#include <raylib.h>
#include <vector>
#include <cmath>
#include <thread>
#include <unordered_map>
#include "include/enemy.h"
#include "include/player.h"
#include "include/functs.h"
#include "include/grid.h"

//Ideas! 
/*
Slime character!

Platformer combination of Kirby and Risk of Rain and Vampire Survivors?
 -> Item based progression
 -> Items obtained by "eating" certain elites (eating from full HP or as an execute or from eating their souls)
Co-op aspect like Lovers in a Dangerous Spacetime
 -> Weapons can be used manually by sub-players
 -> Manually controlled weapons get significantly powered up
Maybe some vampire survivors aspects

Boss ideas
 -> Utilise Raid-like mechanics (for multiplayer)
   - Stacks
   - Positions
   - Objective protection
   - Split objectives

 -> Similar ideas (for single player)
   - Safe zones
   - Danger Zones
   - Objective protection


TODO: 
 - X Basic character movement framework (consider movement upgrades, weight penalties, movement modes)
 - X Basic world + collisions
 - X Entity creator -> make the player and enemies inherit from entity
 - X Enemy generation
 - X Ground enemy

 - Player Actions (attacks and animations)
 - Larger level and viewpoint windowing
 - Player health and damage
 - Enemy health and damage
*/

class platform {
  public:
      int width;
      int height;
      int posX;
      int posY;
};

std::vector<entity> platform_vector;
std::vector<projectileAttack> attackVector;
std::vector<genericEnemy> enemyVector;
bool collision;
enum screen {EXITGAME, TITLE, LEVEL1, LEVEL2, LEVEL3};
screen gameState {TITLE};

bool gameReplay {true};
bool gamePaused {true};
bool gameExitRequested {false};
bool gameExitConfirmed {false};

#endif