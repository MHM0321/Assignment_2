#include<stdlib.h>
#include<pthread.h>
#include<stdio.h>
#include<stdbool.h>
#include<unistd.h>
#include<sys/syscall.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<raylib.h>
#include <time.h>

const int screenWidth = 800;
const int screenHeight = 600;

int getRandomNumber(int min, int max)
{
    return min + rand() % (max - min + 1);
}

int getRandomBooster()
{
    return rand() % 4;
}

typedef struct ballInfo
{
    //movement directions
    bool left;
    bool right;
    bool up;
    bool down;

    //round separater/ state
    bool start;
    bool threadsRunning;  //to track if threads are running for proper joins

    Rectangle *ptr; //ball

    Rectangle *p1;      //player 1
    Rectangle *p2;      //player 2

    //for boosters
    bool p1Contact;     
    bool p2Contact;

    Sound reflect;
    Sound round;
    Sound good;
    Sound bad;
    Sound speedSound;
    Sound strong;
    Sound blast;

    Rectangle *big;         //enlarge booster
    Rectangle *small;       //shrink booster
    Rectangle *speed;       //increase player paddle speed
    Rectangle *powerShot;

    bool spawned;       //booster call flag
    bool sizeChanged;       //to avoid race conditions
    int boosterSpawnDelayer;

    int p1Speed;
    int p2Speed;

    bool p1Power;
    bool p2Power;

    bool p1shootOff;
    bool p2shootOff;

}ballInfo;

ballInfo ballPosConstructor(Rectangle * pl1, Rectangle *pl2, Rectangle *b, Rectangle *s, Rectangle *sp, Rectangle *ps)   //fake constructor
{
    ballInfo pp;

    pp.start = true;
    pp.threadsRunning = false;
    pp.right = true;
    pp.up = true;
    pp.left = false;
    pp.down = false;
    pp.p1 = pl1;
    pp.p2 = pl2;
    pp.p1Contact = false;
    pp.p2Contact = false;
    pp.big = b;
    pp.small = s;
    pp.speed = sp;
    pp.powerShot = ps;
    pp.spawned = false;
    pp.sizeChanged = false;
    pp.boosterSpawnDelayer = 0;
    pp.p1Speed = 4;
    pp.p2Speed = 4;
    pp.p1Power = false;
    pp.p2Power = false;
    pp.p1shootOff = false;
    pp.p2shootOff = false;
    
    pp.ptr = malloc(sizeof(Rectangle));
    *(pp.ptr) = (Rectangle){ screenWidth/2, screenHeight/2 - 25, 25, 25 };

    pp.reflect = LoadSound("reflect.mp3");
    pp.round = LoadSound("round.mp3");
    pp.good = LoadSound("good.mp3");
    pp.bad = LoadSound("bad.mp3");
    pp.speedSound = LoadSound("fast.mp3");
    pp.strong = LoadSound("powah.mp3");
    pp.blast = LoadSound("blast.mp3");

    return pp;
}

typedef struct score
{
    int p1;
    int p2;

    ballInfo * ptr;
    bool scored;
}score;

score scoreConstructor(ballInfo *obj2)   //fake constructor
{
    score obj;
    obj.p1 = 0;
    obj.p2 = 0;
    obj.ptr = obj2;
    obj.scored = false;

    return obj;
}

void* ballMovement(void* info)          //thread
{
    ballInfo * obj = (ballInfo *) info;

    int speedX = 5;
    int speedY = 2;
    
    while (!obj->start)
    {
        if(obj->p1shootOff)
        {
            obj->ptr->x -= 30;
        }
        else if(obj->p2shootOff)
        {
            obj->ptr->x += 30;
        }
        else if(obj->right)                          //main ball movement
        {
            if(obj->up)
            {
                obj->ptr->x += speedX;
                obj->ptr->y -= speedY;
            }
            else if(obj->down)
            {
                obj->ptr->x += speedX;
                obj->ptr->y += speedY;
            }
        }
        else if(obj->left)
        {
            if(obj->up)
            {
                obj->ptr->x -= speedX;
                obj->ptr->y -= speedY;
            }
            else if(obj->down)
            {
                obj->ptr->x -= speedX;
                obj->ptr->y += speedY;
            }
        }
        usleep(16000);
    }
    return NULL;
}

