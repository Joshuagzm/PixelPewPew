#include "include/head.h"

//Level module declarations
void level1(player* protag, int* killCount, int* winCount, Camera2D* camera);
void levelDead();
void levelMainMenu();
void levelWin();
void levelNetConf();
void levelChat(std::string& latestMessage);

//update camera module declaration
void updateCameraClamp(Camera2D* camera, player* protag, float delta, int width, int height);

void initialiseNetworkConf();

////
//MAIN
//// 
int main () {
    //INITIALISATION
    //initialise screen
    InitWindow(screenWidth, screenHeight, "A platformer?");
    SetTargetFPS(targetFPS);

    //set seed
    srand((unsigned) time(NULL));

    boost::asio::io_context io;
    boost::circular_buffer<std::string> chatHistory;
    std::string latestMessage;

    //TIMER TESTS
    if(false)
    {
    //test timer
    //timer initialisation (timing starts from initialisation)
    recurringTimer recurringTimer(io);
    }

    //ASIO UDP COMMS TEST
    std::cout<<"Enter operation mode:\n1: Client functionality\n2: Server functionality\nanything else: Run the game"<<std::endl;
    int runMode {0};
    std::cin>>runMode;
    boost::asio::ip::udp::endpoint partnerEndpoint;
    bool DEBUGSOCKET;
    networkInstance networkHandler(io);


    switch(runMode)
    {
        case 1://Client
        {
            std::string ipAddrStr{""};
            std::cout<<"Enter server ip address:"<<std::endl;
            std::cin >> ipAddrStr;
            //convert IP address and port from string to correct type
            networkHandler.remote_endpoint.address(boost::asio::ip::address::from_string(ipAddrStr));
            networkHandler.remote_endpoint.port(networkHandler.serverPort);
            partnerEndpoint = networkHandler.syncDTClientUDP(io, latestMessage);
        }break;
        case 2://Server
        {
            partnerEndpoint = networkHandler.syncDTServerUDP(io, latestMessage);
        }break;
        default: break;
    }

    if(networkHandler.socket_.is_open()){
        std::cout<<"SOCKET IS OK AT THREAD INIT"<<std::endl;
    }

    std::thread testThread([&]() {

        io.run();
        std::cout << "IO Context thread exiting..." << std::endl;
    });

    std::cout<<"Thread initialised"<<std::endl;

    //grid initialise
    gridCell emptyCell;
    for (auto& cell : pairInterpolator(std::make_pair(getCurrentCol(0),getCurrentRow(0)), std::make_pair(getCurrentCol(stageWidth), getCurrentRow(stageHeight))))
    {
        gridContainer.insert(std::make_pair(cell,emptyCell));
    }

    //initialise enemy director - THREAD1
    enemyDirector enemyDir;
    enemyDir.spawnThresh = 5*targetFPS;
    enemyDir.spawnTimer = enemyDir.spawnThresh-targetFPS;

    //initialise player 
    player protag;
    protag.initPlayer();
    protag.playerIndex = playerVector.size();
    playerVector.push_back(&protag);

    //initialise player 2 CLIENT SIM
    player pFriend;
    pFriend.initPlayer();
    pFriend.playerIndex = playerVector.size();
    playerVector.push_back(&pFriend);

    //initialise camera
    Camera2D camera {{0}};
    camera.target = (Vector2){ protag.hitbox.x, protag.hitbox.y };
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    //initialise win condition
    int winCount {10};
    int killCount {0};

    //GAME LOOP
    while (WindowShouldClose() == false && gameExitConfirmed == false){

        //update camera
        updateCameraClamp(&camera, &protag, 0.0f, screenWidth, screenHeight);

        //Code that should run during gameplay
        if(gamePaused != true){
            //Initialisations
            protag.initLoop();

            //Movement handling
            protag.checkMoveInput();
            protag.checkAttackInput(&attackVector);

            //CLIENT SIM

            //UPDATE VALUES//
            protag.moveX();
            protag.moveY();
            protag.screenBorder(stageHeight, stageWidth);

            //update grid membership
            protag.updateGridOccupation();
            for (auto& enemy: enemyVector){
                enemy.updateGridOccupation();
            }
            for (auto& proj: attackVector){
                proj.updateGridOccupation();
            }

            //check collision between the player and entities within its grid cell
            for(const auto& closeEntity: protag.checkCloseEntities()){
                protag.collisionHandler(closeEntity);
            }

            //check projectile collisions
            for(auto& proj: attackVector){
                bool exitFlag {false};
                //check against close entities
                for(const auto& closeEntity: proj.checkCloseEntities()){
                    //check collision event
                    if(CheckCollisionRecs(proj.hitbox, closeEntity->hitbox)){
                        switch(closeEntity->alignment){
                            case entity::MONSTER:
                            {
                                proj.isAlive = false;
                                closeEntity->isAlive = false;
                                killCount += 1;
                                //indicate exit
                                exitFlag = true;
                            }break;
                            case entity::OBJECT:
                            {
                                proj.isAlive = false;
                                exitFlag = true;
                            }break;
                            default: break; 
                        }
                    }
                    if(exitFlag){
                        exitFlag = false;
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
            }

            //delete out of range enemies
            std::deque<genericEnemy>::iterator it = enemyVector.begin();
            while(it != enemyVector.end())
            {
                if(!it->isAlive)
                {
                    it->removeGridOccupation(it->gridCellsCurrent);
                    it = enemyVector.erase(it);
                }else{
                    ++it;
                }
            }

            //delete out of range enemies
            std::deque<projectileAttack>::iterator atkIter = attackVector.begin();
            while(atkIter != attackVector.end())
            {
                if(!atkIter->isAlive)
                {
                    atkIter->removeGridOccupation(atkIter->gridCellsCurrent);
                    atkIter = attackVector.erase(atkIter);
                }else{
                    ++atkIter;
                }
            }

            //update enemy position
            for(auto& foe: enemyVector){
                foe.updateMovement(protag.hitbox.x,protag.hitbox.y);
                for(auto& plat: platformVector){
                    foe.collisionHandler(&plat);
                }
                //set the destruction bit
                if( !foe.checkInTopless(&stageWidth, &stageHeight) )
                {
                    foe.isAlive = false;
                }
            }
        }

        //begin camera drawing
        //game stage state machine
        switch(gameState)
        {
            case TITLE:
            {
                levelMainMenu();
            } break;
            case LEVEL1:
            {
                level1(&protag, &killCount, &winCount, &camera);
            } break;
            case DEAD:
            {
                levelDead();

            } break;
            case WIN:
            {
                levelWin();

            } break;
            case CHAT:
            {
                levelChat(latestMessage);

            } break;
            case NETCONF:
            {
                levelNetConf();

            } break;
            case EXITGAME:
            {
                gameExitConfirmed = true;
            }break;
            default: break;
        }
        EndMode2D();
        //state update
        prevState = gameState;
        if(requestState != DUMMY){
            gameState = requestState;
        }
    }
    testThread.join();
    CloseWindow();
    return 0;
}

void updateCameraClamp(Camera2D* camera, player* protag, float delta, int width, int height)
{
    
    //camera is focused on the player, and the object of focus is kept at the center of the screen
    //target locates the origin, and offset tells where the offset is on the screen

    //add some wiggle room for the character
    int borderX = screenWidth/2 - 50;  //Border on the X extremeties
    int borderY = screenHeight/2 - 50;  //Border on the Y extremeties

    Vector2 bboxTopLeft = GetScreenToWorld2D((Vector2) {static_cast<float>(borderX), static_cast<float>(borderY)}, *camera);
    Vector2 bboxBottomRight = GetScreenToWorld2D((Vector2) {static_cast<float>(screenWidth - borderX), static_cast<float> (screenHeight - borderY)}, *camera);

    if(protag->hitbox.x < bboxTopLeft.x){       camera->target.x += protag->hitbox.x - bboxTopLeft.x;}
    if(protag->hitbox.y < bboxTopLeft.y){       camera->target.y += protag->hitbox.y - bboxTopLeft.y;}
    if(protag->hitbox.x > bboxBottomRight.x){   camera->target.x += protag->hitbox.x - bboxBottomRight.x;}
    if(protag->hitbox.y > bboxBottomRight.y){   camera->target.y += protag->hitbox.y - bboxBottomRight.y;}

    // camera->target.x = protag->hitbox.x;
    // camera->target.y = protag->hitbox.y;

    //clamps to the screen edges
    //get the world edges in the screen space and clamps
    Vector2 camTopLeft =       GetWorldToScreen2D((Vector2){0,0}, *camera);
    Vector2 camBottomRight =   GetWorldToScreen2D((Vector2){static_cast<float>(stageWidth),static_cast<float>(stageHeight)}, *camera);
    if(camTopLeft.x > 0){          camera->offset.x = (camera->offset.x - camTopLeft.x);              }
    if(camTopLeft.y > 0){          camera->offset.y = (camera->offset.y - camTopLeft.y);              }
    if(camBottomRight.x < width){  camera->offset.x = camera->offset.x + (width - camBottomRight.x);  }
    if(camBottomRight.y < height){ camera->offset.y = camera->offset.y + (height - camBottomRight.y); }
}

void level1(player* protag, int* killCount, int* winCount, Camera2D* camera){
    //on transition
    if(prevState != gameState){
        stageWidth = 2*screenHeight;
        stageHeight = screenHeight;
        resetWorld(&platformVector, &enemyVector, &attackVector);
        registerPlatform(&platformVector, 20,100,400,20);
        registerPlatform(&platformVector, 600,200,100,20);
        registerPlatform(&platformVector, 800,300,600,20);
        registerPlatform(&platformVector, 600,400,100,20);
        registerPlatform(&platformVector, 450,200,75,20);
        registerPlatform(&platformVector, 1350,200,100,20);
        registerPlatform(&platformVector, 1400,300,100,20);
        registerPlatform(&platformVector, 0,stageHeight - 10,stageWidth,10);    //the floor
        *winCount = 10;
        *killCount = 0;
        protag->initPlayer();
    
    }
    //draw - order of drawing determines layers
    gamePaused = false;
    //platform border
    int platformBorder {2};
    BeginDrawing();
    ClearBackground(BLACK);

    BeginMode2D(*camera);

    //draw projectiles
    for(auto& item: attackVector){
        item.moveProjectile();
        DrawRectangle(item.hitbox.x,item.hitbox.y,item.hitbox.width,item.hitbox.height,YELLOW);
    }

    //draw world
    for(const auto& plat: platformVector){
        DrawRectangle(plat.hitbox.x-platformBorder,plat.hitbox.y,plat.hitbox.width+platformBorder*2,plat.hitbox.height,GREEN);
    }

    //draw enemies
    for(const auto& foe: enemyVector){
        DrawRectangle(foe.hitbox.x,foe.hitbox.y,foe.hitbox.width,foe.hitbox.height,RED);
    }

    for(const auto& player: playerVector){
        DrawRectangle(player->hitbox.x, player->hitbox.y, player->hitbox.width, player->hitbox.height, player->entColor);
    }
    EndMode2D();

    //draw text
    //winCondition
    DrawText(TextFormat("Kills: %02i/%02i", *killCount, *winCount), 20, 20, 20, WHITE);
    DrawText(TextFormat("Health: %02i/%02i", protag->hp, protag->maxHp), screenWidth - 150, 20, 20, WHITE);

    EndDrawing();

    //check win condition
    if(*killCount >= *winCount){
        requestState = WIN;
    }
    //check death condition
    if(protag->hp <= 0){
        //go to death screen
        requestState = DEAD;
    }
}

void levelWin(){
    //set paused
    gamePaused = true;
    //draw
    BeginDrawing();
    ClearBackground(BLACK);

    const char* winMessage {"Victory!"};

    //getting size and position of message, should be centered in the top third
    float messasgeSize {50};
    float promptSize {20};
    
    //text definitions
    const char * promptMessage = "return to menu";

    Vector2 messasgeDimensions { MeasureTextEx(GetFontDefault(),winMessage, static_cast<float>(messasgeSize), 2)};
    Vector2 promptDimensions { MeasureTextEx(GetFontDefault(),promptMessage, static_cast<float>(promptSize), 2)};

    //determine the position 
    float messasgeX {screenWidth/2 - messasgeDimensions.x/2};
    float messasgeY {screenHeight/3};
    float promptX   {screenWidth/2 - promptDimensions.x/2};
    float promptY   {screenHeight - 100};

    auto messasgeColor{GREEN};
    auto promptColor{WHITE};

    Vector2 mousePos{GetMousePosition()};

    //check for hover and click events
    if(isMouseInRect(mousePos, promptX, promptY, promptDimensions)){
        promptColor = GREEN;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            requestState = TITLE;
        }
    }else{
        promptColor = WHITE;
    }

    //winCondition
    DrawText(TextFormat(winMessage), messasgeX, messasgeY, messasgeSize, messasgeColor);
    DrawText(TextFormat(promptMessage), promptX, promptY, promptSize, promptColor);

    EndDrawing();
}

void levelDead(){
    //set paused
    gamePaused = true;
    //draw
    BeginDrawing();
    ClearBackground(BLACK);

    std::vector<const char *> deathMessages;
    deathMessages.push_back("Sucks to suck.");
    deathMessages.push_back("You died.");
    deathMessages.push_back("Weak. Noob.");
    deathMessages.push_back("Maybe you should sleep.");

    //getting size and position of message, should be centered in the top third
    float messasgeSize {20};
    float promptSize {20};

    if(prevState != gameState){
        randMsgIndex = rand() % deathMessages.size();
    }
    
    //text definitions
    const char * deathMessage = deathMessages.at(randMsgIndex);
    const char * promptMessage = "return to menu";

    Vector2 messasgeDimensions { MeasureTextEx(GetFontDefault(),deathMessage, static_cast<float>(messasgeSize), 2)};
    Vector2 promptDimensions { MeasureTextEx(GetFontDefault(),promptMessage, static_cast<float>(promptSize), 2)};

    //determine the position 
    float messasgeX {screenWidth/2 - messasgeDimensions.x/2};
    float messasgeY {screenHeight/3};
    float promptX   {screenWidth/2 - promptDimensions.x/2};
    float promptY   {screenHeight - 100};

    auto messasgeColor{RED};
    auto promptColor{WHITE};

    Vector2 mousePos{GetMousePosition()};

    //check for hover and click events
    if(isMouseInRect(mousePos, promptX, promptY, promptDimensions)){
        promptColor = GREEN;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            requestState = TITLE;
        }
    }else{
        promptColor = WHITE;
    }

    //winCondition
    DrawText(TextFormat(deathMessage), messasgeX, messasgeY, messasgeSize, messasgeColor);
    DrawText(TextFormat(promptMessage), promptX, promptY, promptSize, promptColor);

    EndDrawing();
}

