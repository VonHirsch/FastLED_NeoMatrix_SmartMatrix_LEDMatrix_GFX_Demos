#include <EasyButton.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <arduino-timer.h>

//#include "InvaderSprites.h"
#include "pong.h"

//#define OTA

#ifdef OTA
    #include "OTA.h"
#endif

#define RANDOM_MODE_SECONDS 300
#define WAIT_ON_RANDOM_MODE_SELECT_SECONDS 5
#define WAIT_ON_RETURN_TO_DEFAULT_DISPLAY_SECONDS 2
#define BLINKY_DOT_MILLIS 1000

////////////////////////////////v   
// Prototypes
////////////////////////////////

void IRAM_ATTR PinA();
void IRAM_ATTR PinB();
void setupEncoder();
bool drawBlinkyDot(void *);
void drawDot(uint16_t x, uint16_t y, uint16_t color);
void drawSquare(uint16_t x, uint16_t y, uint16_t size, uint16_t color); 
void drawBorders();
void drawLargeText(const char* effect);
void drawSmallText(const char* effect);
void drawTitleText();
void encoderButtonPress();
void changeEffect();
bool chooseRandomEffect(void*);
bool waitingOnRandomModeSelectExpired(void*);
void readEncoder();
void loopTicker();
void readPot();
void setupDisplay();
void resetDisplay();
void setupSSD1306Ascii();
void setupPot();
bool chooseRandomEffect(void*);
bool waitingOnRandomModeSelectExpired(void*);
void printSeconds();
void turnOnRandomMode();

////////////////////////////////
// Used for time conversions
////////////////////////////////

long day = 86400000; // 86400000 milliseconds in a day
long hour = 3600000; // 3600000 milliseconds in an hour
long minute = 60000; // 60000 milliseconds in a minute
long second = 1000; // 1000 milliseconds in a second

////////////////////////////////
// Timers
////////////////////////////////

auto changeToRandomEffectTimer = timer_create_default();
auto waitingOnRandomModeSelectTimer = timer_create_default();
auto waitingToReturnToDefaultDisplayTimer = timer_create_default();
auto blinkyDotTimer = timer_create_default();
bool randomModeOn = false;
bool tickerModeOn = false;
bool pongModeOn = false;
bool pongGameRunning = false;
bool activelyAdjustingBrightness = false;
bool waitingOnRandomModeSelect = false;
bool waitingToReturnToDefaultDisplay = false;
bool debugTimers = false;

////////////////////////////////
// EasyButton
////////////////////////////////

// Arduino pin where the button is connected to.
#define BUTTON_PIN 32

//EasyButton button(BUTTON_PIN, debounce, pullup, invert);
EasyButton encoderButton(BUTTON_PIN);

////////////////////////////////
// Rotary Encoder
////////////////////////////////

static int pinA = 35;
static int pinB = 34;
volatile byte aFlag = 0;
volatile byte bFlag = 0;
volatile byte encoderPos = 0;
volatile byte oldEncPos = 0;
volatile byte reading = 0;

void IRAM_ATTR PinA()
{
    cli();
    reading = GPIO_REG_READ(GPIO_IN1_REG) & 0xC;
    if (reading == B1100 && aFlag)
    {
        encoderPos--;
        bFlag = 0;
        aFlag = 0;
    }
    else if (reading == B1000)
        bFlag = 1;
    sei();
}

void IRAM_ATTR PinB()
{
    cli();
    reading = GPIO_REG_READ(GPIO_IN1_REG) & 0xC;
    if (reading == B1100 && bFlag)
    {
        encoderPos++;
        bFlag = 0;
        aFlag = 0;
    }
    else if (reading == B100)
        aFlag = 1;
    sei();
}

void setupEncoder()
{
    pinMode(pinA, INPUT);
    pinMode(pinB, INPUT);
    attachInterrupt(digitalPinToInterrupt(pinA), PinA, RISING);
    attachInterrupt(digitalPinToInterrupt(pinB), PinB, RISING);

    encoderButton.begin();
    encoderButton.onPressed(encoderButtonPress);
}