void* ballBoundariesCheck(void* info)          //thread
{
    ballInfo * obj = (ballInfo *) info;

    while(!obj->start)
    {
        if(obj->ptr->y <= 0)
        {
            PlaySound(obj->reflect);

            obj->up = false;
            obj->down = true;
        }
        else if(obj->ptr->y >= screenHeight - 25)        //ball size cuz top left corner point is checked
        {
            PlaySound(obj->reflect);

            obj->up = true;
            obj->down = false;
        }

        if(obj->ptr->x <= 0)
        {
            PlaySound(obj->round);
            
            usleep(100000);
            obj->start = true;
            obj->ptr->x = screenWidth/ 2 - 12;
            obj->ptr->y = screenHeight/ 2 - 12;
            obj->p1->y = (screenHeight/2) - (185/2);
            obj->p1->x = screenWidth - 50;
            obj->p1->width = 30;
            obj->p1->height = 185;
            obj->p2->y = (screenHeight/2) - (185/2);
            obj->p2->x = 50-30;
            obj->p2->width = 30;
            obj->p2->height = 185;
            obj->boosterSpawnDelayer = 0;
            obj->p1Speed = 4;
            obj->p2Speed = 4;
            obj->big->x = -100;
            obj->big->y = -100;
            obj->small->x = -200;
            obj->small->y = -200;
            obj->speed->x = -300;
            obj->speed->y = -300;
            obj->powerShot->x = -400;
            obj->powerShot->y = -400;
            obj->spawned = false;
            obj->left = false;
            obj->right = true;
            obj->up = true;
            obj->down = false;
            obj->p1Power = false;
            obj->p2Power = false;
            obj->p1shootOff = false;
            obj->p2shootOff = false;
        }
        else if(obj->ptr->x >= screenWidth - 25)        //ball size cuz top left corner point is checked
        {
            PlaySound(obj->round);
            
            usleep(100000);
            obj->start = true;
            obj->ptr->x = screenWidth/ 2 - 12;
            obj->ptr->y = screenHeight/ 2 - 12;
            obj->p1->y = (screenHeight/2) - (185/2);
            obj->p1->x = screenWidth - 50;
            obj->p1->width = 30;
            obj->p1->height = 185;
            obj->p2->y = (screenHeight/2) - (185/2);
            obj->p2->x = 50-30;
            obj->p2->width = 30;
            obj->p2->height = 185;
            obj->boosterSpawnDelayer = 0;
            obj->p1Speed = 4;
            obj->p2Speed = 4;
            obj->big->x = -100;
            obj->big->y = -100;
            obj->small->x = -200;
            obj->small->y = -200;
            obj->speed->x = -300;
            obj->speed->y = -300;
            obj->powerShot->x = -400;
            obj->powerShot->y = -400;
            obj->spawned = false;
            obj->left = false;
            obj->right = true;
            obj->up = true;
            obj->down = false;
            obj->p1Power = false;
            obj->p2Power = false;
            obj->p1shootOff = false;
            obj->p2shootOff = false;
        }
        usleep(16000);
    }
    return NULL;
}

void* scoreUpdater(void* s)          //thread
{
    score * obj = (score *)s;

    while(!obj->ptr->start)
    {
        if(obj->ptr->ptr->x <= 0 && !obj->scored)
        {
            obj->p1++;
            obj->scored = true;
        }
        else if(obj->ptr->ptr->x >= screenWidth - 25 && !obj->scored)        //ball size cuz top left corner point is checked
        {
            obj->p2++;
            obj->scored = true;
        }

        if(obj->ptr->ptr->x > 0 && obj->ptr->ptr->x < screenWidth - 25)
        {
            obj->scored = false;
        }
        usleep(16000);
    }
    return NULL;
}

