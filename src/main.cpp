#include "include/head.h"

void levelMainMenu(){
     //draw
    BeginDrawing();
    ClearBackground(BLACK);

    const char * mainMenuTitle {"Pixel Pew Pew"};
    const char * mainMenuStart {"Start!"};
    const char * mainMenuQuit {"Quit"};

    float titleSize {75};
    float optionSize {30};

    float titleX {50};
    float titleY {75};
    float startX {50}; 
    float startY {400};
    float quitX {50};
    float quitY {450};

    auto titleColor{WHITE};
    auto startColor{WHITE};
    auto quitColor{WHITE};

    //Vector2 titleDimensions { MeasureTextEx(GetFontDefault(),mainMenuTitle, static_cast<float>(titleSize), 1)};
    Vector2 startOptionDimensions { MeasureTextEx(GetFontDefault(),mainMenuStart, static_cast<float>(optionSize), 1.1)};
    Vector2 quitOptionDimensions { MeasureTextEx(GetFontDefault(),mainMenuQuit, static_cast<float>(optionSize), 1.1)};

    Vector2 mousePos{GetMousePosition()};

    //check for hover and click events
    if(isMouseInRect(mousePos, startX, startY, startOptionDimensions)){
        startColor = GREEN;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            gameState = LEVEL1;
        }
    }else{
        startColor = WHITE;
    }
    if(isMouseInRect(mousePos, quitX, quitY, quitOptionDimensions)){
        quitColor = GREEN;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            gameState = EXITGAME;
        }
    }else{
        quitColor = WHITE;
    }

    //winCondition
    DrawText(TextFormat(mainMenuTitle), titleX, titleY, titleSize, titleColor);
    DrawText(TextFormat(mainMenuStart), startX, startY, optionSize, startColor);
    DrawText(TextFormat(mainMenuQuit), quitX, quitY, optionSize, quitColor);

    EndDrawing();

    //check for option select
}

