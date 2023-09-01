#include "include/head.h"



//update camera module declaration
void updateCameraClamp(Camera2D* camera, player* protag, float delta, int width, int height);
////
//MAIN
//// 
int main () {
    //SERVER CLIENT COMMON
    InitWindow(screenWidth, screenHeight, "A platformer?");
    SetTargetFPS(targetFPS);

    //set seed
    srand((unsigned) time(NULL));

    //start io context
    boost::asio::io_context io;
    //ensure that the context never runs out of work
    boost::asio::io_context::work dummyWork(io);
    //circular buffer to keep limited chat history
    int chatHistoryLength {10};
    std::string textHistoryDefault{""};
    boost::circular_buffer<std::string> chatBuffer(chatHistoryLength, textHistoryDefault);
    //the latest message
    std::string latestMessage;
    int runMode {0};
    std::thread testThread([&io]() {
        io.run();
        std::cout << "IO Context thread exiting..." << std::endl;
    });

    networkInstance networkHandler(io);    

    //ipaddress
    std::string ipAddrStr{""};
    bool inputSelected{false};
    //generic text input string
    std::string textInput{""};

    //Communication state and reading variables
    int readingState {0};//integer representing reading state: 0 - Header, 1 - body;
    uint32_t headerSize{0};
    uint32_t bodySize{0};


    //TIMER TESTS
    if(false)
    {
    //test timer
    //timer initialisation (timing starts from initialisation)
    recurringTimer recurringTimer(io);
    }

    //grid initialise
    gridCell emptyCell;
    for (auto& cell : pairInterpolator(std::make_pair(getCurrentCol(0),getCurrentRow(0)), std::make_pair(getCurrentCol(stageWidth), getCurrentRow(stageHeight))))
    {
        gridContainer.insert(std::make_pair(cell,emptyCell));
    }

    //SERVER ONLY
    //initialise enemy director - THREAD1
    enemyDirector enemyDir;
    enemyDir.spawnThresh = 5*targetFPS;
    enemyDir.spawnTimer = enemyDir.spawnThresh-targetFPS;

    //COMMON
    //initialise player 
    player protag;
    protag.initPlayer();
    protag.playerIndex = playerVector.size();
    playerVector.push_back(&protag);

    player p_client;
    p_client.initPlayer();
    p_client.playerIndex = playerVector.size();
    playerVector.push_back(&p_client);

    //COMMON
    //initialise camera
    Camera2D camera {{0}};
    camera.target = (Vector2){ protag.hitbox.x, protag.hitbox.y };
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    //Common
    //initialise win condition
    int winCount {10};
    int killCount {0};

    //GAME LOOP
    while (WindowShouldClose() == false && gameExitConfirmed == false){
        //COMMON
        //while looping, check the received data queue
        std::unique_lock<std::mutex> lock(queueMutex);

        //initialise reading variables
        messageType readMsgType{M_MISC};
        bool isReading{true};
        //loop untill all is read 
        while(!receivedDataQueue.empty() && isReading)
        {
            switch(readingState)
                {
                    case 0: //read header
                    {
                        //first, read padding to obtaint headerSize
                        std::string headerPadStr{getBytesFromQueue(receivedDataQueue, headerPadSize)};
                        //convert to size
                        headerSize = static_cast<uint32_t>(std::stoul(headerPadStr));
                        //initialise istream and read header from queue contents
                        std::string queueString{getBytesFromQueue(receivedDataQueue, headerSize)};
                        if(queueString.empty()){
                            isReading = false;
                            break;
                        }
                        std::stringstream ss(queueString);
                        boost::archive::text_iarchive iarchive(ss);

                        //deserialise header into dummy header object
                        messageHeader readHeader;
                        iarchive >> readHeader;
                        bodySize = readHeader.bodySize;
                        readMsgType = readHeader.msgType;

                        readingState = 1;
                    }break;

                    case 1:
                    {
                        //initialise istream and read header from queue contents
                        std::string queueString{getBytesFromQueue(receivedDataQueue, bodySize)};
                        if(queueString.empty()){
                            isReading = false;
                            break;
                        }

                        //if serialised, initialised stream and archive
                        std::stringstream ss(queueString);
                        boost::archive::text_iarchive iarchive(ss);

                        switch(readMsgType)
                        {
                            //handle string
                            case M_STR:
                            {
                                std::string recv_msg;
                                iarchive >> recv_msg;
                                chatBuffer.push_back("Received: " + recv_msg);

                            }break;
                            //handle player information
                            case M_PLAYER:
                            {
                                //deserialise into player object
                                player tempPlayer;
                                iarchive >> p_client;
                            }break;

                            case M_PROJ:
                            {
                                //Overwrite the existing external projectiles vector
                                std::deque<projectileAttack> tempAttackVector;
                                iarchive >> tempAttackVector;
                                extAttackVector = tempAttackVector;
                            }break;

                            case M_ENEMY:
                            {
                                //overwrite existing enemyvector
                                std::deque<genericEnemy> tempEnemyVector;
                                iarchive >> enemyVector;

                            }break;
                            
                            //add command - for sending events 
                            case M_EVENT:
                            {
                                //what kind of events would need to be sent
                                /*
                                Enemy hit commands
                                Score update
                                Death
                                Screen transitions

                                */
                              eventMessage tempEvent;
                              iarchive >> tempEvent;
                              //read the event type
                              switch(tempEvent.eType)
                              {
                                case E_HIT:
                                {
                                    //enemy was hit! find the corresponding enemy based on UUID and unalive is TODO: make this better - restructure as a map?
                                    for (auto& enemy: enemyVector){
                                        if(enemy.getIDString() ==  tempEvent.eID){
                                            enemy.killEntity();
                                            break;
                                        }
                                    }

                                }break;

                                case E_SCORE:
                                {
                                    if(peerType == SERVER){
                                        //add on to kill count
                                        killCount += tempEvent.eValue;
                                    }else if(peerType == CLIENT){
                                        killCount = tempEvent.eValue;
                                    }
                                }break;

                                case E_SCREEN:
                                {
                                    //screen transition request
                                    requestState = static_cast<screen>(tempEvent.eValue);
                                }break;
                                default: break;
                              }
                            }break;

                            //default case, fallthrough
                            default: break;
                        }
                        readingState = 0;
                    }break;

                    default:
                    {
                        std::cerr<<"ERROR SOMETHING WENT WRONG WITH THE READING"<<std::endl;
                    }break;

                }
        }
        lock.unlock();
        //update camera
        updateCameraClamp(&camera, &protag, 0.0f, screenWidth, screenHeight);

        //Code that should run during gameplay
        if(gamePaused != true){
            //Initialisations
            protag.initLoop();
            //Movement handling
            protag.checkMoveInput();
            protag.checkAttackInput(&attackVector);

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
                                proj.killEntity();
                                closeEntity->killEntity();
                                killCount += 1;
                                //indicate exit
                                exitFlag = true;
                                //if client, send kill message to the server
                                if(peerType == CLIENT){
                                    //form event
                                    eventMessage killEvent;
                                    killEvent.eType = E_HIT;
                                    killEvent.eID = closeEntity->getIDString();
                                    //serialise and send
                                    std::string msgBody{getSerialisedStr(killEvent)};
                                    std::string msgHeader{getSerialisedStrHeader(msgBody.size(), M_EVENT)};
                                    networkHandler.sendMessage(msgHeader, msgBody);

                                    //form update score event TODO
                                    eventMessage scoreEvent;
                                    scoreEvent.eType = E_SCORE;
                                    scoreEvent.eValue = 1;//score increment value;
                                    //serialise and send
                                    msgBody = getSerialisedStr(scoreEvent);
                                    msgHeader = getSerialisedStrHeader(msgBody.size(), M_EVENT);
                                    networkHandler.sendMessage(msgHeader, msgBody);
                                }
                            }break;
                            case entity::OBJECT:
                            {
                                proj.killEntity();
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

            //ENEMY DIRECTOR
            if(peerType != CLIENT){
                enemyDir.tickUpdate(enemyVector);
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
                    foe.killEntity();
                }
            }
            
            //SEND DATA//
            if(peerType != DEFAULT)
            {
                //transmit player information
                std::string playerStr{protag.getSerialisedEntity()};
                std::string playerHeader{protag.getSerialisedEntityHeader(playerStr.size(), M_PLAYER)};
                networkHandler.sendMessage(playerHeader, playerStr);

                //transmit projectile information
                //TODO: Minimise network load by oneshotting deque if empty, and then resuming if not empty
                std::string projString{getSerialisedStr(attackVector)};
                std::string projHeader{getSerialisedStrHeader(projString.size(), M_PROJ)};
                networkHandler.sendMessage(projHeader, projString);

                if(peerType == SERVER)
                {
                    //TODO: Minimise network load by oneshotting deque if empty, and then resuming if not empty
                    //transmit enemy information
                    std::string enemyString{getSerialisedStr(enemyVector)};
                    std::string enemyHeader{getSerialisedStrHeader(enemyString.size(), M_ENEMY)};
                    networkHandler.sendMessage(enemyHeader, enemyString);

                    //form update score event TODO
                    eventMessage scoreEvent;
                    scoreEvent.eType = E_SCORE;
                    scoreEvent.eValue = killCount;//score increment value;
                    //serialise and send
                    std::string msgBody{getSerialisedStr(scoreEvent)};
                    std::string msgHeader{getSerialisedStrHeader(msgBody.size(), M_EVENT)};
                    networkHandler.sendMessage(msgHeader, msgBody);
                }
            }

            //CLEANUP//
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
        }

        switch(runMode)
        {
            case 1://Connecting as Client
            {
                //default IP
                if(ipAddrStr == ""){
                    ipAddrStr = "192.168.1.142";
                    std::cerr << "Empty string given for IP address, falling back to default"<<std::endl;
                }
                //convert IP address and port from string to correct type
                networkHandler.remote_endpoint.address(boost::asio::ip::address::from_string(ipAddrStr));
                networkHandler.remote_endpoint.port(networkHandler.serverPort);
                //connect to server as client
                networkHandler.syncDTClientUDP(io);
                //reset run mode
                runMode = 0;
                peerType = CLIENT;
            }break;
            case 2://Connecting as Server
            {
                networkHandler.syncDTServerUDP(io);
                //reset run mode
                runMode = 0;
                peerType = SERVER;
            }break;
            case -1://stop service
            {
                io.stop();
                runMode = 0;
                peerType = DEFAULT;
            }
            default: break;
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
                levelChat(latestMessage, inputSelected, textInput, chatHistoryLength, chatBuffer, networkHandler);

            } break;
            case NETCONF:
            {
                levelNetConf(runMode, ipAddrStr, inputSelected);

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
    io.stop();
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
    for(auto& item: extAttackVector){
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

    Vector2 messasgeDimensions { MeasureTextEx(GetFontDefault(),winMessage, static_cast<float>(messasgeSize), textMeasureFactor)};
    Vector2 promptDimensions { MeasureTextEx(GetFontDefault(),promptMessage, static_cast<float>(promptSize), textMeasureFactor)};

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

    Vector2 messasgeDimensions { MeasureTextEx(GetFontDefault(),deathMessage, static_cast<float>(messasgeSize), textMeasureFactor)};
    Vector2 promptDimensions { MeasureTextEx(GetFontDefault(),promptMessage, static_cast<float>(promptSize), textMeasureFactor)};

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
    Vector2 startOptionDimensions { MeasureTextEx(GetFontDefault(),mainMenuStart, static_cast<float>(optionSize), textMeasureFactor)};
    Vector2 quitOptionDimensions { MeasureTextEx(GetFontDefault(),mainMenuQuit, static_cast<float>(optionSize), textMeasureFactor)};
    Vector2 networkOptionDimensions { MeasureTextEx(GetFontDefault(),mainMenuNetwork, static_cast<float>(optionSize), textMeasureFactor)};

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

void levelNetConf(int& runMode, std::string& ipAddrStr, bool& inputSelected){
    gamePaused = true;
    //draw
    BeginDrawing();
    ClearBackground(BLACK);
    Vector2 mousePos{GetMousePosition()};

    const char * textTitle  {"Network Config"};
    const char * textServer {"Connect as Server"};
    const char * textClient {"Connect as Client"};

    float titleSize {50};
    float optionSize {30};

    //Vector2 titleDimensions { MeasureTextEx(GetFontDefault(),mainMenuTitle, static_cast<float>(titleSize), 1)};
    Vector2 titleDimensions  { MeasureTextEx(GetFontDefault(),textTitle, static_cast<float>(titleSize), textMeasureFactor)};
    Vector2 serverDimensions { MeasureTextEx(GetFontDefault(),textServer, static_cast<float>(optionSize), textMeasureFactor)};
    Vector2 clientDimensions { MeasureTextEx(GetFontDefault(),textClient, static_cast<float>(optionSize), textMeasureFactor)};

    float titleX  {screenWidth/2 - titleDimensions.x/2};
    float titleY  {75};
    float serverX {screenWidth/2 - serverDimensions.x/2}; 
    float serverY {200};
    float clientX {screenWidth/2 - clientDimensions.x/2};
    float clientY {300};

    auto titleColor {WHITE};
    auto serverColor{WHITE};
    auto clientColor{WHITE};

    //setup input box
    Rectangle inputBox   {screenWidth/2 - screenWidth/4, clientY + 75, screenWidth/2, 35};
    auto inputColorBorder{WHITE};
    auto inputColorFill  {BLACK};
    auto inputColorText  {WHITE};
    int inputSize        {20};

    createInputBox(inputBox, inputColorBorder, inputColorFill, inputColorText, inputSize, mousePos, ipAddrStr, inputSelected);


    //check for hover and click events
    if(isMouseInRect(mousePos, serverX, serverY, serverDimensions)){
        serverColor = GREEN;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            runMode = 2;
            requestState = CHAT;
        }
    }else{
        serverColor = WHITE;
    }
    if(isMouseInRect(mousePos, clientX, clientY, clientDimensions)){
        clientColor = GREEN;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            runMode = 1;
            requestState = CHAT;
        }
    }else{
        clientColor = WHITE;
    }

    DrawText(TextFormat(textTitle), titleX, titleY, titleSize, titleColor);
    DrawText(TextFormat(textServer), serverX, serverY, optionSize, serverColor);
    DrawText(TextFormat(textClient), clientX, clientY, optionSize, clientColor);

    const char * textWarning {"WIP - Game will freeze while connecting, this is expected."};
    Vector2 warningDimensions { MeasureTextEx(GetFontDefault(),textWarning, static_cast<float>(inputSize), textMeasureFactor)};
    DrawText(TextFormat(textWarning), screenWidth/2 - warningDimensions.x/2, 560, inputSize, RED);

    //Display Connecting status
    std::string textConnecting;

    switch(runMode)
    {
        case 1:
        {
            textConnecting = "Attempting to connect to server...";
        }break;
        case 2:
        {
            textConnecting = "Waiting for client to connect...";
        }
    }

    if(runMode > 0 && runMode < 3)
    {
        Vector2 connectingDimensions { MeasureTextEx(GetFontDefault(),textConnecting.c_str(), static_cast<float>(optionSize), textMeasureFactor)};
        DrawText(TextFormat(textConnecting.c_str()), screenWidth/2 - connectingDimensions.x/2, 500, optionSize, YELLOW);
    }

    EndDrawing();
    
}

void levelChat(std::string& latestMessage, bool& inputSelected, std::string& inputString, 
                int chatHistoryLength, boost::circular_buffer<std::string>& chatBuffer, networkInstance& networkHandler)
{
    gamePaused = true;
    //draw
    BeginDrawing();
    ClearBackground(BLACK);

    const char * textTitle {"Chat"};
    const char * textReturn {"Return to title"};
    const char * textStart {"Start!"};

    float titleSize {50};
    float optionSize {30};

    //Vector2 titleDimensions { MeasureTextEx(GetFontDefault(),mainMenuTitle, static_cast<float>(titleSize), 1)};
    Vector2 titleDimensions { MeasureTextEx(GetFontDefault(),textTitle, static_cast<float>(titleSize), textMeasureFactor)};
    Vector2 startDimensions { MeasureTextEx(GetFontDefault(),textStart, static_cast<float>(optionSize), textMeasureFactor)};
    Vector2 returnDimensions { MeasureTextEx(GetFontDefault(),textReturn, static_cast<float>(optionSize), textMeasureFactor)};

    float titleX {screenWidth/2 - titleDimensions.x/2};
    float titleY {75};
    float startX {screenWidth/2 - startDimensions.x - 50};
    float startY {500};
    float returnX {screenWidth - returnDimensions.x - 50};
    float returnY {550};

    auto titleColor{WHITE};
    auto startColor{WHITE};
    auto returnColor{WHITE};

    Vector2 mousePos{GetMousePosition()};

    //set up input box
    auto inputColorBorder{WHITE};
    auto inputColorFill{BLACK};
    auto inputColorText{WHITE};
    int inputSize {20};
    Rectangle inputBox{screenWidth/2 - 0.75*screenWidth/2, 450, 0.75*screenWidth, 35};
    //create, configure and draw input box
    createInputBox(inputBox,inputColorBorder, inputColorFill, inputColorText, inputSize, mousePos, inputString, inputSelected);

    //draw chat box outer
    Rectangle chatHistory{inputBox.x, inputBox.y - 320, inputBox.width, 300};
    DrawRectangleLines(chatHistory.x, chatHistory.y, chatHistory.width, chatHistory.height, GRAY);
    //fill with text
    int chatHistorySize = 20;
    for (int i = 0; i < chatHistoryLength; i++)
    {
        //draw the text found in the circular buffer
        DrawText(chatBuffer[i].c_str(), chatHistory.x + 10, chatHistory.y + 4 + (30*i), chatHistorySize, WHITE);
    }

    //check for send command
    if(inputSelected)
    {
        if(IsKeyPressed(KEY_ENTER) && !inputString.empty())
        {
            //serialise, get header, then send message
            std::string msg {getSerialisedStr(inputString)};
            std::string msgHeader {getSerialisedStrHeader(msg.size(), M_STR)};
            networkHandler.sendMessage(msgHeader, msg);
            //store text into the circular buffer
            chatBuffer.push_back("Me: " + inputString);
            //clear text
            inputString.clear();
        }
    }

    
    //check for hover and click events
    if(isMouseInRect(mousePos, startX, startY, startDimensions)){
        startColor = GREEN;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            requestState = LEVEL1;
            //send event
            eventMessage msg;
            msg.eValue = LEVEL1;
            msg.eType = E_SCREEN;
            //serialise and send
            std::string sMsg{getSerialisedStr(msg)};
            std::string sMsgHeader{getSerialisedStrHeader(sMsg.size(), M_EVENT)};
            networkHandler.sendMessage(sMsgHeader,sMsg);
        }
    }else{
        startColor = WHITE;
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
    DrawText(TextFormat(textReturn), returnX, returnY, optionSize, returnColor);
    DrawText(TextFormat(textStart), startX, startY, optionSize, startColor);

    EndDrawing();
    
}

void createInputBox(Rectangle inputBox, Color& borderColor, Color& fillColor, Color& textColor, int textSize, Vector2 mousePos, std::string& tempString, bool& isSelected )
{
//check for hover and click events
    if(isMouseInRect(mousePos, inputBox.x, inputBox.y, {inputBox.width, inputBox.height})){
        SetMouseCursor(MOUSE_CURSOR_IBEAM);
    }else{
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }

    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){// click event
        if(isMouseInRect(mousePos, inputBox.x, inputBox.y, {inputBox.width, inputBox.height})){
            isSelected = true;
        }else{
            isSelected = false;
        }
    }

    if(isSelected){
        int key = GetCharPressed();
        borderColor = GREEN;
        fillColor = GRAY;
        while (key > 0) //check for multiple keypresses at once
        {
            if ((key >= 32) && (key <= 125)){
                tempString += (char)key;
            }
            key = GetCharPressed();
        }
        //handle backspace
        if (IsKeyPressed(KEY_BACKSPACE))
        {
            if(!tempString.empty()){
                tempString.erase(tempString.size() - 1);
            }
        }
    }else{
        borderColor = WHITE;
        fillColor = BLACK;
    }

    // Vector2 inputDimensions { MeasureTextEx(GetFontDefault(),tempString.c_str(), static_cast<float>(textSize), textMeasureFactor)};

    //Draw rectangle outline, then filling
    DrawRectangleRec(inputBox, fillColor);
    DrawRectangleLines(inputBox.x, inputBox.y, inputBox.width, inputBox.height, borderColor);
    DrawText(tempString.c_str(), inputBox.x + 10, inputBox.y + 8, textSize, textColor);
}

void createClickableText()
{

}