void* ballCollision(void* o)          //thread
{
    ballInfo* obj = (ballInfo*)o;

    while(!obj->start)
    {
        if(CheckCollisionRecs(*obj->ptr, *obj->p1) && !obj->p1Power)
        {
            PlaySound(obj->reflect);

            obj->p2shootOff = false;
            obj->p1Contact = true;
            obj->p2Contact = false;
            obj->left = true;
            obj->right = false;
        }
        else if(CheckCollisionRecs(*obj->ptr, *obj->p1) && obj->p1Power)
        {
            obj->p1shootOff = true;
            obj->p1Contact = true;
            obj->p2Contact = false;
        }
        else if(CheckCollisionRecs(*obj->ptr, *obj->p2) && !obj->p2Power)
        {
            PlaySound(obj->reflect);

            obj->p1shootOff = false;
            obj->p2Contact = true;
            obj->p1Contact = false;
            obj->left = false;
            obj->right = true;
        }
        else if(CheckCollisionRecs(*obj->ptr, *obj->p2) && obj->p2Power)
        {
            obj->p2shootOff = true;
            obj->p2Contact = true;
            obj->p1Contact = false;
        }
        else if(CheckCollisionRecs(*obj->ptr, *obj->big) && obj->p1Contact && !obj->sizeChanged)
        {
            PlaySound(obj->good);
            obj->spawned = false;
            obj->big->x = -100;
            obj->big->y = -100;
            obj->p1->height += 50;
            obj->sizeChanged = true;
        }
        else if(CheckCollisionRecs(*obj->ptr, *obj->big) && obj->p2Contact && !obj->sizeChanged)
        {
            PlaySound(obj->good);
            obj->spawned = false;
            obj->big->x = -100;
            obj->big->y = -100;
            obj->p2->height += 50;
            obj->sizeChanged = true;
        }
        else if(CheckCollisionRecs(*obj->ptr, *obj->small) && obj->p1Contact && !obj->sizeChanged)
        {
            PlaySound(obj->bad);
            obj->spawned = false;
            obj->small->x = -200;
            obj->small->y = -200;
            obj->p1->height -= 50;
            obj->sizeChanged = true;
        }
        else if(CheckCollisionRecs(*obj->ptr, *obj->small) && obj->p2Contact && !obj->sizeChanged)
        {
            PlaySound(obj->bad);
            obj->spawned = false;
            obj->small->x = -200;
            obj->small->y = -200;
            obj->p2->height -= 50;
            obj->sizeChanged = true;
        }
        else if(CheckCollisionRecs(*obj->ptr, *obj->speed) && obj->p1Contact)
        {
            PlaySound(obj->speedSound);
            obj->spawned = false;
            obj->speed->x = -300;
            obj->speed->y = -300;
            obj->p1Speed = 16;
        }
        else if(CheckCollisionRecs(*obj->ptr, *obj->speed) && obj->p2Contact)
        {
            PlaySound(obj->speedSound);
            obj->spawned = false;
            obj->speed->x = -300;
            obj->speed->y = -300;
            obj->p2Speed = 16;
        }
        else if(CheckCollisionRecs(*obj->ptr, *obj->powerShot) && obj->p1Contact)
        {
            PlaySound(obj->strong);
            obj->spawned = false;
            obj->speed->x = -400;
            obj->speed->y = -400;
            obj->p1Power = true;
        }
        else if(CheckCollisionRecs(*obj->ptr, *obj->powerShot) && obj->p2Contact)
        {
            PlaySound(obj->strong);
            obj->spawned = false;
            obj->speed->x = -400;
            obj->speed->y = -400;
            obj->p2Power = true;
        }


        usleep(16000);
    }
    return NULL;
}

void* p1Move(void* o)          //thread
{
    ballInfo *obj = (ballInfo*)o;

    while(!obj->start)
    {
        if(IsKeyDown(KEY_UP) && obj->p1->y >= 0)
        {
            obj->p1->y -= obj->p1Speed;
        }
        else if(IsKeyDown(KEY_DOWN) && (obj->p1->height + obj->p1->y) <= screenHeight)
        {
            obj->p1->y+=obj->p1Speed;
        }

        usleep(16000);
    }

    return NULL;
}

void* p2Move(void* o)          //thread
{
    ballInfo *obj = (ballInfo*)o;

    while(!obj->start)
    {
        if(IsKeyDown(KEY_W) && obj->p2->y >= 0)
        {
            obj->p2->y-= obj->p2Speed;
        }
        else if(IsKeyDown(KEY_S) && (obj->p2->height + obj->p2->y) <= screenHeight)
        {
            obj->p2->y+= obj->p2Speed;
        }

        usleep(16000);
    }

    return NULL;
}

