#include <FastLED.h>
#include "FastLED_RGBW.h"
#include <Bounce2.h>
#include <ButtonEvents.h>
#include <EEPROM.h>

#define NUM_LEDS 39
#define DATA_PIN 2 
#define BTN_PIN 4
#define LED_TYPE WS2812B

CRGBW leds[NUM_LEDS];
CRGB *ledsRGB = (CRGB *) &leds[0];

int modesTotal = 12;
int modeIndex = 0;
int eeAddress_mode = 0;
int eeAddress_brightness = 2;
int eeAddress_customSunIndex = 4;
int eeAddress_customMoonIndex = 6;
int eeAddress_glimmer = 8;
int eeAddress_rgbIndex = 10;

//FOR BRIGHTNESS SETTINGS
float brightnessPerLED = (float)255/NUM_LEDS;
int MAX_BRIGHTNESS_INDEX = 9;//our brightness indexes are 0,1,2,3,4,5,6,7,8,9... so 10 levels total
int brightnessIndex = MAX_BRIGHTNESS_INDEX;

int DEFAULT_CUSTOM_SUN_INDEX = 1000;
int DEFAULT_CUSTOM_MOON_INDEX = 0;
int customSunIndex = DEFAULT_CUSTOM_SUN_INDEX;
int customMoonIndex = 0;

ButtonEvents primaryButton = ButtonEvents(); 

int LOOP_SPEED = 16;

//Mode 0 - Sunrise to Sunset to Moonrise to Moonset Loop
const int ANI_SUNRISE_SUNSET_MOONRISE_MOONSET = 0;
//Mode 1 - Sunrise to Sunset Loop
const int ANI_SUNRISE_SUNSET = 1;
//Mode 2 - Moonrise to Moonset Loop
const int ANI_MOONRISE_MOONSET = 2;
//Mode 3 - Halted Sunrise
const int SOLID_SUNRISE = 3;
//Mode 4 - Full Sun Light
const int SOLID_SUNLIGHT = 4;
//Mode 5 - Full Warm Light
const int SOLID_WARMLIGHT = 5;
//Mode 6 - Full White Light
const int SOLID_WHITELIGHT = 6;
//Mode 7 - Full Moon Light
const int SOLID_MOONLIGHT = 7;
//Mode 8 - Full Blue Light
const int SOLID_BLUELIGHT = 8;
//Mode 9 - Custom
const int SOLID_CUSTOM = 9;
//Mode 10 - RGB
const int SOLID_RGB = 10;
//Mode Settings
const int MODE_SETTINGS = 11;


const int rgbCOLORS[16][3] = {
  {255, 0, 0},       // Red
  {255, 165, 0},     // Orange
  {255, 255, 0},     // Yellow
  {50, 205, 50},     // Lime
  {0, 255, 0},       // Green
  {64, 224, 208},    // Turquoise
  {0, 255, 255},     // Cyan
  {173, 216, 230},   // Light Blue
  {135, 206, 235},   // Sky Blue
  {65, 105, 225},    // Royal Blue
  {30, 144, 255},    // Dodger Blue
  {0, 191, 255},     // Deep Sky Blue
  {128, 0, 128},     // Purple
  {255, 0, 255},     // Magenta
  {255, 192, 203},   // Pink
  {238, 130, 238}    // Violet
};


int moonLEDIndex = 30;
int sunLEDIndex = 19;

int moonStepIndex = 0;
int moonStepsTotal = 2000;
int sunStepIndex = 0;
int sunStepsTotal = 2000; //2000
int sunTimescale = 3000;
int stepsPerLED = 500;
int moonStepsPerLED = 800;

int canChangeBrightness = 0;

int sunDirection = 1;
int moonDirection = 0;

int moonActivationPointIndex = 300;
int sunActivationPointIndex = 300;

int rgbIndex = 0;

int canGlimmer = 0;

int* glimmer;
float* glimmerSpeed;
int* glimmerCurrent;


int randomGlimmer(){
  return random(20,240);
}
float randomGlimmerSpeed(){
  return random(10,20)/10.0;
}

