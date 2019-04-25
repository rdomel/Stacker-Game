/* 
 *  Stackers 
 *  Classic arcade game remade by Jason Joehlin, Reno Domel, and Ken Pendergest
 *  Embedded Systems Final Project
 */

/* 
 *  Initializing variables 
 * These are all initialized at the top so that they can be referenced throughout multiple loops
 * The use for each one can be seen below as well as in the project report
 */ 
int col = 7;                    // Column index, controls which column is being called
int stepsize = 1;               // Indicates which direction the commands are looping through 
volatile int pressed = 1;       // Indicates when the button is pushed, equal to 0 when button is pressed
volatile int potReading;        // Value recieved from analog input of potentiometer
volatile float difficulty;      // mapped from potReading, sets levels for gameplay
volatile int timestart = 0;     // timing variable used to make sure button is not pushes infinitely fast
volatile int timepassed;        // variable used with timestart to ensure the button is pressed at resonable times
int i = 5;                      // sets initial location of leftmost dot, used to loop through code
int laststop = -1;              // set to an unrealistic number, tracks where the user stops the blocks
boolean startgameB = true;      // boolean value used when the start game and set to the first function 
boolean gameover = false;       // defines when the user losses and sends to the ending function when true
boolean runthreebig = true;     // defines when the user is playing with three blocks
boolean runtwobig = false;      // defines when the user is playing with two blocks
boolean runonebig = false;      // defines when the user is playing with two blocks
boolean firstcol = true;        // defines when the user is starting the game, calls a function for the first play of the game
int difference;                 // Indicates how far off the user is from the blocks below, uses laststop to be determined
volatile int pausetime;         // Used to control how fast the blocks move 

/* Several extra lines are needed to set up the dot matrix
 * The following shows where the pins from the matrix are connected to the arduino
 DIN connects to pin 12
 CLK connects to pin 11
 CS connects to pin 10 
 There is also two libraries needed to be downladed to run the dot matrix
 */
#include "LedControl.h"
#include "binary.h"
LedControl lc=LedControl(12,11,10,1);  

/* The following lines are binary representations of which lights could be lit up at the same time
 *  A 1 represents a light in the column is on and a 0 represents off
 *  These arrays include all possible combinations for the blocks to move across the screen 
 *  There is one for each level (i.e. 3 blocks moving, 2 blocks moving, 1 block moving)
 *  The last two arrays are the combination sneeded to make faces on the matrix according to the outcome of the game
 */
byte threebig[6]= {B11100000, B01110000, B00111000, B00011100, B00001110, B00000111};                       //  three blocks left
byte twobig[7]= {B11000000, B01100000, B00110000,B00011000, B00001100, B00000110, B00000011};               //  two blocks left
byte onebig[8]= {B10000000, B01000000, B00100000, B00010000, B00001000, B00000100, B00000010, B00000001};   //  one block left
byte happy[8]= {B00111100, B01000010, B10100101, B10000001, B10100101, B10011001, B01000010, B00111100};    //  Happy face
byte sad[8]= {B00111100,B01000010,B10100101,B10000001,B10011001,B10100101,B01000010,B00111100};             //  sad face

/* Setup used just at the beginning to get everything setup: 
 *  pinModes
 *  Initializing Matrix
 *  Attaching Interrupt  
 */
void setup() {
  pinMode(5,OUTPUT);// green
  pinMode(6,OUTPUT); // yellow
  pinMode(7,OUTPUT);// red
  pinMode(9,INPUT); // piezo
  
  lc.shutdown(0,false);
  lc.setIntensity(0,7);  // Set brightness to a medium value
  lc.clearDisplay(0);    // Clear the display
  attachInterrupt(0,stop,RISING);
  Serial.begin(9600);
}

/* 
 * The majority of our coding is written in the functions at the bottom
 * However, the loop is full of conditionals that call those functions
 * The loop also contains the code for the blocks moving back and forth across the board
 * when the button is pushed, depending on the position a function is called from this loop
 */
