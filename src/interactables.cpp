#include "include/interactables.h"

//"floating" type movement
void stageDoor::updateMovement()
{
    //change direction
    if(frameCount == 0){
        if(isMoving){
            //stop moving
            isMoving = false;
        }else{
            //start moving in the opposite direction
            isMoving = true;
            faceDirectionY *= -1;
        }
    }

    //move 12 times a second
    if(frameCount % 5 == 0 && isMoving){
        hitbox.y += faceDirectionY;
    }
}

bool stageDoor::checkActivation()
{
    //check collisions for a player type character
    //if found, check for a specific keypress
    //then return a true if activated, else return false
    std::vector<entity*> closeEntities = checkCloseEntities();
    for(auto& entity : closeEntities)
    {
        //check if entity is player -> change color
        if(entity->alignment == PLAYER){
            entColor = YELLOW;

            //add text - will not work if function not called within "beginDrawing" bounds
            DrawText(TextFormat("A"), hitbox.x + hitbox.width/2, hitbox.y - 20, 10, WHITE);

            //check for keypress if in the same region
            if(IsKeyDown(KEY_A)){
                return true;
            }
        }
    }
    return false;
}