void setup() {	
  Serial.begin(115200);
  delay(500);

  glimmer = new int[NUM_LEDS];
  glimmerSpeed = new float[NUM_LEDS];
  glimmerCurrent = new int[NUM_LEDS];

  for(int i=0; i<NUM_LEDS; i++){
    glimmer[i] = randomGlimmer();
    glimmerSpeed[i] = randomGlimmerSpeed();
    glimmerCurrent[i] = 0;
  }

  //Load current configurations
  EEPROM.get(eeAddress_mode, modeIndex);
  if(modeIndex<0 || modeIndex>modesTotal-1){
    modeIndex = 0;//this should handle first time run when loading from memory.... EEPROM might say it's "-1"
  }
  EEPROM.get(eeAddress_brightness, brightnessIndex);
  if(brightnessIndex<0){
    brightnessIndex = MAX_BRIGHTNESS_INDEX;//this should handle first time run when loading from memory.... EEPROM might say it's "-1"
  }  
  EEPROM.get(eeAddress_customSunIndex, customSunIndex);
  if(customSunIndex<0){
    customSunIndex = DEFAULT_CUSTOM_SUN_INDEX;//this should handle first time run when loading from memory.... EEPROM might say it's "-1"
  }
  EEPROM.get(eeAddress_customMoonIndex, customMoonIndex);
  if(customMoonIndex<0){
    customMoonIndex = DEFAULT_CUSTOM_MOON_INDEX;//this should handle first time run when loading from memory.... EEPROM might say it's "-1"
  }
  EEPROM.get(eeAddress_glimmer, canGlimmer);
  if(canGlimmer<0){
    canGlimmer = 0;//this should handle first time run when loading from memory.... EEPROM might say it's "-1"
  }
  EEPROM.get(eeAddress_rgbIndex, rgbIndex);
  if(rgbIndex<0){
    rgbIndex = 0;//this should handle first time run when loading from memory.... EEPROM might say it's "-1"
  }






  Serial.println(F("Animation"));
  Serial.println(modeIndex);

  Serial.println(F("Brightness"));
  Serial.println(brightnessIndex);

  // Initialize FastLED library
	FastLED.addLeds<SK6812, DATA_PIN, RGB>(ledsRGB, getRGBWsize(NUM_LEDS));

  applyBrightness();

  initButtons();
  updateMode();
}

void initButtons(){

  primaryButton.attach(BTN_PIN, INPUT);  
  primaryButton.activeHigh();
  primaryButton.debounceTime(15);
  primaryButton.doubleTapTime(500);
  primaryButton.holdTime(2000);
  
}

void cycleReset(){
  moonStepIndex = 0;
  sunStepIndex = 0;
  sunDirection = 0;
  moonDirection = 0;
}

void modeSunriseSunsetMoonriseMoonset(){
  sunDirection = 1;
}
void modeSunriseSunset(){
  sunDirection = 1;
}
void modeMoonriseMoonset(){
  moonDirection = 1;
}
void modeSolidSunrise(){
  sunStepIndex = 500;
}
void modeSolidSunLight(){
  sunStepIndex = 1500;
}
void modeSolidWarmLight(){
}
void modeSolidWhiteLight(){
}
void modeSolidMoonLight(){
  moonStepIndex = 1500;
}
void modeSolidBlueLight(){

}
void modeSolidCustom(){
  sunStepIndex = customSunIndex;
  moonStepIndex = customMoonIndex;
}

void modeSolidRGB(){

}
void modeSettings(){

}


void updateMode(){
  cycleReset();
  applyBrightness();
  switch(modeIndex){
    case ANI_SUNRISE_SUNSET_MOONRISE_MOONSET:
      modeSunriseSunsetMoonriseMoonset();
    break;
    case ANI_SUNRISE_SUNSET:
      modeSunriseSunset();
    break;
    case ANI_MOONRISE_MOONSET:
      modeMoonriseMoonset();
    break;
    case SOLID_SUNRISE:
      modeSolidSunrise();
    break;
    case SOLID_SUNLIGHT:
      modeSolidSunLight();
    break;
    case SOLID_WARMLIGHT:
      modeSolidWarmLight();
    break;
    case SOLID_WHITELIGHT:
      modeSolidWhiteLight();
    break;
    case SOLID_MOONLIGHT:
      modeSolidMoonLight();
    break;
    case SOLID_BLUELIGHT:
      modeSolidBlueLight();
    break;
    case SOLID_CUSTOM:
      modeSolidCustom();
    break;
    case SOLID_RGB:
      modeSolidRGB();
    break;
    case MODE_SETTINGS:
      modeSettings();
    break;
  }
}

void toggleGlimmer(){
  
  if(canGlimmer==0){
    canGlimmer=1;
  }else{
    canGlimmer=0;
  }
  EEPROM.put(eeAddress_glimmer,canGlimmer);
}
void nextPrevAnimationMode(int dir){
  modeIndex = modeIndex+dir;
  if(modeIndex==modesTotal){
    modeIndex = 0;
    toggleGlimmer();
  }
  if(modeIndex<0){
    modeIndex = modesTotal-1;
    toggleGlimmer();
  }
  updateMode();
  saveMode();
}
void saveMode(){
  EEPROM.put(eeAddress_mode,modeIndex);
}