void loop(){
 
/* 
 * Sets difficulty 
 * read in the vlaue of the potentiometer, maps it to set difficulty
 * sets a timing based on difficulty for how fast blocks move 
 * turns on the LED that represents the difficulty 
 */
while(pressed==1 && startgameB == true){
    potReading = analogRead(A0);
    difficulty = map(potReading, 0, 1023, 0, 90);
  if (difficulty < 31){
    pausetime = 80;
    digitalWrite(7,HIGH); //green
    digitalWrite(6,LOW);  //yellow
    digitalWrite(5,LOW);  //red
    }
  if (difficulty >= 31 && difficulty <=60){
    pausetime = 100;
    digitalWrite(7,LOW);
    digitalWrite(6,HIGH);
    digitalWrite(5,LOW);
    }
  if (difficulty > 60){
    pausetime = 150;
    digitalWrite(7,LOW);
    digitalWrite(6,LOW);
    digitalWrite(5,HIGH);
    }
}

/*
 * Run at the begining of the program
 * sets variable to be able to be used in future conditionals, and avoid certain loops 
 */
if (startgameB == true)
  {
  startgameB = false;
  }

// Each time the loop runs, pressed is reset to 1, allowing the following conditionals to be true
pressed = 1;
 
/*
 * Three blocks moving across screen
 * Using the three big array above and the i variable (location of left block)
 * Shows the blocks, clears the blocks and shows them in the next position
 * when it reaches one end of the matrix, the i counter moves in the opposite direction
 */
while (i>=0 && i<=5 && pressed == 1 && runthreebig == true && startgameB == false)
{
  i=i-stepsize;
  lc.setColumn(0,col,threebig[i]);
  delay(pausetime);
  lc.setColumn(0,col, B00000000);
  if (i == 5 || i == 0)
  {
    stepsize=stepsize*-1;
  }
}
pressed=1;

  /*
   * Allows user to set the blocks in the first column
   * sets the first location variable so that the next row can be compared
   * moves the column value so the the user advances up the board
   */
if (firstcol == true)
{
  laststop = i;
  lc.setColumn(0,col,threebig[i]);
  col = col - 1;
}

/* 
 *  runs if user has three blocks and lines them up perfectly with column below
 */
if (laststop== i && runthreebig == true && firstcol == false)
{
  threeperfect();
}

/*
 * Runs if user has two blocks and lines them up perfectly with two blocks in the column below
 */
if (laststop== i && runtwobig == true)
{
  twoperfect();
}

/*
 * Runs if user has one block and lines it up perfectly with block below
 */
if (laststop== i && runonebig == true)
{
  oneperfect();
}

/* 
 * finds the difference between the blocks below and where user places the current row 
 * based on left most block 
 */
difference=laststop-i;

/* 
 * runs if the user has three blocks
 * if the current blocks are placed two to the right of the row below
 */
if(difference == 2 & runthreebig == true){
  threetooneRIGHT();
  }

/* 
 * runs if the user has three blocks
 * if the current blocks are placed two to the left of the row below
 */
if(difference == -2 & runthreebig == true){
  threetooneLEFT();
  }

/* 
 * runs if the user has three blocks
 * if the current blocks are placed one to the right of the row below
 */
if (difference ==1 && runtwobig == false && runthreebig==true)
{
threetotwoRIGHT();
}

/* 
 * runs if the user has three blocks
 * if the current blocks are placed one to the left of the row below
 */
if (difference ==-1 && runtwobig==false && runthreebig==true){
  threetotwoLEFT();
 }

/* 
 * runs if the user has two blocks
 * if the current blocks are placed one to the left of the row below
 */
if (difference ==-1 && runtwobig==true){
  twotooneLEFT();
 }

/* 
 * runs if the user has two blocks
 * if the current blocks are placed one to the right of the row below
 */
if (difference ==1 && runtwobig==true){
  twotooneRIGHT();
 }

/* 
 * runs if the user has three blocks
 * if the current blocks are not lined up at all with row below
 * calls an enfing function
 */
if (abs(difference)>=3)
{
  gamelost();
}

/* 
 * runs if the user has two blocks
 * if the current blocks are not lined up at all with the row below
 * calls an ending function 
 */
if (abs(difference) >= 2 && runtwobig==true){
  gamelost();
  }

/* 
 * runs if the user has one block
 * if the current blocks is not lined up at all with the row below
 * calls an ending function 
 */
if(abs(difference) >= 1 && runonebig == true){
  gamelost();
}

/*
 * If the user makes it through the final column successfully
 * calls the winning function
 */
if (col == -1){
  gamewon();
  }

/*
 * two blocks moving across screen
 * Using the two big array above and the i variable (location of left block)
 * Shows the blocks, clears the blocks and shows them in the next position
 * when it reaches one end of the matrix, the i counter moves in the opposite direction
 */
while (i>=0 && i<=6 && pressed == 1 && runtwobig == true)
{
  i=i-stepsize;
  lc.setColumn(0,col,twobig[i]);
  delay(pausetime);
  lc.setColumn(0,col, B00000000);
  if (i == 6 || i == 0)
  {
    stepsize=stepsize*-1;
  }
}

/*
 * one blocks moving across screen
 * Using the one big array above and the i variable (location of left block)
 * Shows the blocks, clears the block and shows it in the next position
 * when it reaches one end of the matrix, the i counter moves in the opposite direction
 */
while (i>=0 && i<=7 && pressed == 1 && runonebig == true)
{
  i=i-stepsize;
  lc.setColumn(0,col,onebig[i]);
  delay(pausetime);
  lc.setColumn(0,col, B00000000);
  if (i == 7 || i == 0)
  {
    stepsize=stepsize*-1;
  }
}

// sets value to  false to keep from entering above loop again
firstcol = false; 
}

