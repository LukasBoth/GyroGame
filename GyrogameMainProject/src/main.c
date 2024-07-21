/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "RA8875Lib.h"
#include "SensorLib.h"
#include "Map.h"

//Defines
#define DISPLAY_WIDTH 800
#define DISPLAY_HEIGHT 480
#define BALL_RADIUS 10
#define MOVEMENT_SENSTIVITY_LIMIT 1
#define MAX_SPEED_LIMIT 50 //Maximum Amount of Pixel which the Ball can move in one frame
#define SPEED_SCALER 10 //SPEED is devieded by this Scaler
#define SCREEN_BRIGHTNESS 200 // Controlles the Brightness of the Display min is 0 max is 255
#define START_POSITONXY 10
#define OFFSET_PIXEL 1
//Defines for Quadrant after classical Cartesian Coordianaten System with extra cases for straight movement
#define QUADRANT1 0
#define QUADRANT2 1
#define QUADRANT3 2
#define QUADRANT4 3
#define X_POSITIV 4
#define X_NEGATIV 5
#define Y_POSITIV 6
#define Y_NEGATIV 7

//The coordinatesystem is positiv from the top left corner of the display and ranges from 0 to 480 on the y axis and 0 to 800 on the x axis
//Global Variables
uint8_t InterruptCounter=50;
//Array to safe and access Data of the Sensor for x-y-z Axis
int16_t AccelData[3]={0};
uint32_t GameTimer=0; //Times Game in 25ms steps
 bool GameState=false;//1= Victory 0=Normal Game
 //Safes the Position of the Intersection for Collsion
 uint16_t IntersectX;
 uint16_t IntersectY;
 
//Structure for Ball
typedef struct BallData{
    uint16_t PositionX;
    uint16_t PositionY;
    int16_t nextPositionX;
    int16_t nextPositionY;

}BallData_t;
//Prototyp Function Declarations
BallData_t SetPosition(uint16_t x, uint16_t y, BallData_t Ball);
BallData_t getNextPosition(BallData_t Ball);
BallData_t WriteNextPositon(BallData_t Ball);
BallData_t VictoryCheck(BallData_t Ball);
BallData_t WallCollision(BallData_t Ball, uint16_t Map[][4], uint8_t Mapsize);

void DrawVictoryScreen();
void printMap(uint16_t Map[][4], uint8_t Mapsize);
void drawVictorySquare();
void InitalizeGame();
bool get_line_intersection(uint16_t p0_x, uint16_t p0_y, uint16_t p1_x, uint16_t p1_y, uint16_t p2_x, uint16_t p2_y, uint16_t p3_x, uint16_t p3_y);
int16_t getDistance();
void TC3_Callback_InterruptHandler(TC_TIMER_STATUS status, uintptr_t context);
        
//Initalize Ball
BallData_t Ball={START_POSITONXY,START_POSITONXY,0,0};



// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    TC3_TimerCallbackRegister(TC3_Callback_InterruptHandler, (uintptr_t)NULL);
    InitalizeGame();

    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}
//Game Loop called every 25ms via interrupt of the timer
void TC3_Callback_InterruptHandler(TC_TIMER_STATUS status, uintptr_t context){
    //Normal Game Loop
    if(GameState==false){
        readAcceldataxyz(AccelData);
        Ball=getNextPosition(Ball);
        Ball=WallCollision(Ball,Testmap,(uint8_t)(sizeof(Testmap)/sizeof(Testmap[0])));
        Ball=WriteNextPositon(Ball);
        Ball=VictoryCheck(Ball);
        GameTimer++;
        
    }
    //Victory Screen Game Loop
    else{

        if(InterruptCounter%10==0){
            DrawVictoryScreen();
        }
        readAcceldataxyz(AccelData);
        Ball=getNextPosition(Ball);
        Ball=WriteNextPositon(Ball);
        //Restarts the normal Game 
        if(Ball.PositionX>=720 && Ball.PositionY>=400){
            GameTimer=0;
            Ball=SetPosition(START_POSITONXY,START_POSITONXY,Ball);
            fillScreen(RA8875_WHITE);
            printMap(Testmap,(uint8_t)(sizeof(Testmap)/sizeof(Testmap[0])));
            drawVictorySquare();
            GameState=false;
        }
    }
    // InterruptCounter to make fps adjustable     
    if(InterruptCounter==0){
        InterruptCounter=50; 
    }
    --InterruptCounter;
}


