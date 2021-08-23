/*************************************************** 
  This is a Pong port for the Arduino. The application 
  uses an Arduino Uno, Adafruit’s 128x64 OLED display, 
  2 potentiometers and an piezo buzzer.

  More info about this project can be found on my blog:
  http://michaelteeuw.nl

  Written by Michael Teeuw | Xonay Labs.  
  Apache 2 license, all text above must be included 
  in any redistribution.
 ****************************************************/
  
//Define Pins
#define BEEPER 3
#define CONTROL_A 31 //A0
#define CONTROL_B 32

//Define Visuals
#define FONT_SIZE 2
#define SCREEN_WIDTH 127  //real size minus 1, because coordinate system starts with 0
#define SCREEN_HEIGHT 63  //real size minus 1, because coordinate system starts with 0
#define PADDLE_WIDTH 4
#define PADDLE_HEIGHT 10
#define PADDLE_PADDING 10
#define BALL_SIZE 3
#define SCORE_PADDING 10

#define EFFECT_SPEED 0.5
#define MIN_Y_SPEED 0.5
#define MAX_Y_SPEED 2

#define WINNING_SCORE 11

//Prototypes

void setControlA(int value);
void setControlB(int value);
void startPong();
void resetPong();
void setupPong(Adafruit_SSD1306* mainDisplay);
void splash();
void winGame();
void loopPong();
void calculateMovement();
void draw();
void addEffect(int paddleSpeed);
void centerPrint(char *text, int y, int size);

//Define Variables

Adafruit_SSD1306 *pongDisplay;

bool showSplash = true;

int paddleLocationA = 0;
int paddleLocationB = 0;

float ballX = SCREEN_WIDTH/2;
float ballY = SCREEN_HEIGHT/2;
float ballSpeedX = 2;
float ballSpeedY = 1;

int lastPaddleLocationA = 0;
int lastPaddleLocationB = 0;

int scoreA = 0;
int scoreB = 0;

int controlA = 0;
int controlB = 0;

int lastControlA = 0;
int lastControlB = 0;

int winner = -1;
bool winnerBlink = true;

void setControlA(int value) {
  controlA = value;
}

void setControlB(int value) {
  controlB = value;
}

//Setup 
void setupPong(Adafruit_SSD1306* mainDisplay) 
{
  pongDisplay = mainDisplay;
}

void startPong() {
  showSplash = false;
  //set text correctly for score
  pongDisplay->setTextWrap(false);  
  pongDisplay->setTextColor(WHITE);
  pongDisplay->setTextSize(FONT_SIZE);  
}

//Splash Screen
void splash()
{
  pongDisplay->clearDisplay(); 
  pongDisplay->setTextColor(WHITE);
  centerPrint("PONG!",0,2);
  centerPrint("Play to 11",28,2);    
  pongDisplay->fillRect(0,SCREEN_HEIGHT-10,SCREEN_WIDTH,10,WHITE);
  pongDisplay->setTextColor(BLACK);
  centerPrint("Press btn to start",SCREEN_HEIGHT-9,1);
  pongDisplay->display();  
}

void winGame()
{
  pongDisplay->clearDisplay(); 
  pongDisplay->setTextColor(WHITE);
  centerPrint("Winner Winner",3,1);
  centerPrint("Chicken Dinner",17,1);   

  if (winnerBlink) {
    if (winner == 0) {
      centerPrint("<<<<<",30,2);
    } else {
      centerPrint(">>>>>",30,2);
    }
  } else {
    centerPrint("      ",30,2);
  }
  
  winnerBlink = !winnerBlink;
    
  pongDisplay->fillRect(0,SCREEN_HEIGHT-10,SCREEN_WIDTH,10,WHITE);
  pongDisplay->setTextColor(BLACK);
  centerPrint("Press btn to continue",SCREEN_HEIGHT-9,1);
  pongDisplay->display();  
}

void resetPong() {

  showSplash = true;

  paddleLocationA = 0;
  paddleLocationB = 0;
  
  ballX = SCREEN_WIDTH/2;
  ballY = SCREEN_HEIGHT/2;
  ballSpeedX = 2;
  ballSpeedY = 1;
  
  lastPaddleLocationA = 0;
  lastPaddleLocationB = 0;
  
  scoreA = 0;
  scoreB = 0;

  winner = -1;
  
}

//Loop
void loopPong()
{
  if (showSplash) {
    splash();
    return;
  }
  if (winner >= 0) {
    winGame();
    return;
  }
  calculateMovement();
  draw();
}


