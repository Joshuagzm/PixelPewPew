#ifndef HEAD_H
#define HEAD_H

#include <iostream>
#include <raylib.h>
#include <vector>
#include <cmath>
#include <thread>
#include <unordered_map>
#include <boost/circular_buffer.hpp>
#include "include/enemy.h"
#include "include/player.h"
#include "include/functs.h"
#include "include/grid.h"
#include "include/stage.h"
#include "include/text.h"
#include <queue>
#include <mutex>
#include <string>

#include "include/asio_test.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/deque.hpp>

class platform {
  public:
      int width;
      int height;
      int posX;
      int posY;
};

int tickCounter {0};
int randMsgIndex {0};
int stageHeight{600};
int stageWidth{1800};
const double textMeasureFactor {3.0};
const uint32_t headerPadSize{10};

bool collision;
networkClass peerType {DEFAULT};
screen prevState {DUMMY};
screen gameState {TITLE};
screen requestState {DUMMY};

std::deque<entity> platformVector;
std::deque<projectileAttack> attackVector;
std::deque<genericEnemy> enemyVector;
std::deque<entity*> playerVector;

//tracking
std::deque<genericEnemy>::iterator enemyIt;

//vectors for external object (multiplayer) - no collision handling (yet)
std::deque<projectileAttack> extAttackVector;
std::deque<entity*> extPlayerVector;

bool gameReplay {true};
bool gamePaused {true};
bool gameExitRequested {false};
bool gameExitConfirmed {false};

//screen variables
std::string ipAddrStr{""};
bool inputSelected{false};
std::string textInput{""};
//Communication state and reading variables
int readingState {0};
uint32_t headerSize{0};
uint32_t bodySize{0};

//thread-shared data
networkState nState {N_DISCONNECTED};
std::mutex nStateMutex;
std::deque<std::string> receivedDataQueue;
std::mutex queueMutex;

//Level module declarations
void level1(player* protag, int* killCount, int* winCount, Camera2D* camera);
void levelDead();
void levelMainMenu();
void levelWin();
void levelNetConf(int& runMode, std::string& ipAddrStr, bool& inputSelected);
void levelChat(std::string& latestMessage, bool& inputSelected, std::string& inputString, 
                int chatHistoryLength, boost::circular_buffer<std::string>& chatBuffer, networkInstance& networkHandler);
void createInputBox(Rectangle inputBox, Color& borderColor, Color& fillColor, Color& textColor, int textSize, Vector2 mousePos, std::string& tempString, bool& isSelected );

#endif


////
//Ideas! 
/*

TODO: 
 - Enemy projectile handling
 - Interactables
 - Enemy HP indicator
 - Level exit gate
 - Enemy variety
 - Even larger map
 - Healing/Upgrades
 - Better enemy spawning
 - Fall off the map and come back

DONE:
 - Basic character movement framework (consider movement upgrades, weight penalties, movement modes)
 - Basic world + collisions
 - Entity creator -> make the player and enemies inherit from entity
 - Enemy generation
 - Ground enemy
 - Player Actions (attacks)
 - Larger level and viewpoint windowing
 - Player health and damage
 - Enemy health and damage
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

//BUGS TO FIX:
/*
 - WEIRD STICKING TO CORNERS OF PLATFORMS > make platforms bigger for easier testing
 - Enemy hitboxes not registering until movement
 - Not proper clearing of entities or entity properites after screen change

 - Not resetting network state and things on return to title
 - Can't switch between server and client
FIXED
 - Jump count resets on hitting platform from underneath
 - 

DONE:
 - Improve enemy template to support multiple types and HP 

TO INVESTIGATE:
 - Entity->getSerialisedStr breaks serialisation of polymorphic classes when called from a derived class. Why? 


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

*/
