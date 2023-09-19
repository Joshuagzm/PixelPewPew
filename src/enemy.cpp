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

bossSlime enemyDirector::spawnSlimeBoss()
{
    //spawn an enemy in the upper third of the screen of some default size
    bossSlime enemy;
    enemy.hitbox.x = 50;
    enemy.hitbox.y = 200;

    enemy.hitbox.height = 80;
    enemy.hitbox.width = 80;

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

        default:break;
    }
    return 0;
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
    dirX = getRelativeDir(this->hitbox.x, playerX);
    
    switch(moveState)
    {
        //normal movement 
        case 0:
        {
            //timer elapsed
            if(this->jumpStock == this->jumpMax && jumpTimer >= jumpTimerThresh + rand()%60)
            {
                std::cout<<"MOVESTATE: "<<moveState<<" distance: "<<abs(playerX - this->hitbox.x)<<std::endl;    
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
                    jumpHeight = -1*(20);
                    this->setSpeedX(dirX*baseSpeed);
                    // consume jumpstock
                    jumpStock -= 1;//normal
                    setSpeedY(this->jumpHeight);//reset in normal mode
                    // transition
                    intState = 1;//safe
                }break;

                case 1://second stage -> wait at top of the jump (positive ySpeed)
                {
                    if(speedY > 0)
                    {
                        setSpeedX(0);
                        setSpeedY(0);//stop movement!
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
                        moveState = 0;
                        intState = 3;
                        //reset jump timer
                        // jumpTimer = 0;
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
    }else{
        std::cout<<"IGNORING GRAVITY\n";
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
    
    return 0;
}