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
#include "include/stage.h"

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

 - X Player Actions (attacks)
 - Larger level and viewpoint windowing
 - X Player health and damage
 - X Enemy health and damage
*/

//things to move into functions
/*
Player functions:
 - X Keypress events
 - 

Every level:
 - X Adding platforms/objects
 - Removing platforms/objects
 - 



*/

class platform {
  public:
      int width;
      int height;
      int posX;
      int posY;
};

int randMsgIndex {0};
int stageHeight{600};
int stageWidth{1800};

bool collision;
enum screen {EXITGAME, TITLE, LEVEL1, LEVEL2, LEVEL3, WIN, DEAD, DUMMY};
screen prevState {DUMMY};
screen gameState {TITLE};
screen requestState {TITLE};

std::deque<entity> platformVector;
std::deque<projectileAttack> attackVector;
std::deque<genericEnemy> enemyVector;

bool gameReplay {true};
bool gamePaused {true};
bool gameExitRequested {false};
bool gameExitConfirmed {false};



#endif