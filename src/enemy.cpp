#include "include/enemy.h"

//spawn something
genericEnemy enemyDirector::spawnCommand()
{
    //spawn an enemy in the upper third of the screen of some default size
    genericEnemy enemy;
    enemy.hitbox.x = 20 + rand()%600;
    enemy.hitbox.y = 50 + rand()%200;

    enemy.hitbox.height = 30;
    enemy.hitbox.width = 30;

    enemy.halfheight = enemy.hitbox.height/2;
    enemy.halfWidth = enemy.hitbox.width/2;

    //initial stats
    enemy.speedX = 1;
    enemy.speedY = 1;
    enemy.baseSpeed = 2;
    enemy.gravity = 1;

    enemy.isSolid = false;
    enemy.isTouchDamage = true;

    enemy.updateGridOccupation();


    return(enemy);
}

//define the main movement loop here
//Generic enemy movement - periodically jump towards player
int genericEnemy::updateMovement(float playerX, float playerY)
{   
    //jump based on the timer
    if(this->jumpStock == this->jumpMax && jumpTimer >= jumpTimerThresh)
    {
        //randomise jump height a bit
        this->jumpHeight = -1*(12 + rand()%3);
        //consume jumpstock
        this->jumpStock -= 1;
        this->setSpeedY(this->jumpHeight);
        this->jumpTimer = 0;
        //first determine x direction movement
        this->dirX = getRelativeDir(this->hitbox.x, playerX);
    }else{
        jumpTimer = MIN(jumpTimer+1, jumpTimerThresh+1);
    }
    //if in the air, move x
    if(abs(this->speedY) > 0){
        //adjust speed to dir
        this->setSpeedX(dirX*this->baseSpeed);
    }else{
        this->setSpeedX(0);
    }
    //apply gravity
    this->applyGravity(this->gravity);

    //get previous position
    this->prevX = this->hitbox.x;
    this->prevY = this->hitbox.y;

    //update position
    this->hitbox.x += this->speedX;
    this->hitbox.y += this->speedY;
    return 0;
}