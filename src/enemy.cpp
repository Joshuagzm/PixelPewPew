#include "include/enemy.h"

//spawn something
genericEnemy enemyDirector::spawnCommand()
{
    //spawn an enemy in the upper third of the screen of some default size
    genericEnemy enemy(this);
    enemy.hitbox.x = 100 + rand()%(spawnableWidth - 800);
    enemy.hitbox.y = 100 + rand()%(spawnableHeight - 100);

    std::cout<<"Spawning enemy at ["<<enemy.hitbox.x<<","<<enemy.hitbox.y<<"]"<<std::endl;
    enemy.hitbox.height = 30;
    enemy.hitbox.width = 30;

    enemy.halfheight = enemy.hitbox.height/2;
    enemy.halfWidth = enemy.hitbox.width/2;

    //initial stats
    enemy.speedX = 1;
    enemy.speedY = 1;
    enemy.baseSpeed = 2;
    enemy.gravity = 1;
    enemy.maxHp = 1;
    enemy.hp = 1;

    enemy.isSolid = false;
    enemy.isTouchDamage = true;

    enemy.alignment = MONSTER;

    return(enemy);
}

bossSlime enemyDirector::spawnSlimeBoss()
{
    //spawn an enemy in the upper third of the screen of some default size
    bossSlime enemy(this);

    enemy.hitbox.height = 80;
    enemy.hitbox.width = 80;
    enemy.hitbox.x = 700;
    enemy.hitbox.y = 500 - enemy.hitbox.height;

    enemy.halfheight = enemy.hitbox.height/2;
    enemy.halfWidth = enemy.hitbox.width/2;

    //initial stats
    enemy.speedX = 1;
    enemy.speedY = 1;
    enemy.baseSpeed = 12;
    enemy.gravity = 1;
    enemy.maxHp = 15;
    enemy.hp = 15;

    enemy.isSolid = false;
    enemy.isTouchDamage = true;

    enemy.alignment = MONSTER;

    enemy.updateGridOccupation();

    return enemy;
}

//define the main movement loop here
//Generic enemy movement - periodically jump towards player
int genericEnemy::updateMovement(float playerX, float playerY)
{   
    switch(eType)
    {
        case ET_SLIME:
        {
            slimeMovement(playerX, playerY);
        }break;

        case ET_SLIMEBOSS:
        {
            slimeBossMovement(playerX, playerY);
        }break;

        case ET_ACCELPROJ_H:
        {
            accelProjHMovement();
        }break;

        default:
        {
            std::cout<<"MOVEMENT NOT HANDLED\n";   
        }break;
    }
    return 0;
}

//horizontally travelling accelerating projectile
void genericEnemy::accelProjHMovement()
{
    //quadratic speed scaling
    this->speedX += 1*this->faceDirectionX;

    //update position
    this->hitbox.x += this->speedX;

    //check for invalid position
    if(!checkInBounds(this->hitbox.x, -1*this->hitbox.width, static_cast<float>(2000)) || !checkInBounds(this->hitbox.y, -1*this->hitbox.height, static_cast<float>(2000)))
    {
        this->isAlive = false;
    }
}

//slime movement AI
void genericEnemy::slimeMovement(float playerX, float playerY)
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
}