void* spawnBoosters(void* o)          //thread
{
    ballInfo *obj = (ballInfo*)o;

    while(!obj->start)
    {
        if(!obj->spawned)
        {
            obj->boosterSpawnDelayer++;
            if(obj->boosterSpawnDelayer >= 370)
            {
                obj->boosterSpawnDelayer = 0;
                obj->sizeChanged = false;       //paddle ready for change in size
                obj->spawned = true;
                srand(time(0));
                int randomPositionX = getRandomNumber(screenWidth/2-20, screenWidth/2+20);

                srand(time(0));
                int randomPositionY = getRandomNumber(100, screenHeight - 100);

                srand(time(0));
                int boostType = getRandomBooster();

                if(boostType == 0)
                {
                    obj->big->x = randomPositionX;
                    obj->big->y = randomPositionY;
                }
                else if(boostType == 1)
                {
                    obj->small->x = randomPositionX;
                    obj->small->y = randomPositionY;
                }
                else if(boostType == 2)
                {
                    obj->speed->x = randomPositionX;
                    obj->speed->y = randomPositionY;
                }
                else if(boostType == 3)
                {
                    obj->powerShot->x = randomPositionX;
                    obj->powerShot->y = randomPositionY;
                }

            }
        }
        usleep(16000);
    }

    return NULL;
}

const char* intToString(int num)
{
    static char buffer[20];
    snprintf(buffer, sizeof(buffer), "%d", num);
    return buffer;
}


