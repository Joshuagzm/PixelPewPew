#include "include/player.h"

int player::initLoop()
{
    //store previous location
    this->prevX = this->hitbox.x;
    this->prevY = this->hitbox.y;

    this->abilityCooldown -= 1;
    
    if(this->invulnerableTimer > 0){
        --this->invulnerableTimer; 
        this->isInvulnerable = true;
        this->entColor = BLUE;
    }else{
        this->isInvulnerable = false;
        this->entColor = WHITE;
    }

    //tick counter
    inertiaDecayTick++;

    //slow down decay
    if(inertiaDecayTick >= inertiaDecayRate){
        //inertia decay
        if(this->inertiaX > 0){
            this->inertiaX = cappedSubtraction(this->inertiaX, this->inertiaDecay, 0); //decay inertia towards 0
        }else if (this->inertiaX < 0){
            this->inertiaX = cappedAddition(this->inertiaX, this->inertiaDecay, 0); //decay inertia towardsd 0
        }
        if(this->inertiaY > 0){
            this->inertiaY = cappedSubtraction(this->inertiaY, this->inertiaDecay, 0); //decay inertia towards 0
        }else if (this->inertiaY < 0){
            this->inertiaY = cappedAddition(this->inertiaY, this->inertiaDecay, 0); //decay inertia towardsd 0
        }
        //reset tick counter
        inertiaDecayTick = 0;
    }

    return(0);
}

int player::initPlayer()
{
    this->hitbox.x = 100;
    this->hitbox.y = 100;
    this->hitbox.width = 20;
    this->hitbox.height = 20;
    this->halfheight = this->hitbox.height/2;
    this->halfWidth = this->hitbox.width/2;
    this->speedX = 5;
    this->speedY = 0;
    this->jumpMax = 2;
    this->maxHp = 3;
    this->hp = this->maxHp;
    this->isInvulnerable = false;
    this->invulnerableTimer = 0;
    this->alignment = PLAYER;

    return 0;
}

//check user inputs
int player::checkMoveInput(){
    if(IsKeyDown(KEY_LEFT)){
        this->setSpeedX(-1*this->baseSpeed); 
        this->faceDirectionX = this->left;   
    }else if(IsKeyDown(KEY_RIGHT)){
        this->setSpeedX(1*this->baseSpeed);  
        this->faceDirectionX = this->right;
    }else{
        this->setSpeedX(0);
    }

    //jump
    if(IsKeyPressed(KEY_SPACE)){
        if(this->jumpStock > 0){
            this->setSpeedY(-25);
            this->jumpStock -= 1;
        }
    }

    return 0;
}

int player::checkAttackInput(std::list<projectileAttack>* attackVector){
    //firing
    if(IsKeyDown(KEY_Z)){
        this->fireProjectile(attackVector);
    }
    return 0;
}

//apply speedX
int player::moveX(int tickCounter){
    //Add acceleration to speedX
    /*
    limit speed increase due to speedX to accelerationX per tick
    if speedX > 0, then the abs throttled speed increases by accelerationX to a max abs value of speedX
    if speedX == 0, then the player is stopped, and throttledspeed reduces to 0
    */
    if(abs(speedX) > 0){
        if(tickCounter % accelerationRateX == 0){
            throttledSpeedX = cappedAddition(abs(throttledSpeedX), accelerationX, abs(speedX));
            throttledSpeedX = abs(throttledSpeedX)*faceDirectionX;
        }        
    }else{
        throttledSpeedX = 0;
    }
    hitbox.x += throttledSpeedX + inertiaX;
    return 0;
}

//apply speedY
int player::moveY(){
    applyGravity(gravity);
    hitbox.y += speedY + inertiaY;
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

int player::fireProjectile(std::list<projectileAttack>* attackVector){
    //function to manage firing a basic projectile
    //Allow certain number of shots on screen
    //Cooldown management
    //triggered when a certain key is pressed
    //...     
    if(this->abilityCooldown < 1){
        projectileAttack newProjectile;
        newProjectile.hitbox.x = this->hitbox.x + this->halfWidth + this->halfheight*this->faceDirectionX;
        newProjectile.hitbox.y = this->hitbox.y + this->halfheight - newProjectile.hitbox.height/2;
        newProjectile.speedX *= this->faceDirectionX;
        newProjectile.isSolid = false;
        attackVector->push_back(newProjectile);
        this->abilityCooldown = 30;

        //register into grid
        attackVector->back().updateGridOccupation();
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
    this->hitbox.width = 7;
    this->hitbox.height = 7;
    this->speedX = 15;
}