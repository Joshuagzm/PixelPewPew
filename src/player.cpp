#include "include/player.h"

int player::initLoop()
{
    this->prevX = this->hitbox.x;
    this->prevY = this->hitbox.y;
    this->speedX = 0;
    this->abilityCooldown -= 1;
    return(0);
}

//apply speedX
int player::moveX(){
    //TODO: Add acceleration?
    this->hitbox.x += this->speedX;
    return 0;
}

//apply speedY
int player::moveY(){
    this->applyGravity(this->gravity);
    this->hitbox.y += this->speedY;
    return 0;
}

int player::screenBorder(int screenHeight, int screenWidth)
{
    if(this->hitbox.y > screenHeight - this->hitbox.height){
        this->hitbox.y = screenHeight - this->hitbox.height;
        this->speedY = 0;
        if(this->jumpStock < this->jumpMax) this->jumpStock = this->jumpMax;
    }
    if(this->hitbox.y < 0){
        this->hitbox.y = 0;
        this->speedY = 0;
    }
    if(this->hitbox.x < 0){
        this->hitbox.x = 0;
        this->speedX = 0;
    }
    if(this->hitbox.x > screenWidth - this->hitbox.width){
        this->hitbox.x = screenWidth - this->hitbox.width;
        this->speedX = 0;
    }
    return 0;
}

int player::fireProjectile(std::vector<projectileAttack>* attackVector){
    //function to manage firing a basic projectile
    //Allow certain number of shots on screen
    //Cooldown management
    //triggered when a certain key is pressed
    //...     
    if(this->abilityCooldown < 1){
        projectileAttack newProjectile;
        newProjectile.hitbox.x = this->hitbox.x + this->halfheight + this->halfheight*this->faceDirectionX;
        newProjectile.hitbox.y = this->hitbox.y + this->halfheight;
        newProjectile.speedX *= this->faceDirectionX;
        attackVector->push_back(newProjectile);
        this->abilityCooldown = 30;
        std::cout<< "FIRING\n";
        return 0;
    }
    return 0;
}

int player::spawnProjectile(){

    return 0;
}

int projectileAttack::moveProjectile()
{
    this->hitbox.x += this->speedX;
    return 0;
}


projectileAttack::projectileAttack()
{
    this->hitbox.width = 5;
    this->hitbox.height = 5;
    this->hitbox.width = 5;
    this->hitbox.height = 5;
    this->speedX = 15;
}