//Manuel Positon Setter
BallData_t SetPosition(uint16_t x,uint16_t y, BallData_t Ball){
    Ball.PositionX=x;
    Ball.PositionY=y;
    return Ball;
}
//Use LineIntersection for Collision detection
// Returns 1 if the lines intersect, otherwise 0.
// Here the bezier coordinates of the degree are used
bool get_line_intersection(uint16_t p0_x, uint16_t p0_y, uint16_t p1_x, uint16_t p1_y,
    uint16_t p2_x, uint16_t p2_y, uint16_t p3_x, uint16_t p3_y)
{
    int16_t s1_x, s1_y, s2_x, s2_y;
    s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
    s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

    double s, t;
    s = (double)(-s1_y * ((int16_t)p0_x - p2_x) + s1_x * ((int16_t)p0_y - p2_y)) / (double)(-s2_x * s1_y + s1_x * s2_y);
    t = (double)(s2_x * ((int16_t)p0_y - p2_y) - s2_y * ((int16_t)p0_x - p2_x)) / (double)(-s2_x * s1_y + s1_x * s2_y);

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    {
        // Collision detected

        IntersectX = (p0_x + (t * s1_x));
        IntersectY = (p0_y + (t * s1_y));
        return 1;
    }

    return 0; // No collision
}
//Draws checkered pattern for the Victory Square
void drawVictorySquare(){
    for(uint8_t i=0;i<8;i++){
       for(uint8_t j=0;j<8;j++){
           if((j+i)%2==0){
               fillRect(620+j*5,220+i*5,WALL_WIDTH,WALL_WIDTH,RA8875_BLACK);
           }
        }
    }
}

//Removes Circle and Writes next Position after Collsion detection
BallData_t WriteNextPositon(BallData_t Ball){
    fillCircle(Ball.PositionX,Ball.PositionY,10,RA8875_WHITE);
     Ball.PositionX=Ball.nextPositionX;
     Ball.PositionY=Ball.nextPositionY;
     fillCircle(Ball.PositionX,Ball.PositionY,10,RA8875_BLACK);
     return Ball;
}

//Prints Map 
void printMap(uint16_t Map[][4],uint8_t Mapsize){
    for(uint8_t i; i<Mapsize;i++){
        fillRect(Map[i][0],Map[i][1],Map[i][2],Map[i][3],RA8875_BLACK);
    }
}
//Controlles Speed and Sets the next Positon
BallData_t getNextPosition(BallData_t Ball){
    if(((AccelData[0]/SPEED_SCALER) > MOVEMENT_SENSTIVITY_LIMIT || (AccelData[0]/SPEED_SCALER) < -MOVEMENT_SENSTIVITY_LIMIT)){
        if((AccelData[0]/SPEED_SCALER) > MAX_SPEED_LIMIT){
            AccelData[0]=MAX_SPEED_LIMIT;
        }
        else if((AccelData[0]/SPEED_SCALER) < -MAX_SPEED_LIMIT){
          AccelData[0]=-MAX_SPEED_LIMIT;
        }
        Ball.nextPositionX=Ball.PositionX+(AccelData[0]/SPEED_SCALER);
    }
    if( ((AccelData[1]/SPEED_SCALER) >MOVEMENT_SENSTIVITY_LIMIT || (AccelData[1]/SPEED_SCALER)<-MOVEMENT_SENSTIVITY_LIMIT)){
        if((AccelData[1]/SPEED_SCALER) > MAX_SPEED_LIMIT){
            AccelData[1]=MAX_SPEED_LIMIT;
        }
         else if((AccelData[1]/SPEED_SCALER) < -MAX_SPEED_LIMIT){
          AccelData[1]=-MAX_SPEED_LIMIT;
        }
        Ball.nextPositionY=Ball.PositionY-(AccelData[1]/SPEED_SCALER);
    }
    //Limits the movement to Display and moves the nextPositon to the min or max if it is out of the Display
    if(Ball.nextPositionX<BALL_RADIUS){
        Ball.nextPositionX=0+BALL_RADIUS;
    }
    else if(Ball.nextPositionX>DISPLAY_WIDTH-BALL_RADIUS){
        Ball.nextPositionX=DISPLAY_WIDTH-BALL_RADIUS;
    }
    if(Ball.nextPositionY<BALL_RADIUS){
        Ball.nextPositionY=0+BALL_RADIUS;
    }
    else if(Ball.nextPositionY>DISPLAY_HEIGHT-BALL_RADIUS){
        Ball.nextPositionY=DISPLAY_HEIGHT-BALL_RADIUS;
    }
    
    return Ball;
}

