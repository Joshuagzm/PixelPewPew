#include "include/entity.h"

boost::uuids::random_generator generator;
std::unordered_map<std::pair<int, int>, gridCell> gridContainer;

//custom comparator
bool comparePairs(const std::pair<int,int>& setA, const std::pair<int,int>& setB)
{
    if(setA.first != setB.first){
        return(setA.first < setB.first);
    }else{
        return(setA.second < setB.second);
    }
}

//default constructor
entity::entity()
{
    //uuid generator
    entityID = generator();
    // this->updateGridOccupation();
}

//sets speedY
int entity::setSpeedY(int modifier)
{
    this->speedY = modifier;
    return 0;
}

int entity::setSpeedX(int modifier)
{
    this->speedX = modifier;
    return 0;
}

int entity::applyGravity(int gravity){
    if(!this->yLock){
        this->speedY = MIN(this->speedY + gravity,static_cast<float>(23));
    }else{
        if(this->jumpStock < this->jumpMax) this->jumpStock = this->jumpMax;
        this->speedY = 0;
    }
    return 0;
}

int entity::collisionOrder(float * x, float * y)
{
    //resolve the smaller error first in attempt to minimise the number of resolutions
    if(*x < *y){
        return 1;
    }else{
        return 0;
    }
}

int entity::collisionHandler(entity * obj){
    //Base: check if there is a collision between the object and the next position
    //Generate list of rectangles to compare against 
    std::vector<Rectangle> rectProjections;

    int maxAttempts = 100;
    int attempt = 0;
    bool didCollide = false;
    bool entity_collision = CheckCollisionRecs(obj->hitbox, this->hitbox);
    int dir = 1;
    //if there is, move the hitbox back towards the previous position

    while(entity_collision)
    {
        attempt += 1;
        didCollide = true;
        //check for fast teleporty movements
        //find the direction of movement (current x,y - prev x,y)
        boxCollision = GetCollisionRec(obj->hitbox, this->hitbox);
        if(boxCollision.width <= 0 || boxCollision.height <= 0){
            return 1;
        }

        collisionInt = collisionOrder(&boxCollision.width, &boxCollision.height);

        //stationary case handler
        switch(collisionInt)
        {
            case(0):
                if(this->hitbox.y == this->prevY){
                    collisionInt = 1;
                }
                break;

            case(1):
                if(this->hitbox.x == this->prevX){
                    collisionInt = 0;
                }
                break;

        }
        

        switch(collisionInt)
        {
            //resolve y axis
            case(0):
                this->hitbox.y > this->prevY ? dir = -1 : dir = 1;
                this->hitbox.y = this->hitbox.y + (boxCollision.height)*dir ;
                // this->yLock = true;
                this->speedY = 0;
                if(this->jumpStock < this->jumpMax) this->jumpStock = this->jumpMax;
                break;

            //resolve x axis
            case(1):
                this->hitbox.x > this->prevX ? dir = -1 : dir = 1;                
                this->hitbox.x = this->hitbox.x + (boxCollision.width+1)*dir;
                this->xLock = true;
                this->speedX = 0;
                break;

            default:
                std::cout<<"COLLISION ORDER FAILED"<<std::endl;
        }
        entity_collision = CheckCollisionRecs(obj->hitbox, this->hitbox);
        //queue operations: X movement and Y movement
        //execute smallest operation first
        //move x,y back to exit collision

        //give up after x attempts
        if(attempt >= maxAttempts){
            break;
        }
    }

    if (!entity_collision){
        this->yLock = false;
        this->xLock = false;
        this->speedX = 5;
    }

    if(didCollide)
    {
        return 1;
    }else{
        return 0;
    }


}

bool entity::checkInSquare()
{
    //function to check if an entity is within a square (usually the screen)
    return checkInBounds(this->hitbox.x, float(0), float(screenWidth)) && checkInBounds(this->hitbox.y,float(0),float(screenHeight));

}

//Grid managmeent


//function to look at self and check what grid locations are being occupied
int entity::updateGridOccupation()
{
    pairSetType gridCellsOld = this->gridCellsCurrent;
    pairSetType cellsToRemove {&comparePairs};
    pairSetType cellsToAdd {&comparePairs};
    //obtain the grid location of two opposing corners
    std::pair<int,int> topLeft = {
        getCurrentCol(this->hitbox.x),
        getCurrentRow(this->hitbox.y)
    };
    std::pair<int,int> bottomRight = {
        getCurrentCol(this->hitbox.x + this->hitbox.width),
        getCurrentRow(this->hitbox.y + this->hitbox.height)
    };
    //interpolate the grid spaces covered in between
    this->gridCellsCurrent.clear();
    for (const auto& coord: pairInterpolator(topLeft,bottomRight))
    {
        this->gridCellsCurrent.insert(coord);
    }
    
    //find cells to de-register from
    std::set_difference(gridCellsOld.begin(), gridCellsOld.end(),
                        gridCellsCurrent.begin(), gridCellsCurrent.end(),
                        std::inserter(cellsToRemove, cellsToRemove.begin()));

    //de-register cells
    for (const auto& cell : cellsToRemove){
        //search for cell in gridContainer with the key {cell.first, cell.second}
        //remove the entry in this cell with the ID this->entityID
        if(auto itGridCell = gridContainer.find(cell); itGridCell != gridContainer.end()){
            auto& coordMembers = itGridCell->second;
            if(auto itCoordMembers = coordMembers.find(this->entityID); itCoordMembers != coordMembers.end()){
                coordMembers.erase(itCoordMembers);
            }
        }else{
            //raise exception
        }
        //if entityID is not found, then something here is broken and raise exception
    }


    //find cells to register to
    std::set_difference(gridCellsCurrent.begin(), gridCellsCurrent.end(),
                        gridCellsOld.begin(), gridCellsOld.end(),
                        std::inserter(cellsToAdd, cellsToAdd.begin()));

    //register cells
    for (const auto& cell : cellsToAdd){
        if(auto itGridCell = gridContainer.find(cell); itGridCell != gridContainer.end()){
            itGridCell->second.insert(std::make_pair(this->entityID, this));

            //DEBUG print all entities within currently occupied cells
            /*
            std::cout<<"ENTITIES IN PLAYER CELLS: \n";
            for (auto& ent : itGridCell->second){
                std::cout << boost::uuids::to_string(ent.first)<<std::endl;
            }std::cout<<std::endl;
            */
        }else{
            //raise exception
        }
    }
    return 0;
}

std::vector<entity *> entity::checkCloseEntities()
{
    std::vector<entity *> closeEntities;
    //check grid cells for entities that are NOT self
    for (const auto& cell : gridCellsCurrent){
        //search through the grid to find an ID that matches the cell coords
        if(auto itGridCell = gridContainer.find(cell); itGridCell != gridContainer.end()){
            //loop through the cell contents
            for (const auto& coordMember : itGridCell->second){
                //Check if the hitbox exists, if not, then append it
                if (std::find(closeEntities.begin(), closeEntities.end(), coordMember.second) == closeEntities.end()
                    && coordMember.first != this->entityID){
                    closeEntities.push_back(coordMember.second);
                }
            }
        }
    }
    return closeEntities;
}
