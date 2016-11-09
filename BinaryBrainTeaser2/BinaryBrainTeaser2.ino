#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include "pitches.h"

/*************************************************************
 *  Defines
 *************************************************************/

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define AUDIO_PIN 1

#define LCD_RESET 13 // Can alternately just connect to Arduino's reset pin

#define BIT_0_PIN  A4
#define BIT_1_PIN  A5
#define BIT_2_PIN  10
#define BIT_3_PIN  11
#define BIT_4_PIN  12


// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define ORANGE  0xFDE0
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define DARKGREY  0x7BEF
#define LIGHTGREY 0xBDF7


// game states
#define READY     0
#define PLAYING   1
#define COMPLETE  2

/*************************************************************
 *  Globals
 *************************************************************/

// nominal analog readings for each button
int butVals[] = {637, 405, 97, 252, 0};

uint8_t binVal = 0;  
uint8_t binTarget = 0;
uint8_t score = 0;

uint8_t gameState = READY;

unsigned long gameTimer = 0;

// bit switches
uint8_t bitPins[5] = {BIT_0_PIN, BIT_1_PIN, BIT_2_PIN, BIT_3_PIN, BIT_4_PIN};

// TFT
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);


/*************************************************************
 *  Tunes
 *************************************************************/

int startupTuneLen = 8;
int startupTune[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};
int startupTuneDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

int beginTuneLen = 3;
int beginTune[] = {
  NOTE_C4, NOTE_B3, NOTE_C4
};
int beginTuneDurations[] = {
  4, 4, 4
};

int endTuneLen = 3;
int endTune[] = {
  NOTE_C4, NOTE_B4, NOTE_A4
};
int endTuneDurations[] = {
  4, 4, 4
};


void playNote(int note, int dur) {
  int noteDuration = 1000 / dur;
  tone(AUDIO_PIN, note, noteDuration);

  int pauseBetweenNotes = noteDuration * 1.30;
  delay(pauseBetweenNotes);
  noTone(AUDIO_PIN);
}


void playTune(int tune[], int durations[], int notes) {
  for (uint8_t thisNote = 0; thisNote < notes; thisNote++) {
    playNote(tune[thisNote], durations[thisNote]);
  }
}


/*************************************************************
 *  Setup
 *************************************************************/

void setup(void) {
  // pin modes
  for (uint8_t i=0; i<5; i++) {
    pinMode(bitPins[i], INPUT_PULLUP);
  }
  pinMode(AUDIO_PIN, OUTPUT);

  delay(100);
  
  //Serial.begin(9600);
  //Serial.println(F("Binary Brain Teaser 2"));
  
  tft.reset();

  uint16_t identifier = tft.readID();
  tft.begin(identifier);
}

/*************************************************************
 *  Loop
 *************************************************************/

void loop(void) {
  switch (gameState) {
    case READY:
      readyToStart();
      break;
      
    case PLAYING:
      playing();
      break;
      
    case COMPLETE:
      completed();
      break;
  }
}


/*************************************************************
 *  Game States
 *************************************************************/


void readyToStart() {
  tft.fillScreen(BLACK);
  tft.setRotation(1);

  tft.setCursor(20, 20);
  tft.setTextColor(WHITE);
  tft.setTextSize(6);
  tft.println("Binary");
  tft.println(" Brain");
  tft.println("  Teaser");

  tft.setTextColor(GREEN);
  tft.setCursor(30, 200);
  tft.setTextSize(2);
  tft.print("Flip a switch to start");

  // make sure we've got latest switch values
  readSwitches();

  playTune(startupTune, startupTuneDurations, startupTuneLen);
  
  // wait for change in binVal
  uint8_t lbv = binVal;
  do {
    readSwitches();
  } while (binVal == lbv);
  
  tft.fillScreen(BLACK);
  gameState = PLAYING;
  score = 0;
  binVal = 0;
  randomSeed(millis());
  binTarget = random(1,31);

  playTune(beginTune, beginTuneDurations, beginTuneLen);
  
  gameTimer = millis() + 60000;
}

void readSwitches() {
  binVal = 0;
  for (uint8_t i=0; i<5; i++) {
    binVal |= !digitalRead(bitPins[i]) << (4-i);
  }
}

void drawBinaryValue(unsigned long fgc, unsigned long bgc) {
  
  // does not equal sign
  

  // Current binary value
  tft.setTextColor(fgc, bgc);
  tft.setCursor(60,170);
  tft.setTextSize(7);
  for (uint8_t i=0; i<5; i++) {
    tft.print(1 & (binVal >> (4-i)));
  }
}

void playing() {
  readSwitches();

  // target value
  tft.setCursor(110, 35);
  tft.setTextColor(ORANGE, BLACK);
  tft.setTextSize(10);
  tft.print(binTarget);
  tft.print(" ");

  drawBinaryValue(WHITE, BLACK);

  if (binVal == binTarget) {
    // yay!!!  got it right, flash green for 1 sec
    drawBinaryValue(GREEN, BLACK);

    playNote(NOTE_C4, 4);
    
    delay(1000);
    gameTimer += 1000;
    
    randomSeed(millis());
    uint8_t nv;
    do {
      nv = random(1,31);
    } while(nv == binTarget);
    binTarget = nv;
    score++;
    binVal = 0;

    tft.fillScreen(BLACK);
  }

  // time remaining
  tft.setCursor(280,0);
  tft.setTextColor(LIGHTGREY, BLACK);
  tft.setTextSize(2);
  tft.print((gameTimer - millis()) / 1000);

  // score
  tft.setCursor(10,0);
  tft.print(score);

  if (millis() > gameTimer) {
    gameState = COMPLETE;
  }
}

void completed() {
  tft.fillScreen(BLACK);
  tft.setRotation(1);

  tft.setCursor(40, 40);
  tft.setTextColor(WHITE);
  tft.setTextSize(4);
  tft.print("You Scored");
 
  tft.setCursor(90, 120);
  tft.setTextColor(GREEN);
  tft.setTextSize(11);
  tft.print(score);

  long t = millis() + 10000;

  playTune(endTune, endTuneDurations, endTuneLen);
  
  delay(5000);

  // wait for a bit
  while ( (millis() < t)) {
    delay(1);
  }
  delay(500);
  tft.fillScreen(BLACK);
  gameState = READY;
}


/*************************************************************
 *  Display Utilities
 *************************************************************/

void testText() {
  //tft.fillScreen(BLACK);
  tft.setCursor(110, 20);
  tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(10);
  tft.print(random(1,31));
  tft.println(" ");

  // pin states
  tft.setCursor(60,180);
  tft.setTextSize(7);
  for (uint8_t i=0; i<5; i++) {
    tft.print(digitalRead(bitPins[i]));
  }
}