void nextPrevBrightness(int dir){
  brightnessIndex = brightnessIndex+dir;
  if(brightnessIndex>MAX_BRIGHTNESS_INDEX){
    brightnessIndex=0;
  }
  if(brightnessIndex<0){
    brightnessIndex = MAX_BRIGHTNESS_INDEX;
  }
}


void applyBrightness(){

  if(modeIndex==MODE_SETTINGS){
    FastLED.setBrightness(255);
  }else{
    int calculatedBrightness = ((brightnessIndex+1)*4)*brightnessPerLED;
    if(brightnessIndex==MAX_BRIGHTNESS_INDEX){
      calculatedBrightness = 255;
    }
    FastLED.setBrightness(calculatedBrightness);
  }

}
void saveBrightness(){
  EEPROM.put(eeAddress_brightness,brightnessIndex);
}

void nextRGBColor(){
  rgbIndex = rgbIndex+1;
  if(rgbIndex==16){
    rgbIndex = 0;
  }
  EEPROM.put(eeAddress_rgbIndex,rgbIndex);
}

void modeHoldAction(){

  switch(modeIndex){
    case ANI_SUNRISE_SUNSET:
      customSunIndex = sunStepIndex;
      customMoonIndex = 0;
      setCustomMode();
    break;
    case ANI_MOONRISE_MOONSET:
      customMoonIndex = moonStepIndex;
      customSunIndex = 0;
      setCustomMode();
    break;
    case SOLID_RGB:
      nextRGBColor();
    break;
  }
}
void setCustomMode(){
    modeIndex = SOLID_CUSTOM;
    EEPROM.put(eeAddress_customSunIndex,customSunIndex);
    EEPROM.put(eeAddress_customMoonIndex,customMoonIndex);
    saveMode();
    updateMode();
}


/*
-------LOOPS---LOOPS---LOOPS---LOOPS---LOOPS---LOOPS---LOOPS---LOOPS---LOOPS---LOOPS---LOOPS---LOOPS
*/

void loop() {
  loopButtons();
  loopLEDs();  
}



void loopButtons(){
  bool changed = primaryButton.update();
  // Was the button pushed?
  if (changed){
    int event = primaryButton.event(); // Get how the button was pushed
    switch(event){
      case(tap):
        Serial.println(F("Primary button single press..."));
        if(canChangeBrightness==0){
          nextPrevAnimationMode(1);
        }else{
          nextPrevBrightness(1);
        }

        break;
      case (doubleTap):
        Serial.println(F("Primary button double press..."));
        if(canChangeBrightness==0){
          nextPrevAnimationMode(-1);
        }else{
          nextPrevBrightness(-1);
        }

        break;
      case (hold):
        Serial.println(F("Primary button long press..."));
            if(modeIndex==MODE_SETTINGS){
              if(canChangeBrightness==0){
                //enter brightness mode change
                canChangeBrightness = 1;
              }else{
                //exit brightness mode change...and save
                canChangeBrightness = 0;
                saveBrightness();
              }
            }else{
              modeHoldAction();
            }
        break;
    }
  }
}

void loopLEDs(){

  switch(modeIndex){
    case ANI_SUNRISE_SUNSET_MOONRISE_MOONSET:
      modeSunriseSunsetMoonriseMoonsetLoop();
    break;
    case ANI_SUNRISE_SUNSET:
      modeSunriseSunsetLoop();
    break;
    case ANI_MOONRISE_MOONSET:
      modeMoonriseMoonsetLoop();
    break;
    case SOLID_SUNRISE:
      modeSolidSunriseLoop();
    break;
    case SOLID_SUNLIGHT:
      modeSolidSunLightLoop();
    break;
    case SOLID_WARMLIGHT:
      modeSolidWarmLightLoop();
    break;
    case SOLID_WHITELIGHT:
      modeSolidWhiteLightLoop();
    break;
    case SOLID_MOONLIGHT:
      modeSolidMoonLightLoop();
    break;
    case SOLID_BLUELIGHT:
      modeSolidBlueLightLoop();
    break;
    case SOLID_CUSTOM:
      modeSolidCustomLoop();
    break;
    case SOLID_RGB:
      modeSolidRGBLoop();
    break;
    case MODE_SETTINGS:
      modeSettingsLoop();
    break;
  }
  
  FastLED.show();
}

