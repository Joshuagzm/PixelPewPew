#include "include/enemy.h"

//spawn something
genericEnemy enemyDirector::spawnCommand()
{
    //spawn an enemy in the upper third of the screen of some default size
    genericEnemy enemy;
    enemy.hitbox.x = 100 + rand()%(spawnableWidth - 100);
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

    enemy.updateGridOccupation();

    return(enemy);
}

genericEnemy enemyDirector::spawnSlimeBoss()
{
    //spawn an enemy in the upper third of the screen of some default size
    genericEnemy enemy;
    enemy.hitbox.x = 50;
    enemy.hitbox.y = 200;

    enemy.hitbox.height = 80;
    enemy.hitbox.width = 80;

    enemy.halfheight = enemy.hitbox.height/2;
    enemy.halfWidth = enemy.hitbox.width/2;

    //initial stats
    enemy.speedX = 1;
    enemy.speedY = 1;
    enemy.baseSpeed = 8;
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
void enemyDirector::tickUpdate(std::deque<genericEnemy>& enemyList)
{
    //update spawn timer
    this->spawnTimer += 1;
    if(this->spawnTimer >= this->spawnThresh)
    {
        enemyList.push_back(this->spawnCommand());
        this->spawnTimer = 0;
    }
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
    //state, proximity based movement
    //if far from player (based on X direction) when timer elapses, big jump w/projectile, and then 3 normal jumps to catch up
    //if close to player, just normal jump
    
    //state 3 -> handle big jump
    if(moveState == 3){
        
        switch(intState)
        {
            case 0://first stage, -> jump
            {
                jumpHeight = -1*(200);
                //consume jumpstock
                jumpStock -= 1;
                setSpeedY(this->jumpHeight);
                //transition
                intState = 1;
            }break;

            case 1://second stage -> wait at top of the jump (positive ySpeed)
            {
                if(speedY > 0)
                {
                    intState = 2;
                    setSpeedY(0);//stop movement!
                    //wait for half a second
                    timerGoal = loopedAddition(frameCount, 30, frameMax, 0);
                }

            }break;

            case 2://third stage -> wait for timer to elapse then big speed downwards
            {
                if(frameCount == timerGoal){
                    setSpeedY(30);
                    //transition
                    intState = 3;
                }//else keep waiting

            }break;

            case 3:{//third stage -> on grounded, spawn projectiles and decrement movestate

                if(isGrounded)
                {
                    //spawn enemy projectiles
                    std::cout<<"SPAWN 'EM"<<std::endl;
                    moveState -= 1;
                    intState = 0;
                }
            }break;
            default:break;
        }
    }
    //else-> wait for jump timer and stock and normal jump

    //timer elapsed
    if(this->jumpStock == this->jumpMax && jumpTimer >= jumpTimerThresh + rand()%60)
    {
        std::cout<<"MOVESTATE: "<<moveState<<" distance: "<<abs(playerX - this->hitbox.x)<<std::endl;
        //ready for the big jump
        if(moveState == 0 && abs(playerX - this->hitbox.x) > 200){
            moveState = 3;
        }
        //randomise jump height a bit
        this->jumpHeight = -1*(90 + rand()%3);
        //consume jumpstock
        this->jumpStock -= 1;
        this->setSpeedY(this->jumpHeight);

        //state transition
        moveState = cappedSubtraction(moveState, 1, 0);
        //reset timer
        this->jumpTimer = 0;
        //determine x direction movement
        this->dirX = getRelativeDir(this->hitbox.x, playerX);
    }

    //if in the air, move x
    if(abs(this->speedY) > 0 && moveState < 3){
        //adjust speed to dir
        this->setSpeedX(dirX*this->baseSpeed);
    }else{
        this->setSpeedX(0);
    }

    //update timer
    jumpTimer = MIN(jumpTimer+1, jumpTimerThresh+1);

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