void levelMainMenu(){
    gamePaused = true;
    //draw
    BeginDrawing();
    ClearBackground(BLACK);

    const char * mainMenuTitle {"Pixel Pew Pew"};
    const char * mainMenuStart {"Start!"};
    const char * mainMenuQuit {"Quit"};
    const char * mainMenuNetwork {"Network Configuration"};

    float titleSize {75};
    float optionSize {30};

    float titleX {50};
    float titleY {75};
    float startX {50}; 
    float startY {400};
    float quitX {50};
    float quitY {450};
    float networkX {50};
    float networkY {500};

    auto titleColor{WHITE};
    auto startColor{WHITE};
    auto quitColor{WHITE};
    auto networkColor{WHITE};

    //Vector2 titleDimensions { MeasureTextEx(GetFontDefault(),mainMenuTitle, static_cast<float>(titleSize), 1)};
    Vector2 startOptionDimensions { MeasureTextEx(GetFontDefault(),mainMenuStart, static_cast<float>(optionSize), 1.2)};
    Vector2 quitOptionDimensions { MeasureTextEx(GetFontDefault(),mainMenuQuit, static_cast<float>(optionSize), 1.2)};
    Vector2 networkOptionDimensions { MeasureTextEx(GetFontDefault(),mainMenuNetwork, static_cast<float>(optionSize), 1.2)};

    Vector2 mousePos{GetMousePosition()};

    //check for hover and click events
    if(isMouseInRect(mousePos, startX, startY, startOptionDimensions)){
        startColor = GREEN;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            requestState = LEVEL1;
        }
    }else{
        startColor = WHITE;
    }
    if(isMouseInRect(mousePos, quitX, quitY, quitOptionDimensions)){
        quitColor = GREEN;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            requestState = EXITGAME;
        }
    }else{
        quitColor = WHITE;
    }
    if(isMouseInRect(mousePos, networkX, networkY, networkOptionDimensions)){
        networkColor = GREEN;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            requestState = NETCONF;
        }
    }else{
        networkColor = WHITE;
    }

    //winCondition
    DrawText(TextFormat(mainMenuTitle), titleX, titleY, titleSize, titleColor);
    DrawText(TextFormat(mainMenuStart), startX, startY, optionSize, startColor);
    DrawText(TextFormat(mainMenuQuit), quitX, quitY, optionSize, quitColor);
    DrawText(TextFormat(mainMenuNetwork), networkX, networkY, optionSize, networkColor);

    EndDrawing();
    
}