void encoderButtonPress() {
    Serial.println("encoderButtonPress");       
    if (!pongModeOn && !tickerModeOn) {
        drawLargeText("");  //clear text
        tickerModeOn = true;
    } else {
        if (pongModeOn) {
            if (pongGameRunning) {              
              pongModeOn = false;
              pongGameRunning = false;
              resetDisplay();
              changeEffect();   // call changeEffect to re-display the currently playing effect
            } else {
              pongGameRunning = true;
              startPong();          
            }
        } else {        
            resetPong();
            pongModeOn = true;
            tickerModeOn = false;
        }       
    }   
}

////////////////////////////////
// SSD1306Ascii Display Lib
////////////////////////////////

#define RTN_CHECK 1

#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1

SSD1306AsciiWire oled;

// Ticker state. Maintains text pointer queue and current ticker state.
TickerState state;

const char* text[] = {
    "ESP32 16x16 Panel Display. ",
    "Created for the Ogrady's. ",
    "Aurora visual Demos by Jason Coon. ",
    "FastLED::NeoMatrix by Marc Merlin. ",
    "This Mash-up created by Bart Hirst.",
    "                 ",
};

#define TEXT_LINES 6

const uint8_t* fontList[] = {
    Arial14,
    Arial_bold_14,
    Callibri11,
    Callibri11_bold,
    Callibri11_italic,
    Callibri15, // 5
    Corsiva_12,
    fixed_bold10x15,
    font5x7,
    font8x8,
    Iain5x7, // 10
    lcd5x7,
    Stang5x7,
    System5x7,
    TimesNewRoman16,
    TimesNewRoman16_bold, // 15
    TimesNewRoman16_italic,
    utf8font10x16,
    Verdana12,
    Verdana12_bold,
    Verdana12_italic, // 20
    X11fixed7x14,
    X11fixed7x14B,
    ZevvPeep8x16
};

#define TICKER_FONT_INDEX 5

////////////////////////////////
// Potentiometer
////////////////////////////////

const int potPin = 33;
int potValue = 0;
int potMax = 2047;
int potMin = 0;

////////////////////////////////
// Adafruit_SSD1306
////////////////////////////////

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

////////////////////////////////
// Setup
////////////////////////////////

void setup()
{
    delay(1000);
    Serial.begin(115200);
    randomSeed(analogRead(0));

    #ifdef OTA
        setupOTA();
    #endif
    
    setupDisplay();
    setupPong(&display);
    setupEncoder();
    setupPot();    
    
    matrix_setup();
    matrix->begin();
    matrix->setBrightness(matrix_brightness);
    matrix->setTextWrap(false);
    matrix->clear();

    effects.leds = matrixleds;
    effects.Setup();

    // turn on random mode by default
    turnOnRandomMode();

    Serial.print("Matrix Size: ");
    Serial.print(mw);
    Serial.print(" ");
    Serial.println(mh);
}

void setupDisplay()
{
    setupSSD1306Ascii();
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
    }
    resetDisplay();
    blinkyDotTimer.every(BLINKY_DOT_MILLIS, drawBlinkyDot);
}

void resetDisplay() {
    display.clearDisplay();
    drawBorders();
    drawTitleText();
}

void setupSSD1306Ascii()
{
#if RST_PIN >= 0
    oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
    oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0

    // Setup Ticker
    // oled.tickerInit ( state, font, row, set2x?, from_column, to_column)
    oled.tickerInit(&state, fontList[TICKER_FONT_INDEX], 3, true, 1, display.width() - 3);
    
}

void setupPot() {
  pinMode(potPin,INPUT);
  adcAttachPin(potPin);
  analogReadResolution(11);
  analogSetAttenuation(ADC_6db);  
}

////////////////////////////////
// Loop
////////////////////////////////

#define EFFECT_COUNT 8
uint8_t throttleTicker = 0;
uint8_t readControls = 0;
uint8_t effectCounter = 0;
uint8_t effectCurrent = 0;