int main () {
    //INITIALISATION
    //initialise screen
    InitWindow(screenWidth, screenHeight, "A platformer?");
    SetTargetFPS(targetFPS);

    //grid initialise
    gridCell emptyCell;
    for (auto& cell : pairInterpolator(std::make_pair(getCurrentCol(0),getCurrentRow(0)), std::make_pair(getCurrentCol(screenWidth), getCurrentRow(screenHeight))))
    {
        std::cout<<"INSERTING: [" << cell.first << "," << cell.second << "]\n"; 
        gridContainer.insert(std::make_pair(cell,emptyCell));
    }

    //initialise enemy director - THREAD1
    enemyDirector enemyDir;
    enemyDir.spawnThresh = 5*targetFPS;
    enemyDir.spawnTimer = enemyDir.spawnThresh-targetFPS;

    //initialise player 
    player protag;
    
    //initialise platforms - FUNCT
    entity plat_1;
    entity plat_2;
    entity plat_3;
    entity plat_4;
    entity plat_5;

    plat_1.hitbox = {500,450,200,20};
    platform_vector.push_back(plat_1);
    
    plat_2.hitbox = {600,580,200,20};
    platform_vector.push_back(plat_2);
    
    plat_3.hitbox = {600,350,200,20};
    platform_vector.push_back(plat_3);

    plat_4.hitbox = {200,500,300,20};
    platform_vector.push_back(plat_4);

    plat_5.hitbox = {400,150,200,20};
    platform_vector.push_back(plat_5);

    for (auto& plat : platform_vector){
        plat.updateGridOccupation();
    }

    while (gameReplay){
        gameReplay = false;

        //initialise win condition
        int winKillCount {10};
        int killCount {0};

        //GAME LOOP
        while (WindowShouldClose() == false && gameExitConfirmed == false){

            //Code that should run during gameplay
            if(gamePaused != true){
                //Initialisations
                protag.initLoop();

                //key event handling        
                if(IsKeyDown(KEY_LEFT)){    protag.setSpeedX(-1*protag.baseSpeed); protag.faceDirectionX = protag.left;   }
                if(IsKeyDown(KEY_RIGHT)){   protag.setSpeedX(1*protag.baseSpeed);  protag.faceDirectionX = protag.right;     }
                //firing
                if(IsKeyDown(KEY_Z)){
                    protag.fireProjectile(&attackVector);
                }
                //jump
                if(IsKeyPressed(KEY_SPACE)){
                    if(protag.jumpStock > 0){
                        protag.setSpeedY(-25);
                        protag.jumpStock -= 1;
                    }
                }

                //UPDATE VALUES//
                protag.moveX();
                protag.moveY();
                protag.screenBorder(screenHeight, screenWidth);

                //player membership
                protag.updateGridOccupation();

                //check collision between the player and entities within its grid cell
                for(const auto& closeEntity: protag.checkCloseEntities()){
                    protag.collisionHandler(closeEntity);
                }

                //check projectile collisions
                for(auto& proj: attackVector){
                    for (const auto& plat: platform_vector){
                        if(CheckCollisionRecs(proj.hitbox, plat.hitbox)){
                            proj.isAlive = false;
                            break;
                        }
                    }
                    for (auto& enemy: enemyVector){
                        if(CheckCollisionRecs(proj.hitbox, enemy.hitbox)){
                            proj.isAlive = false;
                            enemy.isAlive = false;
                            killCount += 1;
                            break;
                        }
                    }
                }

                //update spawn timer
                enemyDir.spawnTimer += 1;
                if(enemyDir.spawnTimer >= enemyDir.spawnThresh)
                {
                    enemyVector.push_back(enemyDir.spawnCommand());
                    enemyDir.spawnTimer = 0;
                    std::cout<<enemyVector.size()<<std::endl;
                }

                //delete out of range enemies
                std::vector<genericEnemy>::iterator it = enemyVector.begin();
                while(it != enemyVector.end())
                {
                    if(!it->isAlive)
                    {
                        it = enemyVector.erase(it);
                    }else{
                        ++it;
                    }
                }

                //delete out of range enemies
                std::vector<projectileAttack>::iterator atkIter = attackVector.begin();
                while(atkIter != attackVector.end())
                {
                    if(!atkIter->isAlive)
                    {
                        atkIter = attackVector.erase(atkIter);
                    }else{
                        ++atkIter;
                    }
                }

                //update enemy position
                for(auto& foe: enemyVector){
                    foe.updateMovement(protag.hitbox.x,protag.hitbox.y);
                    for(const auto& plat: platform_vector){
                        foe.collisionHandler(&plat.hitbox);
                    }
                    //set the destruction bit
                    if(!foe.checkInSquare())
                    {
                        foe.isAlive = false;
                    }
                }
            }

            //game stage state machine
            switch(gameState)
            {
                case TITLE:
                {
                    levelMainMenu();
                } break;
                case LEVEL1:
                {
                    //draw
                    gamePaused = false;
                    BeginDrawing();
                    ClearBackground(BLACK);

                    DrawRectangle(protag.hitbox.x, protag.hitbox.y, protag.hitbox.width, protag.hitbox.height, WHITE);

                    //draw world
                    for(const auto& plat: platform_vector){
                        DrawRectangle(plat.hitbox.x,plat.hitbox.y,plat.hitbox.width,plat.hitbox.height,GREEN);
                    }

                    //draw enemies
                    for(const auto& foe: enemyVector){
                        DrawRectangle(foe.hitbox.x,foe.hitbox.y,foe.hitbox.width,foe.hitbox.height,RED);
                    }

                    //draw projectiles
                    for(auto& item: attackVector){
                        item.moveProjectile();
                        DrawRectangle(item.hitbox.x,item.hitbox.y,item.hitbox.width,item.hitbox.height,YELLOW);
                    }

                    //draw text
                    //winCondition
                    DrawText(TextFormat("Kills: %02i/%02i", killCount, winKillCount), 20, 20, 20, WHITE);

                    EndDrawing();

                    //check win condition
                    if(killCount >= winKillCount){
                        gameState = TITLE;
                    }
                } break;
                case EXITGAME:
                {
                    gameExitConfirmed = true;
                }break;
                default: break;
            }
        }
    }
    CloseWindow();
    return 0;
}