void levelNetConf(){
    gamePaused = true;
    //draw
    BeginDrawing();
    ClearBackground(BLACK);

    const char * textTitle {"Network Config"};
    const char * textServer {"Connect as Server"};
    const char * textClient {"Connect as Client"};

    float titleSize {50};
    float optionSize {30};

    //Vector2 titleDimensions { MeasureTextEx(GetFontDefault(),mainMenuTitle, static_cast<float>(titleSize), 1)};
    Vector2 titleDimensions { MeasureTextEx(GetFontDefault(),textTitle, static_cast<float>(optionSize), 1.2)};
    Vector2 serverDimensions { MeasureTextEx(GetFontDefault(),textServer, static_cast<float>(optionSize), 1.2)};
    Vector2 clientDimensions { MeasureTextEx(GetFontDefault(),textClient, static_cast<float>(optionSize), 1.2)};

    float titleX {screenWidth/2 - titleDimensions.x/2};
    float titleY {75};
    float serverX {screenWidth/2 - serverDimensions.x/2}; 
    float serverY {200};
    float clientX {screenWidth/2 - clientDimensions.x/2};
    float clientY {300};

    auto titleColor{WHITE};
    auto serverColor{WHITE};
    auto clientColor{WHITE};

    Vector2 mousePos{GetMousePosition()};

    //check for hover and click events
    if(isMouseInRect(mousePos, serverX, serverY, serverDimensions)){
        serverColor = GREEN;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            requestState = CHAT;
        }
    }else{
        serverColor = WHITE;
    }
    if(isMouseInRect(mousePos, clientX, clientY, clientDimensions)){
        clientColor = GREEN;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            requestState = CHAT;
        }
    }else{
        clientColor = WHITE;
    }

    //winCondition
    DrawText(TextFormat(textTitle), titleX, titleY, titleSize, titleColor);
    DrawText(TextFormat(textServer), serverX, serverY, optionSize, serverColor);
    DrawText(TextFormat(textClient), clientX, clientY, optionSize, clientColor);

    EndDrawing();
    
}

