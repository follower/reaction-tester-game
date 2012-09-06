/*

   This is a version of my reaction tester game modified to work
   with an ATtiny85.

   ATtiny85 Pin-out:

      Reset (1)  ---  (8) Vcc
     Pin D3 (2) |   | (7) Pin D2 & SCK
     Pin D4 (3) |   | (6) Pin D1 & MISO
        GND (4)  ___  (5) Pin D0 & MOSI

   (see below for pins used)

 */

const int NUM_LEDS = 3;

int leds[NUM_LEDS] = {3, 4, 2};


const int buttonPin = 1;


void setup() {

  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, HIGH);
  
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], LOW);
  }

}

unsigned long finishTime = 0;

#define DEFAULT_WAIT 10
#define MAX_WAIT 2500

int waitTime = DEFAULT_WAIT;

unsigned long nextChange = 0;

int index = 0;

byte countdownControl[][3] = {
                     {0,0,0},
                     {1,0,1},
                     {0,0,0},
                     {0,0,0},
                     {1,0,1},
                     {0,0,0},
                     {0,0,0},
                     {1,1,1},
                     {0,0,0},
                     {0,0,0},                     
                   };


byte attractModeControl[][3] = {
                     {0,0,0},
                     {1,0,0},
                     {1,1,0},
                     {1,1,1},
                     {0,1,1},
                     {0,0,1},
                     {0,0,0},
                     {0,0,1},
                     {0,1,1},
                     {1,1,1},
                     {1,1,0},
                     {1,0,0},
                   };

#define POWERUP 0
#define START_COUNTDOWN 1
#define DO_COUNTDOWN 2
#define WAIT 3
#define WAIT_START 4
#define PLAY_GAME 5
#define START_LEVEL 6
#define PLAY_LEVEL 7
#define WIN_LEVEL 8
#define LOSE_LEVEL 9

int levelSpeed = 0; 

byte currentState = POWERUP;

boolean inKeyPress = false; 

void clearButtonState() {
  finishTime = 0;
  while (!(digitalRead(buttonPin) == HIGH)) {
    // wait for button to be released
  }
}

boolean buttonPress() {
  if (inKeyPress && (digitalRead(buttonPin) == LOW) && (millis() > finishTime)) {
    inKeyPress = false; // TODO: Wait until release?
    return true;
  }

  if ((digitalRead(buttonPin) == LOW) && !inKeyPress) {
    inKeyPress = true;
    finishTime = millis() + 50;
  } 
  
  return false;
}

void lightAll() {
  for (int i = 0; i < NUM_LEDS; i++) {
    digitalWrite(leds[i], HIGH);
  }
}

void unlightAll() {
  for (int i = 0; i < NUM_LEDS; i++) {
    digitalWrite(leds[i], LOW);
  }
}

byte gameDisplayControl [][3] = {
  {1,0,0},
  {0,1,0},
  {0,0,1},
  {0,1,0},
};

byte lossDisplayControl [][3] = {
  {1,0,1},
  {0,0,0},
  {1,0,1},
  {0,0,0},
  {0,0,0},
  {0,0,0},
};

byte winDisplayControl [][3] = {
  {0,1,0},
  {0,0,0},
  {0,1,0},
  {0,0,0},
};

void updateGameDisplay(byte *displayControl) {

  for (int i = 0; i < NUM_LEDS; i++) {
    digitalWrite(leds[i], displayControl[i]);
  }

}

boolean playAnimation(byte displayControl[][3], int maxIndex) {
       if (millis() > nextChange) {
         if (index < maxIndex) {
           updateGameDisplay(displayControl[index]);
           index++;
           nextChange = millis() + waitTime;
         } else {
           index = 0;
           return true;
         }
       }
     
     return false;  
}

void loop() {
  
  switch (currentState) {
     case POWERUP:
       currentState = WAIT_START;
       clearButtonState();
       lightAll();

       waitTime = 250; index = 0; nextChange = millis();
       delay(waitTime);
       break;
       
     case WAIT_START:
       if(playAnimation(attractModeControl, sizeof(attractModeControl)/3)) {
         waitTime -= 25;
         if (waitTime < 0) {
           lightAll();
           waitTime = 250;
           nextChange = millis() + waitTime;
         }
       }
       if (buttonPress()) {
         currentState = START_COUNTDOWN;
         randomSeed(millis()); // TODO: Use different source? And only once?
       }
       break;
       
     case START_COUNTDOWN:
       lightAll();
       delay(500);
       unlightAll();
       delay(500);

       waitTime = 250;
       nextChange = millis();
       // currentState = DO_COUNTDOWN; // The countdown just seemed to confuse people.
       currentState = PLAY_GAME;
       index = 0;
       break;

     case DO_COUNTDOWN:
       if (playAnimation(countdownControl, sizeof(countdownControl)/3)) {
         currentState = PLAY_GAME;
       }
       break;
       
     case PLAY_GAME:
       levelSpeed = 1100;
       currentState = START_LEVEL;
       break;  
       
     case START_LEVEL:
       levelSpeed -= 100;

       // Choose a side to start on
       if (random(2) == 0) {
         index = -1;
       } else {
         index = 1;
       }

       currentState = PLAY_LEVEL;
       nextChange = 0;
       clearButtonState();
       break;

     case PLAY_LEVEL:
       if (buttonPress()) {
         if ((index == 1) || (index == 3)) { // TODO: Handle this better.
           currentState = WIN_LEVEL;
         } else {
           currentState = LOSE_LEVEL;
         }
         index = 0;
         nextChange = millis();
         break;
       }
       
       if (millis() > nextChange) {
         index = (index + 1) % (sizeof(gameDisplayControl)/3);
         updateGameDisplay(gameDisplayControl[index]);
         nextChange = millis() + levelSpeed;
       }
       break;
       
     case LOSE_LEVEL:
       if (playAnimation(lossDisplayControl, sizeof(lossDisplayControl)/3)) {
         currentState = POWERUP;
       }        
       break;

     case WIN_LEVEL:
       if (playAnimation(winDisplayControl, sizeof(winDisplayControl)/3)) {
         currentState = START_LEVEL;
       }               
       break;
       
     case WAIT:
     default:
       break;
  }
             
}
  
