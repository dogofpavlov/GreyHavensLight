
#include <FastLED.h>
#include "FastLED_RGBW.h"


#define NUM_LEDS 39
#define DATA_PIN 2 
#define LED_TYPE WS2812B

CRGBW leds[NUM_LEDS];
CRGB *ledsRGB = (CRGB *) &leds[0];

int animationModesTotal = 7;
int animationModeIndex = 0;

//Mode 1 - Sunrise to full Warm to Sunset Loop
//Mode 2 - Sunrise to Sunset to Moonrise to Moonset to Sunrise Loop
//Mode 3 - Sunrise to full Warm
//Mode 4 - Sunrise to Halt
//Mode 5 - Moonrise to Moonset
//Mode 6 - Warm Glimmer
//Mode 7 - Moon Glimmer



void setup() {	
  Serial.begin(115200);
  delay(500);

  // Initialize FastLED library
	FastLED.addLeds<SK6812, DATA_PIN, RGB>(ledsRGB, getRGBWsize(NUM_LEDS));
	FastLED.setBrightness(255);

  modeSunriseAndSetSetup();
}

void loop() {
  loopLEDs();  
}

void loopLEDs(){

  switch(animationModeIndex){
    case 0:
      modeSunriseAndSetLoop();
    break;
  }
  FastLED.show();
}

int middleLEDIndex = 19;
int numLEDsPerSide = 19;

float ledStartStep[20] = {
  0.025, //0
  0.030, //1
  0.035, //2
  0.040, //3
  0.045, //4
  0.05, //5
  0.055, //6
  0.06, //7
  0.065, //8
  0.07, //9
  0.075, //10
  0.08, //11
  0.085, //12
  0.09, //13
  0.095, //14
  0.1, //15
  0.105, //16
  0.11, //17
  0.115, //18
  0.12 //19
};

void modeSunriseAndSetSetup(){
  
}


int sunStepIndex = 0;

int sunStepsTotal = 2000;
int sunTimescale = 3000;
int stepsPerLED = 500;

int direction = 1;

void modeSunriseAndSetLoop(){

  EVERY_N_MILLISECONDS(16){
    
    for(int i=0; i<NUM_LEDS; i++){

      int distance = abs(middleLEDIndex - i);

      float startStepPercent = ledStartStep[distance];
      int redStartStepIndex = sunTimescale*startStepPercent;

      float startStepWhitePercent = startStepPercent;
      if(i==middleLEDIndex){
        startStepWhitePercent = 0.01;
      }
      int whiteStartStepIndex = sunTimescale*((startStepWhitePercent*2)+0.025);

      float redProgress = 0;
      float whiteProgress = 0;
      float capWhiteProgress = 0;
      if(sunStepIndex>redStartStepIndex){
        float redProgression = sunStepIndex-redStartStepIndex;
        redProgress = (float)redProgression/stepsPerLED;

        if(redProgress>1){
          redProgress = 1;
        }
      }
      if(sunStepIndex>whiteStartStepIndex){
        float whiteProgression = sunStepIndex-whiteStartStepIndex;
        whiteProgress = (float)whiteProgression/stepsPerLED;

        capWhiteProgress = whiteProgress;
        if(capWhiteProgress>1){
          capWhiteProgress = 1;
        }

        if(distance<3){
          if(whiteProgress>1){

            redProgress = 1 - (whiteProgress-1);
            if(redProgress<0){
              redProgress = 0;
            }
          }
        }
      }

      float white = 255 * capWhiteProgress;
      float red = 255 * redProgress;

      leds[i] = CRGBW(red,0,0,white);
    }

    sunStepIndex = sunStepIndex+direction;

    if(direction==1){
      if(sunStepIndex > sunStepsTotal){
        direction = -1;
      }
    }else{
      if(sunStepIndex<0){
        sunStepIndex = 0;
        direction = 1;
      }
    }

  }
}