void levelChat(std::string& latestMessage){
    gamePaused = true;
    //draw
    BeginDrawing();
    ClearBackground(BLACK);

    const char * textTitle {"Chat"};
    const char * textServer {latestMessage.c_str()};
    const char * textClient {"Connect as client"};
    const char * textReturn {"Return to title"};

    float titleSize {50};
    float optionSize {30};

    //Vector2 titleDimensions { MeasureTextEx(GetFontDefault(),mainMenuTitle, static_cast<float>(titleSize), 1)};
    Vector2 titleDimensions { MeasureTextEx(GetFontDefault(),textTitle, static_cast<float>(optionSize), 1.2)};
    Vector2 serverDimensions { MeasureTextEx(GetFontDefault(),textServer, static_cast<float>(optionSize), 1.2)};
    Vector2 clientDimensions { MeasureTextEx(GetFontDefault(),textClient, static_cast<float>(optionSize), 1.2)};
    Vector2 returnDimensions { MeasureTextEx(GetFontDefault(),textReturn, static_cast<float>(optionSize), 1.2)};

    float titleX {screenWidth/2 - titleDimensions.x/2};
    float titleY {75};
    float serverX {screenWidth/2 - serverDimensions.x/2}; 
    float serverY {200};
    float clientX {screenWidth/2 - clientDimensions.x/2};
    float clientY {300};
    float returnX {screenWidth - returnDimensions.x - 50};
    float returnY {500};

    auto titleColor{WHITE};
    auto serverColor{WHITE};
    auto clientColor{WHITE};
    auto returnColor{WHITE};

    Vector2 mousePos{GetMousePosition()};

    //check for hover and click events
    if(isMouseInRect(mousePos, serverX, serverY, serverDimensions)){
        serverColor = GREEN;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            requestState = CHAT;
        }
    }else{
        serverColor = WHITE;
    }
    if(isMouseInRect(mousePos, clientX, clientY, clientDimensions)){
        clientColor = GREEN;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            requestState = CHAT;
        }
    }else{
        clientColor = WHITE;
    }
    if(isMouseInRect(mousePos, returnX, returnY, returnDimensions)){
        returnColor = GREEN;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            requestState = TITLE;
        }
    }else{
        returnColor = WHITE;
    }

    //winCondition
    DrawText(TextFormat(textTitle), titleX, titleY, titleSize, titleColor);
    DrawText(TextFormat(textServer), serverX, serverY, optionSize, serverColor);
    DrawText(TextFormat(textClient), clientX, clientY, optionSize, clientColor);
    DrawText(TextFormat(textReturn), returnX, returnY, optionSize, returnColor);

    EndDrawing();
    
}