void loop()
{
    #ifdef OTA
        loopOTA();
    #endif

    encoderButton.read();

    if (pongModeOn || readControls % 10 == 0)
    {
        readPot();
        readEncoder();        
    }
    readControls++;
    
    if (pongModeOn) {
        loopPong();        
    }
    
    if (tickerModeOn) {
        if (throttleTicker % 2 == 0) { 
            loopTicker();
        }
        throttleTicker++;
    }
    
    switch (effectCurrent)
    {
    case 0:
        patternRadar.drawFrame();
        break;
    case 1:
        patternCube.drawFrame();
        break;
    case 2:
        patternSwirl.drawFrame();
        break;
    case 3:        
        if (!tickerModeOn) delay(50);   // Effect looks better when a bit slower
        patternAttract.drawFrame();
        break;
    case 4:
        if (!tickerModeOn) delay(10);   // Effect looks better when a bit slower
        patternFlock.drawFrame();
        break;
    case 5:
        patternSpiral.drawFrame();
        break;
    case 6:
        patternSpiro.drawFrame();
        break;
    case 7:
        // TODO Create "Random Mode" that switches every 30 minutes or something
        patternSpiro.drawFrame();
        break;
    }

    matrix->show();    

    if (randomModeOn)
    {
        changeToRandomEffectTimer.tick();
    }

    if (waitingOnRandomModeSelect)
    {
        // short timer is used to activate the random effect a few seconds after the user chooses it 
        waitingOnRandomModeSelectTimer.tick();
    }

    if (waitingToReturnToDefaultDisplay)
    {
        // short timer is used to return to the normal display mode after brightness tweaking
        waitingToReturnToDefaultDisplayTimer.tick();
    }
    
    blinkyDotTimer.tick();
}

////////////////////////////////
// SSD1306Ascii Scrolling Ticker
////////////////////////////////

uint32_t tickTime = 0;
int n = 0;

void loopTicker()
{
    Wire.setClock(400000L); // 400 kHz I2C rate
    if (tickTime <= millis())
    {
        tickTime = millis() + 30;

        int8_t rtn = oled.tickerTick(&state);

        // See above for definition of RTN_CHECK.
        if (rtn <= RTN_CHECK)
        {
            // Should check for error. Return of false indicates error.
            oled.tickerText(&state, text[(n++) % TEXT_LINES]);
        }
    }
}

////////////////////////////////
// Effect Switching
////////////////////////////////

bool debugEffectCounter = false;

void changeEffect()
{
    // if the ticker is running when the effect changes (due to user select, or random mode) turn it off
    //  they can always resume the ticker message by pressing the button once again
    tickerModeOn = false;
  
    if (debugEffectCounter) Serial.print(": effectCounter: ");
    if (debugEffectCounter) Serial.print(effectCounter);

    effectCurrent = effectCounter % EFFECT_COUNT;

    if (debugEffectCounter) Serial.print(": effectCurrent: ");
    if (debugEffectCounter) Serial.println(effectCurrent);

    switch (effectCurrent)
    {
    case 0:
        patternRadar.start();
        drawLargeText("Radar");
        break;
    case 1:
        patternCube.start();
        drawLargeText("Cube");
        break;
    case 2:
        patternSwirl.start();
        drawLargeText("Swirl");
        break;
    case 3:
        patternAttract.start();
        drawLargeText("Attract");
        break;
    case 4:
        patternFlock.start();
        drawLargeText("Flock");
        break;
    case 5:
        patternSpiral.start();
        drawLargeText("Spiral");
        break;
    case 6:
        patternSpiro.start();
        drawLargeText("Spiro");
        break;
        // Always make Random the last effect, because the random code only generates a random effect up to (EFFECT_COUNT - 1)          
    case 7:
        // if the user selects random and is idle for WAIT_ON_RANDOM_MODE_SELECT_SECONDS, start the random effect mode 
        drawLargeText("Random");
        waitingOnRandomModeSelect = true;
        waitingOnRandomModeSelectTimer.cancel();
        waitingOnRandomModeSelectTimer.in(WAIT_ON_RANDOM_MODE_SELECT_SECONDS * 1000, waitingOnRandomModeSelectExpired);
        if (debugTimers)
        {
            printSeconds();
            Serial.println(" : chose random, waiting on user");
        }
        break;
    }
}

