#include "include/functs.h"
#include <iostream>

int getRelativeDir(float src, float dst)
{
    if(src < dst){
        return 1;
    }else{
        return -1;
    }
}

std::vector<std::pair<int,int>> pairInterpolator(std::pair<int,int> in1, std::pair<int,int> in2)
{
    //find direction of traversal
    int xStart {0};
    int xEnd {0};
    int yStart {0};
    int yEnd {0};

    std::vector<std::pair<int,int>> gridListCurrent;
    std::pair<int,int> gridCell;

    //to account for all orientation of inputs, sort the inputs in ascending order
    if (in1.first < in2.first){
        xStart  = in1.first; 
        xEnd    = in2.first;
    }else{
        xStart  = in2.first; 
        xEnd    = in1.first;
    }

    if (in1.second < in2.second){
        yStart  = in1.second; 
        yEnd    = in2.second;
    }else{
        yStart  = in2.second; 
        yEnd    = in1.second;
    }

    for (int i = xStart; i <= xEnd; i++){
        for (int j = yStart; j <= yEnd; j++){
            gridCell = {i,j};
            gridListCurrent.push_back(gridCell);
        }
    }
    return gridListCurrent; 
}

bool isMouseInRect(Vector2 mousePos, float rectX, float rectY, Vector2 rectDimensions)
{
    if( mousePos.x > rectX && mousePos.x < (rectX + rectDimensions.x) &&
        mousePos.y > rectY && mousePos.y < (rectY + rectDimensions.y) )
    {
        return true;
    }
    return false;

    
}

//function to get the serialised header of a string message
std::string getSerialisedStrHeader(uint32_t strSize, messageType msgType)
{
    std::stringstream ss;
    boost::archive::text_oarchive oarchive(ss);
    messageHeader strHeader;
    strHeader.bodySize = strSize;
    strHeader.msgType = msgType;
    oarchive << strHeader;
    std::string paddedHeader{padHeader(ss.str())};
    return paddedHeader;
}

//function to append header with a 10 character 0 padded string indicating size of the header
std::string padHeader(std::string header)
{
    //return size appended string
    std::string headerSize{std::to_string(header.size())};
    std::string headerSizePad{""};
    //pad to get 10 characters max
    while(headerSize.size() + headerSizePad.size() < 10){
        headerSizePad += "0";
    }
    return headerSizePad + headerSize + header;
}

//function to read a certain number of bytes from a queue
std::string getBytesFromQueue(std::deque<std::string>& stringQueue, uint32_t bytesToRead)
{
    std::string readString;

    //loop through the queue
    while(!stringQueue.empty() && bytesToRead > 0)
    {
        //find the size of the front of the queue
        std::string& frontString = stringQueue.front();

        if(frontString.size() <= bytesToRead)
        {
            readString += frontString;
            bytesToRead -= frontString.size();
            stringQueue.pop_front();
            //read the entirety of the string
        }else if(frontString.size() > bytesToRead){ //the string is bigger than what we need, so we only read part of it
            readString += frontString.substr(0, bytesToRead); //read remainingBytes from the frontString, reassigning it to be the unread remainder of the string
            frontString = frontString.substr(bytesToRead);
            bytesToRead = 0;            
        }
    }

    if(bytesToRead <= 0){
        return readString;
    }else{//not enough data, body or header incomplete
        stringQueue.push_front(readString);
        return "";
    }
}