/*

float sunriseRedSteps = 1000;
float sunriseRedStepIndex = 0;
float sunriseSteps = 500;
int sunriseStepIndexStart = -500;
*/


/*
int middleLEDIndex = 19;//index
const int numLEDsPerSide = 19;//count

float sunriseRedSteps = 1000;
float sunriseRedStepIndex = 0;
float sunriseSteps = 500;
int sunriseStepIndexStart = -500;
float sunriseStepIndex = sunriseStepIndexStart;
float sunriseRedFullBrightnessRate = 255/(numLEDsPerSide-1);
float sunriseFullBrightnessRate = 255/(numLEDsPerSide-1);
float sunriseBrightnessRate = 100;
float sunriseBrightnessRateDistance = sunriseBrightnessRate-sunriseFullBrightnessRate;
float sunriseBrightnessRedRateDistance = sunriseBrightnessRate-sunriseRedFullBrightnessRate;
float sunriseRatePerStep = sunriseBrightnessRateDistance/sunriseSteps;
float sunriseRedRatePerStep = sunriseBrightnessRedRateDistance/sunriseRedSteps;

int sunriseAnimationSteps = 2000;
int sunriseAnimationStepIndex = 0;

int sunriseAnimationDirection = 1;


void modeSunriseAndSetSetup(){
  sunriseStepIndex = sunriseStepIndexStart;
  sunriseRedStepIndex = 0;
}

void modeSunriseAndSetLoop(){

  EVERY_N_MILLISECONDS(16){
    
    if(sunriseAnimationStepIndex==sunriseAnimationSteps){
      sunriseAnimationStepIndex = 0;
      if(sunriseAnimationDirection==1){
        sunriseAnimationDirection = -1;
        //reset indexes for reverse
        //sunriseStepIndex = sunriseSteps*2;
        //sunriseRedStepIndex = sunriseRedSteps;
      }else{
        sunriseAnimationDirection = 1;
        //reset indexes for forward
        //sunriseStepIndex = sunriseStepIndexStart;
        //sunriseRedStepIndex = 0;
      }
    }
    sunriseAnimationStepIndex++;

    float brightnessRate = sunriseBrightnessRate-(sunriseStepIndex*sunriseRatePerStep);
    float brightnessRedRate = sunriseBrightnessRate-(sunriseRedStepIndex*sunriseRedRatePerStep);

    sunriseStepIndex = sunriseStepIndex+sunriseAnimationDirection;
    if(sunriseStepIndex>sunriseSteps){
      sunriseStepIndex = sunriseSteps;
    }
    if(sunriseStepIndex<sunriseStepIndexStart){
      sunriseStepIndex=sunriseStepIndexStart;
    }
    sunriseRedStepIndex = sunriseRedStepIndex+sunriseAnimationDirection;
    if(sunriseRedStepIndex>sunriseRedSteps){
      sunriseRedStepIndex = sunriseRedSteps;
    }
    if(sunriseRedStepIndex<0){
      sunriseRedStepIndex=0;
    }

    float stepProgressionPercent = sunriseStepIndex/sunriseSteps;
    float stepProgressionRedPercent = sunriseRedStepIndex/sunriseRedSteps;

    //Serial.println(".");
    for(int i=0; i<NUM_LEDS; i++){
      int distanceFromMid = abs(middleLEDIndex - i);
      float brightnessReduction = distanceFromMid*brightnessRate;
      float brightnessRedReduction = distanceFromMid*brightnessRedRate;

      float white = (255*(stepProgressionPercent)) - brightnessReduction;

      float redPer = 0.3;
      float red = ((255*stepProgressionRedPercent) - brightnessRedReduction)*redPer;

      if(white<0){
        white = 0;
      }
      if(white>255){
        white = 255;
      }
      if(red<0){
        red = 0;
      }
      if(red>255){
        red = 255;
      }
      leds[i] = CRGBW(red,0,0,white);
    }
  }
}
*/