/**********************/

/*
 * Interrupt function
 * sets value of pressed to 0 so other functions can be called
 * initiates a timing system that keeps the user from pressing the button infinitely fast
 */
void stop(){
    if(millis() - timestart > 250){
    pressed = 0;
    pausetime = pausetime - 8;
    timestart = millis();
     }
  }

  /*
   * Runs when user has three blocks and lines them up perfectly
   * sets the blocks to the next line and allows user to keep playing
   */
void threeperfect(){
  laststop = i;
  lc.setColumn(0,col,threebig[i]);
  col = col - 1;
  }

  /* 
   *  Runs when user has three blocks and misses by one to the right
   *  sets the correct blocks to stay in their position
   *  moves to the next line and runs with two blocks moving across
   */
void threetooneRIGHT(){
  laststop = i+2;
  runthreebig = false;
  runonebig=true;
  lc.setColumn(0,col,onebig[i+2]);
  col = col - 1;
  difference = 0;
  }

  /* 
   *  Runs when user has three blocks and misses by one to the left
   *  sets the correct blocks to stay in their position
   *  moves to the next line and runs with two blocks moving across
   */
void threetooneLEFT(){
  laststop = i;
  runthreebig = false;
  runonebig=true;
  lc.setColumn(0,col,onebig[i]);
  col = col - 1;
  difference = 0;
  }

  /* 
   *  Runs when user has three blocks and misses by owo to the right
   *  sets the correct block to stay in its position
   *  moves to the next line and runs with one block moving across
   */
void threetotwoRIGHT(){
  laststop = i+1;
  runthreebig = false;
  runtwobig=true;
  lc.setColumn(0,col,twobig[i+1]);
  col = col - 1;
  difference = 0;
  }

  /* 
   *  Runs when user has three blocks and misses by owo to the left
   *  sets the correct block to stay in its position
   *  moves to the next line and runs with one block moving across
   */
