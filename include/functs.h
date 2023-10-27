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
#include <boost/serialization/deque.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>
#include <string>
#include <queue>
#include <deque>
#include <list>


//initialise system variables
const int screenWidth {800};
const int screenHeight {600};
const int targetFPS {60};
const std::string emptyUUID{"00000000-0000-0000-0000-000000000000"};
enum screen {EXITGAME, TITLE, LEVEL1, LEVEL1BOSS, LEVEL2, LEVEL3, WIN, DEAD, CHAT, NETCONF, DUMMY};
enum eventType {E_HIT, E_DIE, E_SCORE, E_SCREEN, E_MISC};
enum networkClass {DEFAULT, CLIENT, SERVER};
enum messageType {M_MISC, M_PLAYER, M_STR, M_PROJ, M_ENEMY, M_EVENT};
enum networkState{N_DISCONNECTED = 0, N_CONNECTING, N_CONNECTED, N_FAILED};

int getRelativeDir(float src, float dst);//gets the direction of incrementation to get from src to dst
std::vector<std::pair<int,int>> pairInterpolator(std::pair<int,int> in1, std::pair<int,int> in2);
bool isMouseInRect(Vector2 mousePos, float rectX, float rectY, Vector2 rectDimensions);

//checks if input is within bounds upper and lower (inclusive)
template <typename T> bool checkInBounds(T input, T lower, T upper)
{
    if(input > upper){
        return false;
    }
    if(input < lower){
        return false;   
    }
    return true;
}

//substraction operator but with a defined limit
template <typename T> T cappedSubtraction(T currentVal, T subtractVal, T limit){
    if(currentVal - subtractVal < limit){
        return limit;
    }else{
        return currentVal - subtractVal;
    }
}

//addition operator but with a defined limit
template <typename T> T cappedAddition(T currentVal, T addVal, T limit){
    if(currentVal + addVal > limit){
        return limit;
    }else{
        return currentVal + addVal;
    }
}

//addition that loops on defined overflow
template <typename T> T loopedAddition(T& inputValue, T addValue, T lLimit, T uLimit){
    if(inputValue + addValue > uLimit){//if overflow, loop from lowerLimit
        return(lLimit + inputValue + addValue - uLimit);
    }else{
        return(inputValue + addValue);
    }    
}



std::string getSerialisedStrHeader(uint32_t strSize, messageType msgType);
std::string getBytesFromQueue(std::deque<std::string>& stringQueue, uint32_t bytesToRead);
std::string padHeader(std::string header);


//general serialisation function
template <typename T> std::string getSerialisedStr(const T & obj)
{
    std::stringstream ss;
    boost::archive::text_oarchive oarchive(ss);
    oarchive << obj;
    return ss.str();
}

//mutable overload
template <typename T> std::string getSerialisedStr(T & obj)
{
    const T & constObj = obj;
    return getSerialisedStr(constObj);
}

//padded message header - to be padded with padHeader() after serialisation
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

//padded event message - to be padded with padHeader() after serialisation
class eventMessage
{
    public:
        eventType eType{E_MISC};
        std::string eID{emptyUUID};
        uint32_t eValue{0};

        template <class Archive> void serialize(Archive &a, const unsigned version){
            a & eType;
            a & eID;
            a & eValue;
        }
};


#endif