/*
 * Line intersection is used to detect Collsions since classical Collision Algoriths assume that the Shapes intersect atleast in one Frame 
 * which isnt the case in this project. The movement is dynamic enough that the Ball can move past a wall without colliding in a single Frame.
 * Increasing the Framerate isnt an Option because the display refresh rate is limited by display specs.
 * Since diffrent Lines have to be checked depending on the movement direction a switch is necessary
 * Furthermore 4 cases for Straight movement have been implementet since there two points of bounding box of the ball have to be checked
 */
//Check Collsion for the Walls
BallData_t WallCollision(BallData_t Ball,uint16_t Map[][4],uint8_t Mapsize){
    uint16_t StartPointX=0;
    uint16_t EndPointX=0;
    uint16_t StartPointY=0;
    uint16_t EndPointY=0;
    //Gets the closest Collision since the line can intersect with multiple Walls
    uint16_t newCollisionPointDistance=0;
    uint16_t minCollisionPointDistance=2*(MAX_SPEED_LIMIT+BALL_RADIUS)*(MAX_SPEED_LIMIT+BALL_RADIUS)+1; //ensures that the first detected collsion has smaller distance than declaration
    bool isCollisionX=false;
    bool isCollisionY=false;
    uint8_t Quadrant;
    if(abs(AccelData[0])>(MOVEMENT_SENSTIVITY_LIMIT*SPEED_SCALER) && abs(AccelData[1])>(MOVEMENT_SENSTIVITY_LIMIT*SPEED_SCALER)){
        Quadrant = (AccelData[0]<0)+(AccelData[1]>0)*2;// X&Y Positiv=0 / X=negativ&Y=Positiv=1/ x Positiv and Y negativ =2/ X&Y Negativ =3  Y is inverse since the nextPosition is Postion -y/SPEED_SCALER
    }
    else {
        if(abs(AccelData[1])>(MOVEMENT_SENSTIVITY_LIMIT*SPEED_SCALER) && AccelData[0]>0){
            Quadrant=X_POSITIV;
        }
        else if(abs(AccelData[1])>(MOVEMENT_SENSTIVITY_LIMIT*SPEED_SCALER) && AccelData[0]<0){
            Quadrant=X_NEGATIV;
        }
        else if(abs(AccelData[0])>(MOVEMENT_SENSTIVITY_LIMIT*SPEED_SCALER) && AccelData[1]<0){
            Quadrant=Y_POSITIV;
        }
        else {
            Quadrant=Y_NEGATIV;
        }
    }
    for(uint8_t i=0;i<Mapsize;i++){
        StartPointX=Map[i][0];
        EndPointX=Map[i][0]+Map[i][2];
        StartPointY=Map[i][1];
        EndPointY=Map[i][1]+Map[i][3];
        
        // IntersectX and IntersectY is writen by get_line_intersection if a collision is detected
        switch(Quadrant){ 
            
            case QUADRANT1:  
                //For X-Line of the Wall
                isCollisionX=get_line_intersection(StartPointX,StartPointY,EndPointX,StartPointY, Ball.PositionX,Ball.PositionY,Ball.nextPositionX+BALL_RADIUS,Ball.nextPositionY+BALL_RADIUS);
                newCollisionPointDistance=getDistance();
                if(isCollisionX && (newCollisionPointDistance<minCollisionPointDistance)){
                    Ball.nextPositionX=IntersectX-BALL_RADIUS-OFFSET_PIXEL;
                    Ball.nextPositionY=IntersectY-BALL_RADIUS-OFFSET_PIXEL;
                    minCollisionPointDistance=newCollisionPointDistance;           
                }
                
                //For the Y-Line of the Wall
                isCollisionY=get_line_intersection(StartPointX,StartPointY,StartPointX,EndPointY,Ball.PositionX,Ball.PositionY,Ball.nextPositionX+BALL_RADIUS,Ball.nextPositionY+BALL_RADIUS);
                newCollisionPointDistance=getDistance();
                if(isCollisionY && newCollisionPointDistance<minCollisionPointDistance) {
                    Ball.nextPositionX=IntersectX-BALL_RADIUS-OFFSET_PIXEL;
                    Ball.nextPositionY=IntersectY-BALL_RADIUS-OFFSET_PIXEL;
                    minCollisionPointDistance=newCollisionPointDistance;
                        
                } 
                break;
            case QUADRANT2:
                  //For X-Line of the Wall
                isCollisionX=get_line_intersection(StartPointX,EndPointY,EndPointX,EndPointY, Ball.PositionX,Ball.PositionY,Ball.nextPositionX-BALL_RADIUS,Ball.nextPositionY+BALL_RADIUS);
                newCollisionPointDistance=getDistance();
                if(isCollisionX && (newCollisionPointDistance<minCollisionPointDistance)){
                        Ball.nextPositionX=IntersectX+BALL_RADIUS+OFFSET_PIXEL;
                        Ball.nextPositionY=IntersectY-BALL_RADIUS-OFFSET_PIXEL;
                        minCollisionPointDistance=newCollisionPointDistance;
                       
                        
                }
                      
                //For the Y-Line of the Wall
                isCollisionY=get_line_intersection(StartPointX,StartPointY,StartPointX,EndPointY,Ball.PositionX,Ball.PositionY,Ball.nextPositionX-BALL_RADIUS,Ball.nextPositionY+BALL_RADIUS);
                newCollisionPointDistance=getDistance();
                if(isCollisionY && newCollisionPointDistance<minCollisionPointDistance) {
                        Ball.nextPositionX=IntersectX+BALL_RADIUS+OFFSET_PIXEL;
                        Ball.nextPositionY=IntersectY-BALL_RADIUS-OFFSET_PIXEL;
                        minCollisionPointDistance=newCollisionPointDistance;
                } 
                
                break;
            case QUADRANT3:
                 //For X-Line of the Wall
                isCollisionX=get_line_intersection(StartPointX,StartPointY,EndPointX,StartPointY, Ball.PositionX,Ball.PositionY,Ball.nextPositionX+BALL_RADIUS,Ball.nextPositionY-BALL_RADIUS);
                newCollisionPointDistance=getDistance();
                if(isCollisionX && (newCollisionPointDistance<minCollisionPointDistance)){
                        Ball.nextPositionX=IntersectX-BALL_RADIUS-OFFSET_PIXEL;
                        Ball.nextPositionY=IntersectY+BALL_RADIUS+OFFSET_PIXEL;
                        minCollisionPointDistance=newCollisionPointDistance;       
                        
                }
                     
                //For the Y-Line of the Wall
                isCollisionY=get_line_intersection(EndPointX,StartPointY,EndPointX,EndPointY,Ball.PositionX,Ball.PositionY,Ball.nextPositionX+BALL_RADIUS,Ball.nextPositionY-BALL_RADIUS);
                newCollisionPointDistance=getDistance();
                if(isCollisionY && newCollisionPointDistance<minCollisionPointDistance) {
                        Ball.nextPositionX=IntersectX-BALL_RADIUS-OFFSET_PIXEL;
                        Ball.nextPositionY=IntersectY+BALL_RADIUS+OFFSET_PIXEL;
                        minCollisionPointDistance=newCollisionPointDistance;     
                } 
                
            break;
             
            case QUADRANT4:
                 //For X-Line of the Wall
                isCollisionX=get_line_intersection(StartPointX,EndPointY,EndPointX,EndPointY, Ball.PositionX,Ball.PositionY,Ball.nextPositionX-BALL_RADIUS,Ball.nextPositionY-BALL_RADIUS);
                newCollisionPointDistance=getDistance();
                if(isCollisionX && (newCollisionPointDistance<minCollisionPointDistance)){
                        Ball.nextPositionX=IntersectX+BALL_RADIUS+OFFSET_PIXEL;
                        Ball.nextPositionY=IntersectY+BALL_RADIUS+OFFSET_PIXEL;
                        minCollisionPointDistance=newCollisionPointDistance;
                        
                }
                       
                //For the Y-Line of the Wall
                isCollisionY=get_line_intersection(EndPointX,StartPointY,EndPointX,EndPointY,Ball.PositionX,Ball.PositionY,Ball.nextPositionX-BALL_RADIUS,Ball.nextPositionY-BALL_RADIUS);
               newCollisionPointDistance=getDistance();
                if(isCollisionY && newCollisionPointDistance<minCollisionPointDistance) {
                        Ball.nextPositionX=IntersectX+BALL_RADIUS+OFFSET_PIXEL;
                        Ball.nextPositionY=IntersectY+BALL_RADIUS+OFFSET_PIXEL;
                        minCollisionPointDistance=newCollisionPointDistance;
                } 
                
                
            break;
            //Collison for Upper-Right and Lower-Right Point of Ball Bounding Box 
            case X_POSITIV:
                //For the Lower-Right Point
                isCollisionX=get_line_intersection(StartPointX,StartPointY,EndPointX,StartPointY, Ball.PositionX,Ball.PositionY,Ball.nextPositionX+BALL_RADIUS,Ball.nextPositionY+BALL_RADIUS);
                newCollisionPointDistance=getDistance();
                if(isCollisionX && (newCollisionPointDistance<minCollisionPointDistance)){
                    Ball.nextPositionX=IntersectX-BALL_RADIUS-OFFSET_PIXEL;
                    Ball.nextPositionY=IntersectY-BALL_RADIUS-OFFSET_PIXEL;
                    minCollisionPointDistance=newCollisionPointDistance;           
                }
                //For the Upper-Right Point
                isCollisionX=get_line_intersection(StartPointX,StartPointY,EndPointX,StartPointY, Ball.PositionX,Ball.PositionY,Ball.nextPositionX+BALL_RADIUS,Ball.nextPositionY-BALL_RADIUS);
               newCollisionPointDistance=getDistance();
                if(isCollisionX && (newCollisionPointDistance<minCollisionPointDistance)){
                    Ball.nextPositionX=IntersectX-BALL_RADIUS-OFFSET_PIXEL;
                    Ball.nextPositionY=IntersectY+BALL_RADIUS+OFFSET_PIXEL;
                    minCollisionPointDistance=newCollisionPointDistance;           
                }
            break;
            //Collision for Upper-Left and Lower-Left Point of Ball Bounding Box
            case X_NEGATIV:
                //For the Upper-Left Point
                 isCollisionX=get_line_intersection(StartPointX,EndPointY,EndPointX,EndPointY, Ball.PositionX,Ball.PositionY,Ball.nextPositionX-BALL_RADIUS,Ball.nextPositionY-BALL_RADIUS);
                newCollisionPointDistance=getDistance();
                if(isCollisionX && (newCollisionPointDistance<minCollisionPointDistance)){
                        Ball.nextPositionX=IntersectX+BALL_RADIUS+OFFSET_PIXEL;
                        Ball.nextPositionY=IntersectY+BALL_RADIUS+OFFSET_PIXEL;
                        minCollisionPointDistance=newCollisionPointDistance;
                        
                }
                //For the Lower-Left Point
                 isCollisionX=get_line_intersection(StartPointX,EndPointY,EndPointX,EndPointY, Ball.PositionX,Ball.PositionY,Ball.nextPositionX-BALL_RADIUS,Ball.nextPositionY+BALL_RADIUS);
                newCollisionPointDistance=getDistance();
                if(isCollisionX && (newCollisionPointDistance<minCollisionPointDistance)){
                        Ball.nextPositionX=IntersectX+BALL_RADIUS+OFFSET_PIXEL;
                        Ball.nextPositionY=IntersectY-BALL_RADIUS-OFFSET_PIXEL;
                        minCollisionPointDistance=newCollisionPointDistance;
                        
                }
            break;
            //Collision for Lower-Left and Lower-Right Point of Ball Bounding Box    
            case Y_POSITIV:
                //For the Lower-Left Point
                isCollisionY=get_line_intersection(StartPointX,StartPointY,StartPointX,EndPointY,Ball.PositionX,Ball.PositionY,Ball.nextPositionX-BALL_RADIUS,Ball.nextPositionY+BALL_RADIUS);
                newCollisionPointDistance=getDistance();
                if(isCollisionY && newCollisionPointDistance<minCollisionPointDistance) {
                        Ball.nextPositionX=IntersectX+BALL_RADIUS+OFFSET_PIXEL;
                        Ball.nextPositionY=IntersectY-BALL_RADIUS-OFFSET_PIXEL;
                        minCollisionPointDistance=newCollisionPointDistance;
                } 
                //For the Lower-Right Point
                isCollisionY=get_line_intersection(StartPointX,StartPointY,StartPointX,EndPointY,Ball.PositionX,Ball.PositionY,Ball.nextPositionX+BALL_RADIUS,Ball.nextPositionY+BALL_RADIUS);
                newCollisionPointDistance=getDistance();
                if(isCollisionY && newCollisionPointDistance<minCollisionPointDistance) {
                        Ball.nextPositionX=IntersectX-BALL_RADIUS-OFFSET_PIXEL;
                        Ball.nextPositionY=IntersectY-BALL_RADIUS-OFFSET_PIXEL;
                        minCollisionPointDistance=newCollisionPointDistance;
                } 
                
            break;  
            //Collision for Upper-Left and Upper-Right Point of Ball Bounding Box       
            case Y_NEGATIV:
                //For Upper-Left Point
               isCollisionY=get_line_intersection(EndPointX,StartPointY,EndPointX,EndPointY,Ball.PositionX,Ball.PositionY,Ball.nextPositionX-BALL_RADIUS,Ball.nextPositionY-BALL_RADIUS);
               newCollisionPointDistance=getDistance();
                if(isCollisionY && newCollisionPointDistance<minCollisionPointDistance) {
                        Ball.nextPositionX=IntersectX+BALL_RADIUS-OFFSET_PIXEL;
                        Ball.nextPositionY=IntersectY+BALL_RADIUS+OFFSET_PIXEL;
                        minCollisionPointDistance=newCollisionPointDistance;
                }
               //For Upper-Right Point
               isCollisionY=get_line_intersection(EndPointX,StartPointY,EndPointX,EndPointY,Ball.PositionX,Ball.PositionY,Ball.nextPositionX+BALL_RADIUS,Ball.nextPositionY-BALL_RADIUS);
               newCollisionPointDistance=getDistance();
                if(isCollisionY && newCollisionPointDistance<minCollisionPointDistance) {
                        Ball.nextPositionX=IntersectX-BALL_RADIUS-OFFSET_PIXEL;
                        Ball.nextPositionY=IntersectY+BALL_RADIUS+OFFSET_PIXEL;
                        minCollisionPointDistance=newCollisionPointDistance;
                } 
            break;
        } 
            
        
    }
    return Ball;
}
//Calculates Distance without Square-root of Pythargos
int16_t getDistance(){
    int16_t deltaX=0;
    int16_t deltaY=0;
    deltaX=((int16_t)Ball.PositionX-IntersectX);
    deltaY=((int16_t)Ball.PositionY-IntersectY);
    return (deltaX*deltaX+deltaY*deltaY);
}

