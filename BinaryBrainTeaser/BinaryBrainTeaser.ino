#include <LiquidCrystal.h>

#define BUTTON_TOL    30   // tolerance in analog reading

// game states
#define READY     0
#define PLAYING   1
#define COMPLETE  2

// nominal analog readings for each button
int butVals[] = {637, 405, 97, 252, 0};

LiquidCrystal lcd(8, 9, 4,5,6,7);

uint8_t binVal = 0;  
uint8_t binTarget = 0;
uint8_t score = 0;

uint8_t gameState = READY;

unsigned long gameTimer = 0;

// return button number (1-5) or zero for no button pressed
uint8_t whichButton() {
  int v = analogRead(0);
  uint8_t i;
  for (i=0; i<5; i++) {
    if ((v > butVals[i]-BUTTON_TOL) && (v < butVals[i] + BUTTON_TOL)) {
      return i+1;
    }
  }
  return 0;
}


void readyToStart() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Binary Challenge");

  lcd.setCursor(0,1);
  lcd.print("Press a button");

  while (whichButton() ==0) {
    delay(1);
  }
  
  lcd.clear();
  gameState = PLAYING;
  score = 0;
  binVal = 0;
  randomSeed(millis());
  binTarget = random(1,31);

  delay(500);
  
  gameTimer = millis() + 60000;
}

void playing() {
  // see if button input has changed
  static uint8_t lastButton = 0;
  uint8_t button = whichButton();

  if (button != lastButton) {
    if (button > 0) {
      binVal ^= 1 << (5-button);
    }
    
    lastButton = button;
  }

  // target value
  lcd.setCursor(0,0);
  lcd.print(binTarget);
  lcd.print(" ");

  // Current binary value
  lcd.setCursor(0,1);
  for (uint8_t i=0; i<5; i++) {
    lcd.print(1 & (binVal >> (4-i)));
  }

  // time remaining
  lcd.setCursor(14,0);
  lcd.print((gameTimer - millis()) / 1000);

  if (binVal == binTarget) {
    randomSeed(millis());
    uint8_t nv;
    do {
      nv = random(1,31);
    } while(nv == binTarget);
    binTarget = nv;
    score++;
    binVal = 0;
  }

  // score
  lcd.setCursor(13,1);
  lcd.print(score);

  
  

  if (millis() > gameTimer) {
    gameState = COMPLETE;
  }
}

void completed() {
  // display score
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("You Scored:");
  lcd.setCursor(0,1);
  lcd.print(score);

  long t = millis() + 30000;
  
  delay(5000);

  while ((whichButton() ==0) && (millis() < t)) {
    delay(1);
  }
  delay(500);
  lcd.clear();
  gameState = READY;
}


void setup() {
  lcd.begin(16, 2);
}

void loop() {
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