void turnOnRandomMode()
{
    if (debugTimers)
    {
        printSeconds();
        Serial.println(" : waitingOnRandomModeSelectExpired, randomModeOn!");
    }
        
    changeToRandomEffectTimer.cancel();
    changeToRandomEffectTimer.every(RANDOM_MODE_SECONDS * 1000, chooseRandomEffect);
    randomModeOn = true;
    drawTitleText();
    chooseRandomEffect(NULL);
}

////////////////////////////////
// Timer Callbacks
////////////////////////////////

bool chooseRandomEffect(void*)
{
    if (debugTimers)
    {
        printSeconds();
        Serial.println(" : chooseRandomEffect");
    }

    if (randomModeOn)
    {
        effectCounter = random(EFFECT_COUNT - 1);
        // Effect "random" is always the last effect in the list so exclude it
        changeEffect();
    }
    return true;
}

bool waitingOnRandomModeSelectExpired(void*)
{
    turnOnRandomMode();
    return false;
}

bool waitingToReturnToDefaultDisplayExpired(void*)
{
    if (debugTimers)
    {
        printSeconds();
        Serial.println(" : waitingToReturnToDefaultDisplayExpired");
    }
    drawTitleText();
    activelyAdjustingBrightness = false;
    waitingToReturnToDefaultDisplayTimer.cancel();
    return false;
}


////////////////////////////////
// Effect Choice Rotary Encoder
////////////////////////////////

uint8_t debugEncoder = 0;

void readEncoder()
{        
    
    if (oldEncPos != encoderPos)
    {

        uint8_t encoderIncrement = 0;
        
        if (debugTimers)
        {
            printSeconds();
            Serial.println(" : cancel randomMode && waitingOnRandomModeSelect!");
        }

        if (randomModeOn && !pongModeOn)
        {
            // turn random mode off if any user input
            randomModeOn = false;
            drawTitleText();
        }

        waitingOnRandomModeSelect = false;

        if (debugEncoder == 1) Serial.print("encoderPos: ");
        if (debugEncoder == 1) Serial.print(encoderPos);

        if (encoderPos == 255 && oldEncPos == 0)
        {
            encoderIncrement--;
        }
        else if (encoderPos == 0 && oldEncPos == 255)
        {
            encoderIncrement--;
        }
        else if (oldEncPos > encoderPos)
        {
            encoderIncrement--;
        }
        else
        {
            encoderIncrement++;
        }

        if (pongModeOn) {
            setControlB(encoderPos);
        } else {
            effectCounter += encoderIncrement;
            changeEffect();
        }
        
        oldEncPos = encoderPos;
    }
}

////////////////////////////////
// Brightness Potentiometer
////////////////////////////////

bool debugPot = false;
uint8_t brightness = 0;
uint8_t deadzone = 2;

void readPot()
{

    potValue = analogRead(potPin);

    if (pongModeOn) {
        setControlA(potValue);
        return;
    }
    
    if (debugPot) Serial.print("potValue: ");
    if (debugPot) Serial.print(potValue);

    uint8_t newBrightness = map(potValue, potMin, potMax, 1, 255);

    if (debugPot) Serial.print(" | brightness: ");
    if (debugPot) Serial.print(brightness);

    if (debugPot) Serial.print(" | newBrightness: ");
    if (debugPot) Serial.println(newBrightness);
    
    if (abs(newBrightness - brightness) >= deadzone) {
                 
        if (debugPot) Serial.print("****** set brightness: ");
        if (debugPot) Serial.println(newBrightness);
        if (newBrightness > 0 || newBrightness <= 255) {  // deadzone       
            matrix->setBrightness(newBrightness);           
        }
        
        if (brightness > 0) {   // this check prevents the brightness being displayed on the first brightness adjustment during boot
  
            activelyAdjustingBrightness = true;
      
            String brightnessString = "Brightness: ";
            String brightnessValue = String(newBrightness);
                    
            drawSmallText((brightnessString + brightnessValue).c_str());
    
            // start a timer to return the display to normal after some time
            waitingToReturnToDefaultDisplay = true;
            waitingToReturnToDefaultDisplayTimer.cancel();
            waitingToReturnToDefaultDisplayTimer.in(WAIT_ON_RETURN_TO_DEFAULT_DISPLAY_SECONDS * 1000, waitingToReturnToDefaultDisplayExpired);
            if (debugTimers)
            {
                printSeconds();
                Serial.println(" : activelyAdjustingBrightness, waiting on user");
            }
            
        }
        
        brightness = newBrightness;
       
    }
    
}