//Checks for Victory and prints Endscreen
BallData_t VictoryCheck(BallData_t Ball){
    if(Ball.PositionX>=620 && Ball.PositionX<=660 && Ball.PositionY>=220 && Ball.PositionY<=260){
        GameState=true;
        fillScreen(RA8875_WHITE);
        Ball=SetPosition(10,10,Ball);
        DrawVictoryScreen();
    }
    return Ball;
}
//Draws Endscreen and calculates time of the game in 25ms resolution
void DrawVictoryScreen(){
        textMode();
        textSetCursor(270,150);
        textColor(RA8875_BLACK,RA8875_WHITE);
        textEnlarge(10);
        textWrite("Victory",7);
        textEnlarge(6);
        textSetCursor(170,220);
        textWrite("Your Time was ",14);
        uint8_t mins=GameTimer*25/60000;
        uint8_t secs=(GameTimer*25-mins*60000)/1000;
        uint16_t milsecs=(GameTimer*25-mins*60000-secs*1000);
        char buffer[10]={'/0'};
        textSetCursor(240,290);
        snprintf(buffer,10,"%02d:%02d,%03d",mins,secs,milsecs);
        textWrite(buffer,10);
        textSetCursor(530,360);
        textEnlarge(1);
        textWrite("Go in to restart",16);
        fillRect(720,400,80,80,RA8875_BLACK);

}
//Initalizes Sensor as well as Display and then starts the gameplay loop (TC3-Start to get Interrupts every 25ms)
void InitalizeGame(){
     SYSTICK_TimerStart();
    //Config via SPI for Sensor 
    SensorSPIInit();
    SensorConfig();
    //Initiales Display on RA8875 for 800x480
    fullInitLCD();
    displayOn(true);
    GPIOX(true);      // Enable TFT - display enable tied to GPIOX
    PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
    //Brightness Controll
    PWM1out(SCREEN_BRIGHTNESS);
    //Draws Map 
    fillScreen(RA8875_WHITE);
    printMap(Testmap,(uint8_t)(sizeof(Testmap)/sizeof(Testmap[0])));
    drawVictorySquare();
    //Start Game
    TC3_TimerStart();
}
/*******************************************************************************
 End of File
*/