void glimmerUpdate(int i){
  if(canGlimmer==1){
    int glimmerTarget = glimmer[i];
    float glimmerTargetSpeed = glimmerSpeed[i];
    //increase/decrease the current value
    glimmerCurrent[i] = glimmerCurrent[i]+glimmerTargetSpeed;

    //if increasing
    if(glimmerTargetSpeed>0){
      //and current is greater or equal to target
      if(glimmerCurrent[i]>=glimmerTarget){
        //flip the speed
        glimmerSpeed[i] = -glimmerTargetSpeed;
      }
    }
    //if decreasing
    if(glimmerTargetSpeed<0){
      //and current is less or equal to zero
      if(glimmerCurrent[i]<=0){
        //flip the speed
        glimmerSpeed[i] = randomGlimmerSpeed();
        glimmer[i] = randomGlimmer();
      }
    }
  }
}

float applyGlimmer(float value, int i){

  if(canGlimmer){
    value = value - glimmerCurrent[i];

    if(value<0){
      value=0;
    }
    if(value>255){
      value=255;
    }
  }
  return value;
}

void lightCycleUpdate(){

    for(int i=0; i<NUM_LEDS; i++){

      int sunDistance = abs(sunLEDIndex - i);
      int moonDistance = abs(moonLEDIndex - i);


      float sunStartStepPercent = (sunDistance * 0.005) + 0.025;
      float moonStartStepPercent = (moonDistance * 0.005) + 0.025;
      
      int redStartStepIndex = sunTimescale*sunStartStepPercent;

      int moonStartStepIndex = sunTimescale*moonStartStepPercent;

      //allow offset for beginning of white to look more "sunlike"
      float startStepWhitePercent = sunStartStepPercent;
      if(sunDistance==0){
        startStepWhitePercent = 0.005;
      }else if(sunDistance==1){
        startStepWhitePercent = 0.015;
      }
      int whiteStartStepIndex = sunTimescale*((startStepWhitePercent*2) + 0.025);

      float redProgress = 0;
      float greenProgress = 0;
      float blueProgress = 0;

      float whiteProgress = 0;
      float capWhiteProgress = 0;
      
      if(sunStepIndex>redStartStepIndex){
        float redProgression = sunStepIndex-redStartStepIndex;
        redProgress = (float)redProgression/stepsPerLED;
      }
      if(sunStepIndex>whiteStartStepIndex){
        float whiteProgression = sunStepIndex-whiteStartStepIndex;
        whiteProgress = (float)whiteProgression/stepsPerLED;
        capWhiteProgress = whiteProgress;
      }

      if(moonStepIndex>moonStartStepIndex){
        float blueProgression = moonStepIndex-moonStartStepIndex;
        blueProgress = (float)blueProgression/moonStepsPerLED;
        if(blueProgress>0.5){
          blueProgress = 0.5;//max blue
        }

        //redProgress = (moonDistance*0.01)*blueProgress;

        if(moonDistance==0){
          blueProgress = blueProgress*5;
          redProgress = blueProgress;
          greenProgress = blueProgress;
        }
      }

      //safety caps
      if(redProgress>1) redProgress = 1;
      if(redProgress<0) redProgress = 0;
      if(greenProgress>1) greenProgress = 1;
      if(greenProgress<0) greenProgress = 0;
      if(blueProgress>1) blueProgress = 1;
      if(blueProgress<0) blueProgress = 0;
      if(capWhiteProgress>1) capWhiteProgress = 1;
      if(capWhiteProgress<0) capWhiteProgress = 0;

      float red = (255 * redProgress);
      float green = (255 * greenProgress);
      float blue = (255 * blueProgress);
      float white = (255 * capWhiteProgress);

      glimmerUpdate(i);
      red = applyGlimmer(red,i);
      green = applyGlimmer(green,i);
      blue = applyGlimmer(blue,i);
      white = applyGlimmer(white,i);

      leds[i] = CRGBW(red,green,blue,white);
    }

    moonStepIndex = moonStepIndex+moonDirection;
    sunStepIndex = sunStepIndex+sunDirection;
}