void threetotwoLEFT(){
  laststop = i;
  runthreebig = false;
  runtwobig=true;
  lc.setColumn(0,col,twobig[i]);
  col = col - 1;
  difference = 0;
  }

  /*
   * Runs when user has two blocks and lines them up perfectly
   * sets the blocks to the next line and allows user to keep playing
   */
void twoperfect(){
  laststop = i;
  lc.setColumn(0,col,twobig[i]);
  col = col - 1;
  }

   /*
   * Runs when user has one block and lines them up perfectly
   * sets the blocks to the next line and allows user to keep playing
   */
void oneperfect(){
  laststop = i;
  lc.setColumn(0,col,onebig[i]);
  col = col - 1;
  }
  
  /* 
   *  Runs when user has two blocks and misses by one to the left
   *  sets the correct block to stay in its position
   *  moves to the next line and runs with one block moving across
   */
void twotooneLEFT(){
  laststop = i;
  runtwobig = false;
  runonebig = true;
  lc.setColumn(0,col,onebig[i]);
  col = col - 1;
  difference = 0;
  Serial.println(runonebig);
  Serial.println(col);
  Serial.println(pressed);
 }

  /* 
   *  Runs when user has two blocks and misses by one to the right
   *  sets the correct block to stay in its position
   *  moves to the next line and runs with one block moving across
   */
void twotooneRIGHT(){
  laststop = i+1;
  runtwobig = false;
  runonebig = true;
  lc.setColumn(0,col,onebig[i+1]);
  col = col - 1;
  difference = 0;
  }

  /* 
   *  Runs when user makes it through the last row successfully 
   *  plays a happy tune and shows a happy face
   */
void gamewon(){
  // Set the happy Face
  delay(1000);
  lc.setColumn(0,0,happy[0]);
  lc.setColumn(0,1,happy[1]);
  lc.setColumn(0,2,happy[2]);
  lc.setColumn(0,3,happy[3]);
  lc.setColumn(0,4,happy[4]);
  lc.setColumn(0,5,happy[5]);
  lc.setColumn(0,6,happy[6]);
  lc.setColumn(0,7,happy[7]);
  // Set the music in the background
  pinMode(9,OUTPUT);
  tone(9,196);
  delay(100);
      pinMode(9,INPUT);
      delay(100);
    pinMode(9,OUTPUT);
  tone(9,196);
  delay(100);
      pinMode(9,INPUT);
      delay(300);
    pinMode(9,OUTPUT);
  tone(9,196);
  delay(100);
      pinMode(9,INPUT);
      delay(300);
    pinMode(9,OUTPUT);
  tone(9,146.83);
  delay(100);
      pinMode(9,INPUT);
      delay(100);
    pinMode(9,OUTPUT);
  tone(9,196);
    delay(100);
      pinMode(9,INPUT);
      delay(300);
    pinMode(9,OUTPUT);
  tone(9,220);
  delay(100);
      pinMode(9,INPUT);
  delay(600);
pinMode(9,OUTPUT);
 tone(9,130);
  delay(150);
      pinMode(9,INPUT);
  lc.clearDisplay(0);
  while(1==1);
  }

/*
 * runs when the user suffers a loss
 * plays a sad tune and shows a sad face
 */
void gamelost(){
    gameover=true; 
    delay(1000);      
       // Set the sad face
    lc.setColumn(0,0,sad[0]);
    lc.setColumn(0,1,sad[1]);
    lc.setColumn(0,2,sad[2]);
    lc.setColumn(0,3,sad[3]);
    lc.setColumn(0,4,sad[4]);
    lc.setColumn(0,5,sad[5]);
    lc.setColumn(0,6,sad[6]);
    lc.setColumn(0,7,sad[7]);
    // Sound 
    pinMode(9,OUTPUT);
    tone(9,200);
    delay(1000);
    tone(9,100);
    delay(1000);
    pinMode(9,INPUT);
    while(1==1);
  }
