#ifndef INTERACTABLES_H
#define INTERACTABLES_H

#include <raylib.h>
#include "include/functs.h"
#include "include/entity.h"

//interactables: Door that goes to the next stage
class stageDoor : public entity
{
    public:
        stageDoor(){
            //initialise values
            faceDirectionY = 1;
            frameMax = 60;
            frameCount = 0;
            hitbox.height = 40;
            hitbox.width = 20;
            hitbox.x = 50;
            hitbox.y = 525;
            entColor = ORANGE;
        }
        void updateMovement();
        bool checkActivation();
        void activate();

    private:
        bool isMoving{true};
};


#endif