void calculateMovement() 
{

  paddleLocationA = map(controlA, 0, 1023, 0, SCREEN_HEIGHT - PADDLE_HEIGHT);
  paddleLocationB = map(controlB, 0, 15, 0, SCREEN_HEIGHT - PADDLE_HEIGHT);

  int paddleSpeedA = paddleLocationA - lastPaddleLocationA;
  int paddleSpeedB = paddleLocationB - lastPaddleLocationB;

  ballX += ballSpeedX;
  ballY += ballSpeedY;

  //bounce from top and bottom
  if (ballY >= SCREEN_HEIGHT - BALL_SIZE || ballY <= 0) {
    ballSpeedY *= -1;
  }

  //bounce from paddle A
  if (ballX >= PADDLE_PADDING && ballX <= PADDLE_PADDING+BALL_SIZE && ballSpeedX < 0) {
    if (ballY > paddleLocationA - BALL_SIZE && ballY < paddleLocationA + PADDLE_HEIGHT) {      
      ballSpeedX *= -1;
    
      addEffect(paddleSpeedA);
    }

  }

  //bounce from paddle B
  if (ballX >= SCREEN_WIDTH-PADDLE_WIDTH-PADDLE_PADDING-BALL_SIZE && ballX <= SCREEN_WIDTH-PADDLE_PADDING-BALL_SIZE && ballSpeedX > 0) {
    if (ballY > paddleLocationB - BALL_SIZE && ballY < paddleLocationB + PADDLE_HEIGHT) {      
      ballSpeedX *= -1;
    
      addEffect(paddleSpeedB);
    }

  }

  //score points if ball hits wall behind paddle
  if (ballX >= SCREEN_WIDTH - BALL_SIZE || ballX <= 0) {
    if (ballSpeedX > 0) {
      scoreA++;
      ballX = SCREEN_WIDTH / 4;
    }
    if (ballSpeedX < 0) {
      scoreB++;
      ballX = SCREEN_WIDTH / 4 * 3;
    }

  }

  //set last paddle locations
  lastPaddleLocationA = paddleLocationA;
  lastPaddleLocationB = paddleLocationB;  
}

void draw() 
{
  pongDisplay->clearDisplay(); 

  //draw paddle A
  pongDisplay->fillRect(PADDLE_PADDING,paddleLocationA,PADDLE_WIDTH,PADDLE_HEIGHT,WHITE);

  //draw paddle B
  pongDisplay->fillRect(SCREEN_WIDTH-PADDLE_WIDTH-PADDLE_PADDING,paddleLocationB,PADDLE_WIDTH,PADDLE_HEIGHT,WHITE);

  //draw center line
  for (int i=0; i<SCREEN_HEIGHT; i+=4) {
    pongDisplay->drawFastVLine(SCREEN_WIDTH/2, i, 2, WHITE);
  }

  //draw ball
  pongDisplay->fillRect(ballX,ballY,BALL_SIZE,BALL_SIZE,WHITE);

  //print scores

  //backwards indent score A. This is dirty, but it works ... ;)
  int scoreAWidth = 5 * FONT_SIZE;
  if (scoreA > 9) scoreAWidth += 6 * FONT_SIZE;
  if (scoreA > 99) scoreAWidth += 6 * FONT_SIZE;
  if (scoreA > 999) scoreAWidth += 6 * FONT_SIZE;
  if (scoreA > 9999) scoreAWidth += 6 * FONT_SIZE;

  pongDisplay->setCursor(SCREEN_WIDTH/2 - SCORE_PADDING - scoreAWidth,0);
  pongDisplay->print(scoreA);

  pongDisplay->setCursor(SCREEN_WIDTH/2 + SCORE_PADDING+1,0); //+1 because of dotted line.
  pongDisplay->print(scoreB);

  pongDisplay->display();

  if (scoreA >= WINNING_SCORE) {
    winner = 0;
  } else if (scoreB >= WINNING_SCORE) {
    winner = 1;
  }  
  
} 

void addEffect(int paddleSpeed)
{
  float oldBallSpeedY = ballSpeedY;

  //add effect to ball when paddle is moving while bouncing.
  //for every pixel of paddle movement, add or substact EFFECT_SPEED to ballspeed.
  for (int effect = 0; effect < abs(paddleSpeed); effect++) {
    if (paddleSpeed > 0) {
      ballSpeedY += EFFECT_SPEED;
    } else {
      ballSpeedY -= EFFECT_SPEED;
    }
  }

  //limit to minimum speed
  if (ballSpeedY < MIN_Y_SPEED && ballSpeedY > -MIN_Y_SPEED) {
    if (ballSpeedY > 0) ballSpeedY = MIN_Y_SPEED;
    if (ballSpeedY < 0) ballSpeedY = -MIN_Y_SPEED;
    if (ballSpeedY == 0) ballSpeedY = oldBallSpeedY;
  }

  //limit to maximum speed
  if (ballSpeedY > MAX_Y_SPEED) ballSpeedY = MAX_Y_SPEED;
  if (ballSpeedY < -MAX_Y_SPEED) ballSpeedY = -MAX_Y_SPEED;
}

void centerPrint(char *text, int y, int size)
{
  pongDisplay->setTextSize(size);
  pongDisplay->setCursor(SCREEN_WIDTH/2 - ((strlen(text))*6*size)/2,y);
  pongDisplay->print(text);
}
