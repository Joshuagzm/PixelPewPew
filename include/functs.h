#ifndef FUNCTS_H
#define FUNCTS_H

#define MIN(x,y) (x < y ? x : y)
#define MAX(x,y) (x > y ? x : y)

#include <boost/uuid/uuid.hpp>              //uuid class
#include <boost/uuid/uuid_generators.hpp>   //uuid generator
#include <vector>
#include <raylib.h>
//serialisation
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <string>
#include <queue>


//initialise system variables
const int screenWidth {800};
const int screenHeight {600};
const int targetFPS {60};
const std::string emptyUUID{"00000000-0000-0000-0000-000000000000"};

int getRelativeDir(float src, float dst);//gets the direction of incrementation to get from src to dst
bool checkInBounds(float input, float lower, float upper);//checks if input is within bounds upper and lower (inclusive)
std::vector<std::pair<int,int>> pairInterpolator(std::pair<int,int> in1, std::pair<int,int> in2);
bool isMouseInRect(Vector2 mousePos, float rectX, float rectY, Vector2 rectDimensions);
int cappedAddition(int currentVal, int addVal, int limit);
int cappedSubtraction(int currentVal, int subtractVal, int limit);

std::string getSerialisedStrHeader(uint32_t strSize);
std::string getBytesFromQueue(std::deque<std::string>& stringQueue, uint32_t bytesToRead);
std::string padHeader(std::string header);

enum screen {EXITGAME, TITLE, LEVEL1, LEVEL2, LEVEL3, WIN, DEAD, CHAT, NETCONF, DUMMY};
enum networkClass {DEFAULT, CLIENT, SERVER};
enum messageType {M_ENTITY, M_STR, M_MISC};

class messageHeader
{
    public:
        std::string headerUUID {emptyUUID};
        uint32_t bodySize {0};
        messageType msgType {M_MISC};
    
        template <class Archive>
        void serialize(Archive &a, const unsigned version){
            a & headerUUID;
            a & bodySize;
            a & msgType;
        }
};


#endif