//slime boss movement AI
void genericEnemy::slimeBossMovement(float playerX, float playerY)
{
    //state, proximity based movement
    //if far from player (based on X direction) when timer elapses, big jump w/projectile, and then 3 normal jumps to catch up
    //if close to player, just normal jump

    //determine x direction movement
    
    
    switch(moveState)
    {
        //normal movement 
        case 0:
        {
            //timer elapsed
            if(this->jumpStock == this->jumpMax && jumpTimer >= jumpTimerThresh + rand()%60)
            {
                dirX = getRelativeDir(this->hitbox.x, playerX);
                //randomise jump height a bit
                this->jumpHeight = -1*(10 + rand()%3);
                //consume jumpstock
                this->jumpStock -= 1;
                this->setSpeedY(this->jumpHeight);
                //state transition
                intState = cappedSubtraction(intState, 1, 0);
                //reset timer
                this->jumpTimer = 0;
                
                //ready for the big jump
                if(intState == 0 && abs(playerX - this->hitbox.x) > 100){
                    moveState = 1;
                    intState = 0;
                    break;
                }
            }

            //if in the air, move x
            if(isGrounded){
                //adjust speed to dir
                this->setSpeedX(0);
            }else{
                this->setSpeedX(dirX*this->baseSpeed);
            }
            //update timer
            jumpTimer = MIN(jumpTimer+1, jumpTimerThresh+1);
        }break;

        //state 1 -> handle big jump
        case 1:
        {
            switch(intState)
            {
                case 0://first stage, -> jump
                {
                    //calculate direction
                    dirX = getRelativeDir(this->hitbox.x, playerX);

                    //set velocity
                    jumpHeight = -1*(20);
                    this->setSpeedX(dirX*baseSpeed);
                    setSpeedY(this->jumpHeight);

                    // consume jumpstock
                    jumpStock -= 1;

                    // transition
                    intState = 1;
                }break;

                case 1://second stage -> wait at top of the jump (positive ySpeed)
                {
                    if(speedY > 0)
                    {
                        //stop movement!
                        setSpeedX(0);
                        setSpeedY(0);
                        
                        //wait for half a second
                        timerGoal = loopedAddition(frameCount, 15, 0, frameMax);//just the timer
                        intState = 2;//transition
                    }

                }break;

                case 2://third stage -> wait for timer to elapse then big speed downwards
                {
                    setSpeedX(0);
                    setSpeedY(0);//stop movement!
                    if(frameCount == timerGoal){
                        setSpeedY(20);
                        //transition
                        intState = 3;
                    }//else keep waiting

                }break;

                case 3:{//third stage -> on grounded, spawn projectiles and decrement movestate
                    setSpeedX(0);//slam striaght down
                    if(isGrounded)
                    {
                        //spawn enemy projectiles
                        std::cout<<"SPAWN 'EM"<<std::endl;
                        //set up projectile
                        genericEnemy proj(enemyDir);
                        //spawn shockwave at feet
                        proj.hitbox.height = 50;
                        proj.hitbox.width = 20;

                        proj.halfheight = proj.hitbox.height/2;
                        proj.halfWidth = proj.hitbox.width/2;

                        //initial stats
                        proj.speedX = 1;
                        proj.speedY = 1;
                        proj.baseSpeed = 2;
                        proj.gravity = 1;
                        proj.maxHp = 999;
                        proj.hp = 999;

                        proj.isSolid = false;
                        proj.isTouchDamage = true;

                        proj.alignment = MONSTER;
                        proj.entColor = MAROON;
                        proj.eType = ET_ACCELPROJ_H;

                        proj.hitbox.y = this->hitbox.y + (this->hitbox.height - proj.hitbox.height);
                        //spawn left side
                        proj.faceDirectionX = left;
                        proj.hitbox.x = this->hitbox.x - proj.hitbox.width;
                        enemyDir->spawnGenericEnemy(proj);

                        //spawn right side
                        proj.faceDirectionX = right;
                        proj.hitbox.x = this->hitbox.x + this->hitbox.width;
                        enemyDir->spawnGenericEnemy(proj);

                        //transition
                        moveState = 0;
                        intState = 3;
                    }
                }break;
                default:break;
            }
            
        }break;
        default:
        {
            std::cout<<"DEFAULT SLIME BOSS MOVEMENT\n";            
        }break;
    }

    //get previous position
    this->prevX = this->hitbox.x;
    this->prevY = this->hitbox.y;

    //apply gravity
    if(!(moveState == 1 && intState == 2)){
        //apply gravity
        this->applyGravity(this->gravity);
    }

    //update position
    this->hitbox.x += this->speedX;
    this->hitbox.y += this->speedY;
}

int genericEnemy::onHit()
{
    //on hit decrement HP by damage (default 1 for now)
    hp--;
    if(hp < 1){
        onDeath();
        return 1;
    }
    return 0;
}

void genericEnemy::onDeath()
{
    //on death, kill itself
    killEntity();
}

//function to update the enemy director on each tick
void enemyDirector::tickUpdate()
{
    //update spawn timer
    this->spawnTimer += 1;
    if(this->spawnTimer >= this->spawnThresh)
    {
        enemyList->push_back(this->spawnCommand());
        this->spawnTimer = 0;
    }
}

void enemyDirector::spawnGenericEnemy(genericEnemy enemy)
{
    enemyList->push_back(enemy);
}

//AI piece for the first stage boss
void genericEnemy::slimeBossAI()
{
    //Movement
    
    //Attack Pattern
    
    //Phases
}

int bossSlime::updateMovement(float playerX, float playerY)
{   
    
    return 0;
}