void modeSunriseSunsetMoonriseMoonsetLoop(){
  EVERY_N_MILLISECONDS(LOOP_SPEED){
    lightCycleUpdate();

    //Sun Progression
    if(sunDirection==1){
      //Sunrise
      if(sunStepIndex > sunStepsTotal){
        sunDirection = -1;
      }
    }else if(sunDirection==-1){
      //Sunset
      if(sunStepIndex<=moonActivationPointIndex){//Moon Activation
        if(moonDirection==0){
          moonDirection = 1;
        }
      }
      if(sunStepIndex<0){//Stop Sun
        sunStepIndex = 0;
        sunDirection = 0;
      }
    }

    //Moon Progression
    if(moonDirection==1){
      //Moonrise
      if(moonStepIndex>moonStepsTotal){
        moonDirection = -1;
      }
    }else if(moonDirection==-1){
      //Moonset
      if(moonStepIndex<=sunActivationPointIndex){//Sun Activation
        if(sunDirection==0){
          sunDirection=1;
        }
      }
      if(moonStepIndex<0){//Stop Moon
        moonStepIndex = 0;
        moonDirection = 0;
      }
    }
  }
}
void modeSunriseSunsetLoop(){
  EVERY_N_MILLISECONDS(LOOP_SPEED){
    lightCycleUpdate();

    if(sunDirection==1){
      if(sunStepIndex > sunStepsTotal){
        sunDirection = -1;
      }
    }else{
      if(sunStepIndex<0){
        sunDirection = 1;
      }
    }
  }
}
void modeMoonriseMoonsetLoop(){
  EVERY_N_MILLISECONDS(LOOP_SPEED){
    lightCycleUpdate();

    if(moonDirection==1){
      if(moonStepIndex>moonStepsTotal){
        moonDirection = -1;
      }
    }else{
      if(moonStepIndex<0){
        moonDirection = 1;
      }
    }
  }
}

void modeSolidSunriseLoop(){
  EVERY_N_MILLISECONDS(LOOP_SPEED){
    lightCycleUpdate();
  }
}
void modeSolidSunLightLoop(){
  EVERY_N_MILLISECONDS(LOOP_SPEED){
    lightCycleUpdate();
  }
}

void modeSolidWarmLightLoop(){
  EVERY_N_MILLISECONDS(LOOP_SPEED){
    for(int i=0; i<NUM_LEDS; i++){

      glimmerUpdate(i);
      float white = applyGlimmer(255,i);

      leds[i] = CRGBW(0,0,0,white);
    }
  }
}
void modeSolidWhiteLightLoop(){
  EVERY_N_MILLISECONDS(LOOP_SPEED){
    for(int i=0; i<NUM_LEDS; i++){


      glimmerUpdate(i);
      float red = applyGlimmer(255,i);
      float green = applyGlimmer(255,i);
      float blue = applyGlimmer(255,i);

      leds[i] = CRGBW(red,green,blue,0);
    }
  }
}

void modeSolidMoonLightLoop(){
  EVERY_N_MILLISECONDS(LOOP_SPEED){
    lightCycleUpdate();
  }
}
void modeSolidBlueLightLoop(){
  EVERY_N_MILLISECONDS(LOOP_SPEED){
    for(int i=0; i<NUM_LEDS; i++){


      glimmerUpdate(i);
      float red = applyGlimmer(50,i);
      float green = applyGlimmer(30,i);
      float blue = applyGlimmer(255,i);

      leds[i] = CRGBW(red,green,blue,0);
    }
  }
}

void modeSolidCustomLoop(){
  EVERY_N_MILLISECONDS(LOOP_SPEED){
    lightCycleUpdate();
  }
}

void modeSolidRGBLoop(){
  EVERY_N_MILLISECONDS(LOOP_SPEED){
    
    for(int i=0; i<NUM_LEDS; i++){
      glimmerUpdate(i);
      float red = applyGlimmer(rgbCOLORS[rgbIndex][0],i);
      float green = applyGlimmer(rgbCOLORS[rgbIndex][1],i);
      float blue = applyGlimmer(rgbCOLORS[rgbIndex][2],i);

      leds[i] = CRGBW(red,green,blue,0);
    }
  }
}



void modeSettingsLoop(){
  EVERY_N_MILLISECONDS(LOOP_SPEED){

    for(int i=0; i<NUM_LEDS; i++){
      int brightnessW = 0;
      int brightnessB = 0;
      int brightnessR = 0;
      if(i%4==0){
        brightnessW = (i+1)*brightnessPerLED;
      }
      if(i==brightnessIndex*4){
        if(canChangeBrightness){
          brightnessR = brightnessW;
        }else{
          brightnessB = brightnessW;
        }
        brightnessW = 0;
      }
      leds[i] = CRGBW(brightnessR,0,brightnessB,brightnessW);
    }
  }
}