int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------

    InitWindow(screenWidth, screenHeight, "Ping Pong");

    // TODO: Initialize all required variables and load all required data here!
    InitAudioDevice();
    //---------------------------------------Textures-------------------------------------------------
    Texture2D mush = LoadTexture("mush.png");
    Texture2D skull = LoadTexture("skull.png");
    Texture2D fest = LoadTexture("fast.png");
    Texture2D mooscles = LoadTexture("powah.png");

    //---------------------------------------SOUNDS-------------------------------------------------
    Sound welcome = LoadSound("welcome.mp3");
    Sound start = LoadSound("start.mp3");

    SetTargetFPS(60);               // Set desired framerate (frames-per-second)

    //Initialize and open main game window
    
    //---------------------------------------Player----------------------------------------------
    Rectangle p1 = { screenWidth - 50,(screenHeight/2) - (185/2),30,185 };    //x position, y positon, length, height
    //we subtract half the length of the bar from half of height so that the centre of the bar is placed at half of screen height
    Rectangle p2 = { 50-30,(screenHeight/2) - (185/2),30,185 };    //x position, y positon, length, height
    //we substract length of bar from the gap because same was being added in the rightSpeed one (box starts rendering from top leftSpeed corner)
    Rectangle mid = { screenWidth/2 - 1,0,2,screenHeight };

    Rectangle big = {-100,-100, 70, 70};            //boosters
    Rectangle smol = {-200,-200, 70, 70};
    Rectangle sped = {-300,-300, 70, 70};
    Rectangle powah = {-400,-400, 70, 70};

    //--------------------------------------initial values------------------------------------
    ballInfo ballObj = ballPosConstructor(&p1, &p2, &big, &smol, &sped, &powah);
    pthread_t BallDirectionTID;
    pthread_t ballBoundariesCheckTID;
    ballObj.right = true;
    ballObj.up = true;
    ballObj.left = false;
    ballObj.down = false;

    pthread_t scoreUpdaterTID;
    score sc = scoreConstructor(&ballObj);
    
    pthread_t ballCollisionTID;
    pthread_t p1MoveTID;
    pthread_t p2MoveTID;

    int startBlink = 0;
    bool initStart = true;
    int rounds = 0;

    pthread_t boostSpawnTID;


    //SOUND PLAYBACK CALLS
    bool welc = false;

    //screens
    int screen = 0;
    bool end = false;
    
    //--------------------------------------------------------------------------------------

    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update       (keep threads running for each round, do not create for each frame but rather make them for each round)
        //----------------------------------------------------------------------------------
        switch (screen)
        {
            case 0:
            {
                if(ballObj.start && initStart)  //at start of the game i.e. 1st round, only create all the threads initially when prompted
                {

                    if(!welc)
                    {
                        PlaySound(welcome);
                        welc = true;
                    }

                    if(IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
                    {
                        StopSound(welcome);
                        PlaySound(start);
                        
                        ballObj.start = false;
                        initStart = false;
                        ballObj.threadsRunning = true;  // Mark threads as running

                        rounds++;
                        pthread_create(&BallDirectionTID, NULL, &ballMovement, &ballObj);
                        pthread_create(&ballBoundariesCheckTID, NULL, &ballBoundariesCheck, &ballObj);
                        pthread_create(&scoreUpdaterTID, NULL, &scoreUpdater, &sc);
                        pthread_create(&ballCollisionTID, NULL, &ballCollision, &ballObj);
                        pthread_create(&p1MoveTID, NULL, &p1Move, &ballObj);
                        pthread_create(&p2MoveTID, NULL, &p2Move, &ballObj);
                        pthread_create(&boostSpawnTID, NULL, &spawnBoosters, &ballObj);
                    }
                    startBlink++;
                }
                else if(ballObj.start && !initStart  && !end)    //then, after 1st round, first join the previous running threads then create them again for next round when prompted
                {
                    // Only join threads if they're currently running, this prevents from threads joining even before they are created
                    if(ballObj.threadsRunning)
                    {
                        pthread_join(BallDirectionTID, NULL);
                        pthread_join(ballBoundariesCheckTID, NULL);
                        pthread_join(scoreUpdaterTID, NULL);
                        pthread_join(ballCollisionTID, NULL);
                        pthread_join(p1MoveTID, NULL);
                        pthread_join(p2MoveTID, NULL);
                        pthread_join(boostSpawnTID, NULL);
                        // Mark threads as not running
                        ballObj.threadsRunning = false; 
                    }

                    if(IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
                    {
                        ballObj.start = false;
                        // Mark threads as running
                        ballObj.threadsRunning = true; 

                        PlaySound(start);

                        rounds++;
                        pthread_create(&BallDirectionTID, NULL, &ballMovement, &ballObj);
                        pthread_create(&ballBoundariesCheckTID, NULL, &ballBoundariesCheck, &ballObj);
                        pthread_create(&scoreUpdaterTID, NULL, &scoreUpdater, &sc);
                        pthread_create(&ballCollisionTID, NULL, &ballCollision, &ballObj);
                        pthread_create(&p1MoveTID, NULL, &p1Move, &ballObj);
                        pthread_create(&p2MoveTID, NULL, &p2Move, &ballObj);
                        pthread_create(&boostSpawnTID, NULL, &spawnBoosters, &ballObj);
                    }
                    startBlink++;
                }

                break;
            }


        
            default:
            {   
                break;
            }
        }
        //----------------------------------------------------------------------------------





        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(BLACK);

        switch (screen)
        {
        
            case 0:
            {
                if(!ballObj.p1Power)
                {
                    DrawRectangleRec(p1, GREEN);
                }
                else
                {
                    DrawRectangleRec(p1, GOLD);
                }

                if(!ballObj.p2Power)
                {
                    DrawRectangleRec(p2, MAGENTA);
                }
                else
                {
                    DrawRectangleRec(p2, GOLD);
                }
                
                
                DrawText( intToString(sc.p2), 50, 30, 25, WHITE);
                DrawText( intToString(sc.p1), screenWidth - (50 + 25), 30, 25, WHITE);

                if(sc.p1 == 10)
                {
                    if(ballObj.threadsRunning)
                    {
                        end = true;

                        pthread_join(BallDirectionTID, NULL);
                        pthread_join(ballBoundariesCheckTID, NULL);
                        pthread_join(scoreUpdaterTID, NULL);
                        pthread_join(ballCollisionTID, NULL);
                        pthread_join(p1MoveTID, NULL);
                        pthread_join(p2MoveTID, NULL);
                        pthread_join(boostSpawnTID, NULL);
                        // Mark threads as not running
                        ballObj.threadsRunning = false; 
                    }

                    DrawText( "P1 Wins !!!", screenWidth/2 - 200, screenHeight/2 - 150, 100, GREEN);
                }
                else if(sc.p2 == 10)
                {
                    if(ballObj.threadsRunning)
                    {
                        end = true;

                        pthread_join(BallDirectionTID, NULL);
                        pthread_join(ballBoundariesCheckTID, NULL);
                        pthread_join(scoreUpdaterTID, NULL);
                        pthread_join(ballCollisionTID, NULL);
                        pthread_join(p1MoveTID, NULL);
                        pthread_join(p2MoveTID, NULL);
                        pthread_join(boostSpawnTID, NULL);
                        // Mark threads as not running
                        ballObj.threadsRunning = false; 
                    }

                    DrawText( "P2 Wins !!!", screenWidth/2 - 200, screenHeight/2 - 150, 100, MAGENTA);
                }

                if(!ballObj.start && !end)
                {
                    for (int y = 0; y < screenHeight; y += 20)
                    {
                        DrawRectangle(screenWidth / 2 - 1, y, 2, 10, LIGHTGRAY);
                    }

                    Color roundColor = { 0, 255, 255, 255 };
                    DrawText( "Round ", screenWidth/2 - 100, 30, 50, roundColor);
                    DrawText( intToString(rounds), screenWidth/2 + 70, 30, 50, roundColor);

                    if(ballObj.p1Contact)
                    {
                        DrawRectangleRec(*(ballObj.ptr), GREEN);
                    }
                    else if(ballObj.p2Contact)
                    {
                        DrawRectangleRec(*(ballObj.ptr), MAGENTA);
                    }
                    else
                    {
                        DrawRectangleRec(*(ballObj.ptr), WHITE);
                    }
                    

                    if(ballObj.spawned)
                    {
                        DrawRectangleRec(*ballObj.big,BLACK);
                        DrawTexturePro(mush,(Rectangle){ 0, 0, mush.width, mush.height }, *ballObj.big, (Vector2){ 0, 0 },0,WHITE);

                        DrawRectangleRec(*ballObj.small,BLACK);
                        DrawTexturePro(skull,(Rectangle){ 0, 0, skull.width, skull.height }, *ballObj.small, (Vector2){ 0, 0 },0,WHITE);

                        DrawRectangleRec(*ballObj.speed,BLACK);
                        DrawTexturePro(fest,(Rectangle){ 0, 0, fest.width, fest.height }, *ballObj.speed, (Vector2){ 0, 0 },0, PURPLE);

                        DrawRectangleRec(*ballObj.powerShot,BLACK);
                        DrawTexturePro(mooscles,(Rectangle){ 0, 0, mooscles.width, mooscles.height }, *ballObj.powerShot, (Vector2){ 0, 0 },0, GOLD);
                    }
                }
                else if(!end)
                {
                    if(startBlink <= 10)
                    {
                        Color startColor = { 0, 255, 255, 255 };
                        DrawText( "Start", screenWidth/2 - 200, screenHeight/2 - 150, 150, startColor);
                    }
                    else if(startBlink < 20 && startBlink > 10)
                    {}
                    else if(startBlink >= 20)
                    {
                        startBlink = 0;
                    }
                }

                break;
            }

            default:
            {
                break;
            }
        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    
    // Make sure to join any running threads before exiting
    if(ballObj.threadsRunning) {
        ballObj.start = true;  // Signal threads to stop
        usleep(50000);  // Give threads time to notice the signal
        pthread_join(BallDirectionTID, NULL);
        pthread_join(ballBoundariesCheckTID, NULL);
        pthread_join(scoreUpdaterTID, NULL);
        pthread_join(ballCollisionTID, NULL);
        pthread_join(p1MoveTID, NULL);
        pthread_join(p2MoveTID, NULL);
    }
    
    //UnloadTexture(one);
    
    //UnloadSound(op);
    // TODO: Unload all loaded data (textures, fonts, audio) here!

    //free pointers
    free(ballObj.ptr);

    CloseWindow();        // Close window and OpenGL context
    CloseAudioDevice();
    //--------------------------------------------------------------------------------------

    return 0;
}