////////////////////////////////
// Draw Routines
////////////////////////////////

void drawTitleText()
{
    if (randomModeOn) {
        drawSmallText("Random Mode");
    } else {
        drawSmallText("ESP32 LED Panel");
    }   
}

void drawBorders()
{
    // Top Rect
    // drawn with text now
    display.drawRect(0, 0, display.width() - 1, 15, SSD1306_WHITE);
    // Bottom Rect
    display.drawRect(0, 16, display.width() - 1, display.height() - 16, SSD1306_WHITE);
}

void drawLargeText(const char* effect)
{
    int16_t x1, y1;
    uint16_t w, h;

    int centeredTextYStart = 0;

    // Draw centered text on a canvas buffer
    GFXcanvas1 canvas(SCREEN_WIDTH - 3, SCREEN_HEIGHT / 2);

    // Adjust canvas cursor offset slightly based on text size
    if (strlen(effect) > 6)
    {
        canvas.setTextSize(2);
        centeredTextYStart += 5; // smaller text, move down a little to center
    }
    else
    {
        canvas.setTextSize(3);
    }

    canvas.getTextBounds(effect, 0, 0, &x1, &y1, &w, &h);
    int centeredTextXStart = (SCREEN_WIDTH / 2) - (w / 2);
    canvas.setCursor(centeredTextXStart, centeredTextYStart);
    canvas.println(effect);

    // Copy centered text to display
    display.drawBitmap(1, 30, canvas.getBuffer(), canvas.width(), canvas.height(), SSD1306_WHITE, SSD1306_BLACK);
    // Copy to screen
    display.display();
}

void drawSmallText(const char* effect)
{
    int16_t x1, y1;
    uint16_t w, h;

    int centeredTextYStart = 0;

    // Draw centered text on a canvas buffer
    GFXcanvas1 canvas(SCREEN_WIDTH - 3, 8);
    canvas.setTextSize(1);

    canvas.getTextBounds(effect, 0, 0, &x1, &y1, &w, &h);
    int centeredTextXStart = (SCREEN_WIDTH / 2) - (w / 2);
    canvas.setCursor(centeredTextXStart, centeredTextYStart);
    canvas.println(effect);

    // Copy centered text to display
    display.drawBitmap(1, 3, canvas.getBuffer(), canvas.width(), canvas.height(), SSD1306_WHITE, SSD1306_BLACK);
    // Copy to screen
    display.display();
}


bool blinkyOn = true;
uint16_t dotX = display.width() - 5;
uint16_t dotY = 19;
uint16_t dotColor = WHITE;

bool drawBlinkyDot(void *)
{             
    // prevent the blinking the dot from flickering the ticker or flonging the pong ...
    if (tickerModeOn || pongModeOn) return true;

    //byte MotherShipType = random(2);
    //drawInvader(0, 0, MotherShipGfx[MotherShipType], MOTHERSHIP_WIDTH, MOTHERSHIP_HEIGHT, WHITE);
  
    if (blinkyOn) {
      dotColor = WHITE;
    } else {
      dotColor = BLACK;
    }    
    drawDot(dotX, dotY, dotColor);
    blinkyOn = !blinkyOn;    
    return true;
}

void drawDot(uint16_t x, uint16_t y, uint16_t color) {      
    display.drawPixel(x, y, color);
    display.display();  
}

void drawSquare(uint16_t x, uint16_t y, uint16_t size, uint16_t color) {         
    display.setCursor(x, y);
    display.drawRect(0, 0, size, size, color);    
    display.display();  
}

////////////////////////////////
// Debug
////////////////////////////////

void printSeconds()
{
    long timeNow = millis();
    int seconds = (((timeNow % day) % hour) % minute) / second;
    Serial.print(seconds);
}
