#include<stdlib.h>
#include<pthread.h>
#include<stdio.h>
#include<stdbool.h>
#include<unistd.h>
#include<sys/syscall.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<raylib.h>

const int screenWidth = 800;
const int screenHeight = 600;

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

}ballInfo;

ballInfo ballPosConstructor(Rectangle * pl1, Rectangle *pl2)   //fake constructor
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
    
    pp.ptr = malloc(sizeof(Rectangle));
    *(pp.ptr) = (Rectangle){ screenWidth/2, screenHeight/2 - 25, 25, 25 };

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
        if(obj->right)                          //main ball movement
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
            obj->up = false;
            obj->down = true;
        }
        else if(obj->ptr->y >= screenHeight - 25)        //ball size cuz top left corner point is checked
        {
            obj->up = true;
            obj->down = false;
        }

        if(obj->ptr->x <= 0)
        {
            usleep(100000);
            obj->start = true;
            obj->ptr->x = screenWidth/ 2 - 12;
            obj->ptr->y = screenHeight/ 2 - 12;
            obj->p1->y = (screenHeight/2) - (185/2);
            obj->p1->x = screenWidth - 50;
            obj->p2->y = (screenHeight/2) - (185/2);
            obj->p2->x = 50-30;
            obj->left = false;
            obj->right = true;
            obj->up = true;
            obj->down = false;
        }
        else if(obj->ptr->x >= screenWidth - 25)        //ball size cuz top left corner point is checked
        {
            usleep(100000);
            obj->start = true;
            obj->ptr->x = screenWidth/ 2 - 12;
            obj->ptr->y = screenHeight/ 2 - 12;
            obj->p1->y = (screenHeight/2) - (185/2);
            obj->p1->x = screenWidth - 50;
            obj->p2->y = (screenHeight/2) - (185/2);
            obj->p2->x = 50-30;
            obj->left = false;
            obj->right = true;
            obj->up = true;
            obj->down = false;
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
        if(CheckCollisionRecs(*obj->ptr, *obj->p1))
        {
            obj->p1Contact = true;
            obj->p2Contact = false;
            obj->left = true;
            obj->right = false;
        }
        else if(CheckCollisionRecs(*obj->ptr, *obj->p2))
        {
            obj->p2Contact = true;
            obj->p1Contact = false;
            obj->left = false;
            obj->right = true;
        }
        usleep(16000);
    }
    return NULL;
}

void* p1Move(void* o)          //thread
{
    ballInfo *obj = (ballInfo*)o;

    int speed = 4;

    while(!obj->start)
    {
        if(IsKeyDown(KEY_UP) && obj->p1->y >= 0)
        {
            obj->p1->y -= speed;
        }
        else if(IsKeyDown(KEY_DOWN) && obj->p1->y <= screenHeight - 185)
        {
            obj->p1->y+=speed;
        }

        usleep(16000);
    }

    return NULL;
}

void* p2Move(void* o)          //thread
{
    ballInfo *obj = (ballInfo*)o;

    int speed = 4;

    while(!obj->start)
    {
        if(IsKeyDown(KEY_W) && obj->p2->y >= 0)
        {
            obj->p2->y-=speed;
        }
        else if(IsKeyDown(KEY_S) && obj->p2->y <= screenHeight - 185)
        {
            obj->p2->y+=speed;
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
    
    //Texture2D player = LoadTexture("Assets\\SHIP1.png");

    //---------------------------------------SOUNDS-------------------------------------------------
    //Sound heroic = LoadSound("Assets\\Titan GIGANTO.mp3");

    SetTargetFPS(60);               // Set desired framerate (frames-per-second)

    //Initialize and open main game window
    
    //---------------------------------------Player----------------------------------------------
    Rectangle p1 = { screenWidth - 50,(screenHeight/2) - (185/2),30,185 };    //x position, y positon, length, height
    //we subtract half the length of the bar from half of height so that the centre of the bar is placed at half of screen height
    Rectangle p2 = { 50-30,(screenHeight/2) - (185/2),30,185 };    //x position, y positon, length, height
    //we substract length of bar from the gap because same was being added in the rightSpeed one (box starts rendering from top leftSpeed corner)
    Rectangle mid = { screenWidth/2 - 1,0,2,screenHeight };


    //--------------------------------------initial values------------------------------------
    ballInfo ballObj = ballPosConstructor(&p1, &p2);
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

    int kakappoo;
    //SOUND PLAYBACK CALLS
    
    int screen = 0;
    
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
                    if(IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
                    {
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
                    }
                    startBlink++;
                }
                else if(ballObj.start && !initStart)    //then, after 1st round, first join the previous running threads then create them again for next round when prompted
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
                        // Mark threads as not running
                        ballObj.threadsRunning = false; 
                    }

                    if(IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
                    {
                        ballObj.start = false;
                        // Mark threads as running
                        ballObj.threadsRunning = true; 

                        rounds++;
                        pthread_create(&BallDirectionTID, NULL, &ballMovement, &ballObj);
                        pthread_create(&ballBoundariesCheckTID, NULL, &ballBoundariesCheck, &ballObj);
                        pthread_create(&scoreUpdaterTID, NULL, &scoreUpdater, &sc);
                        pthread_create(&ballCollisionTID, NULL, &ballCollision, &ballObj);
                        pthread_create(&p1MoveTID, NULL, &p1Move, &ballObj);
                        pthread_create(&p2MoveTID, NULL, &p2Move, &ballObj);
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
                DrawRectangleRec(p1, WHITE);
                DrawRectangleRec(p2, WHITE);
                
                DrawText( intToString(sc.p1), 50, 30, 25, WHITE);
                DrawText( intToString(sc.p2), screenWidth - (50 + 25), 30, 25, WHITE);

                if(!ballObj.start)
                {
                    DrawRectangleRec(mid, LIGHTGRAY);

                    DrawText( "Round ", screenWidth/2 - 100, 30, 50, WHITE);
                    DrawText( intToString(rounds), screenWidth/2 + 70, 30, 50, WHITE);

                    DrawRectangleRec(*(ballObj.ptr), WHITE);
                }
                else
                {
                    if(startBlink <= 10)
                    {
                        DrawText( "Start", screenWidth/2 - 200, screenHeight/2 - 150, 150, WHITE);
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