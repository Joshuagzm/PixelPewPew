#ifndef HEAD_H
#define HEAD_H

//BUGS TO FIX:
/*
 - WEIRD STICKING TO CORNERS OF PLATFORMS
 - Not proper clearing of entities after screen change


*/

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

//avoid naming conflicts with windows functions
#if defined(_WIN32)           
	#define NOGDI             // All GDI defines and routines
	#define NOUSER            // All USER defines and routines
#endif

#include "include/asio_test.h"

#if defined(_WIN32)           // raylib uses these names as function parameters
	#undef near
	#undef far
#endif

////

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
 - X Larger level and viewpoint windowing
 - X Player health and damage
 - X Enemy health and damage

 - Level exit gate
 - Enemy variety
 - Even larger map
 - Healing/Upgrades
 - Better enemy spawning

 - Fall off the map and come back
 - Multiplayer

*/

//things to move into functions
/*
Player functions:
 - X Keypress events

Every level:
 - X Adding platforms/objects
 - Removing platforms/objects
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
screen requestState {DUMMY};

std::deque<entity> platformVector;
std::deque<projectileAttack> attackVector;
std::deque<genericEnemy> enemyVector;
std::deque<entity*> playerVector;

bool gameReplay {true};
bool gamePaused {true};
bool gameExitRequested {false};
bool gameExitConfirmed {false};

#endif