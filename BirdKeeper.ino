/*-------------------------------------------------------------------------
  Teensy 3.2 program to extend the day light duration for birds.

  Written by TurBoss for JauriaStudios INC,

  -------------------------------------------------------------------------
  This file is part of the BirdKeeper.

  BirdKeeper is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  BirdKeeper is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with BirdKeeper.  If not, see
  <http://www.gnu.org/licenses/>.
  -------------------------------------------------------------------------*/

// Libs
#include <TimeLib.h>
#include <EEPROM.h>
#include <Bounce2.h>
#include <MenuSystem.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

// Inputs

#define BUTTON_UP 4
#define BUTTON_DOWN 5
#define BUTTON_LEFT 6
#define BUTTON_RIGHT 7

// outputs

#define PWM_LEDS 20
#define TEST_LED0 11
#define TEST_LED1 12
#define TEST_LED2 13

// Variables


bool debug = true;
bool backlightOn = true;
int backlightCounter = 60;

bool drawSetup = false;
int editMenu = 0;

int configAddr = 0; // Stores if the device is configured
int dataAddr = 10; // Stores the main data in a Settings Struct

bool configured = false;

int processRuning = 0;

int startFadingInMorning = 0;
int startFadingOutMorning = 0;

int startFadingInNight = 0;
int startFadingOutNight = 0;

unsigned long previousMillis = 0;
unsigned long interval = 100;

unsigned long backlightPreviousMillis = 0;
unsigned long backlightInterval = 1000;

bool saveTime = false;

int setDay = 0;
int setMonth = 0;
int setYear = 0;

int setHour = 0;
int setMin = 0;
int setSec = 0;

bool edit_mm1_m1_start_HH = false;
bool edit_mm1_m1_start_MM = false;
bool edit_mm1_m1_start_SS = false;

bool edit_mm1_m1_stop_HH = false;
bool edit_mm1_m1_stop_MM = false;
bool edit_mm1_m1_stop_SS = false;


bool edit_mm1_m2_start_HH = false;
bool edit_mm1_m2_start_MM = false;
bool edit_mm1_m2_start_SS = false;

bool edit_mm1_m2_stop_HH = false;
bool edit_mm1_m2_stop_MM = false;
bool edit_mm1_m2_stop_SS = false;

bool edit_mm1_m3_HH = false;
bool edit_mm1_m3_MM = false;
bool edit_mm1_m3_SS = false;

bool edit_mm2_m1_DD = false;
bool edit_mm2_m1_MM = false;
bool edit_mm2_m1_YY = false;

bool edit_mm2_m2_HH = false;
bool edit_mm2_m2_MM = false;
bool edit_mm2_m2_SS = false;


int hoursInSecs = 0;
int minutesInSecs = 0;
float secs = 0;
float maxSecs = 0;
float lastSecs = 0;

int pwmResolution = 4095;

float fade = 0.0;
float fadeInc = 0.0;
float fadeDec = 0.0;

// Instantiate the bouncers

Bounce up = Bounce();
Bounce down = Bounce();
Bounce left = Bounce();
Bounce right = Bounce();

// Menu variables

MenuSystem ms;

Menu rm("Jauria Studios INC");

Menu mm1("Temporizador");

Menu mm1_m1("Amanecer");

Menu mm1_m1_start("Hora inicio");

MenuItem mm1_m1_start_HH("HH");
MenuItem mm1_m1_start_MM("MM");
MenuItem mm1_m1_start_SS("SS");

Menu mm1_m1_stop("Hora fin");

MenuItem mm1_m1_stop_HH("HH");
MenuItem mm1_m1_stop_MM("MM");
MenuItem mm1_m1_stop_SS("SS");

Menu mm1_m2("Anochecer");

Menu mm1_m2_start("Hora inicio");

MenuItem mm1_m2_start_HH("HH");
MenuItem mm1_m2_start_MM("MM");
MenuItem mm1_m2_start_SS("SS");

Menu mm1_m2_stop("Hora fin");

MenuItem mm1_m2_stop_HH("HH");
MenuItem mm1_m2_stop_MM("MM");
MenuItem mm1_m2_stop_SS("SS");

Menu mm1_m3("Duracion");

MenuItem mm1_m3_HH("HH");
MenuItem mm1_m3_MM("MM");
MenuItem mm1_m3_SS("SS");

Menu mm2("Ajuste fecha/hora");

Menu mm2_m1("Fecha");

MenuItem mm2_m1_DD("DD");
MenuItem mm2_m1_MM("MM");
MenuItem mm2_m1_YY("YY");

Menu mm2_m2("Hora");

MenuItem mm2_m2_HH("HH");
MenuItem mm2_m2_MM("MM");
MenuItem mm2_m2_SS("SS");

MenuItem m_BACK("Volver");
MenuItem m_SAVE("Guardar");
MenuItem m_RUN("Ejecutar");

// Init LCD display (20x4)
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7);
LCD *screen = &lcd;

// Settings stored data
struct Settings {

  int morningStartHour; // Start fading on Morning
  int morningStartMin;
  int morningStartSec;

  int morningStopHour; // Start fading off
  int morningStopMin;
  int morningStopSec;
  
  int nightStartHour; // Start fading on Night
  int nightStartMin;
  int nightStartSec;

  int nightStopHour; // Start fading off
  int nightStopMin;
  int nightStopSec;

  int fadeHour; // Time to fade the leds on and off
  int fadeMin;
  int fadeSec;

  bool processRunning; // Stores if the process is running

};

Settings data;

// Teensy time function
time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

// Standard arduino functions

// Setup function
void setup() {


  Serial.begin(115200);

  Serial.println("inicializacion");

  setSyncProvider(getTeensy3Time);
  setSyncInterval(60);
  setTime(getTeensy3Time());

  delay(100);

  if (timeStatus() != timeSet) {
    Serial.println("Unable to sync with the RTC");
  } else {
    Serial.println("RTC has set the system time");
  }

  setDay = day();
  setMonth = month();
  setYear = year();
  setHour = hour();
  setMin = minute();
  setSec = second();


  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);

  pinMode(PWM_LEDS, OUTPUT);
  pinMode(TEST_LED0, OUTPUT);
  pinMode(TEST_LED1, OUTPUT);
  pinMode(TEST_LED2, OUTPUT);

  digitalWrite(TEST_LED0, LOW);
  digitalWrite(TEST_LED1, LOW);
  digitalWrite(TEST_LED2, LOW);

  analogWriteResolution(12);  // analogWrite value 0 to 4095

  // Setup the Bouncers instances :

  up.attach(BUTTON_UP);
  down.attach(BUTTON_DOWN);
  left.attach(BUTTON_LEFT);
  right.attach(BUTTON_RIGHT);

  up.interval(5);
  down.interval(5);
  left.interval(5);
  right.interval(5);

  // Menu setup

  /*
    Menu Structure:

    - Ajuste horario
    -- Hora amanecer
    ---- Hora inicio
    ----- HH
    ----- MM
    ----- SS
    ----- Guardar
    ----- Volver
    ---- Hora fin
    ----- HH
    ----- MM
    ----- SS
    ----- Guardar
    ----- Volver
    -- Hora anochecer
    ---- Hora inicio
    ----- HH
    ----- MM
    ----- SS
    ----- Guardar
    ----- Volver
    ---- Hora fin
    ----- HH
    ----- MM
    ----- SS
    ----- Guardar
    ----- Volver
    -- Duracion amanecer/anochecer
    --- HH
    --- MM
    --- SS
    --- Guardar
    --- Volver
    - Fecha y hora
    -- Fecha
    --- DD
    --- MM
    --- YY
    --- Guardar
    --- Volver
    -- Hora
    --- HH
    --- MM
    --- SS
    --- Guardar
    --- Volver
    -- Volver
    - Ejecutar
  */


  mm1_m1_start.add_item(&mm1_m1_start_HH, &on_mm1_m1_start_HH);
  mm1_m1_start.add_item(&mm1_m1_start_MM, &on_mm1_m1_start_MM);
  mm1_m1_start.add_item(&mm1_m1_start_SS, &on_mm1_m1_start_SS);
  mm1_m1_start.add_item(&m_SAVE, &on_m_SAVE);
  mm1_m1_start.add_item(&m_BACK, &on_m_BACK);
  
  mm1_m1_stop.add_item(&mm1_m1_stop_HH, &on_mm1_m1_stop_HH);
  mm1_m1_stop.add_item(&mm1_m1_stop_MM, &on_mm1_m1_stop_MM);
  mm1_m1_stop.add_item(&mm1_m1_stop_SS, &on_mm1_m1_stop_SS);
  mm1_m1_stop.add_item(&m_SAVE, &on_m_SAVE);
  mm1_m1_stop.add_item(&m_BACK, &on_m_BACK);

  mm1_m1.add_menu(&mm1_m1_start);
  mm1_m1.add_menu(&mm1_m1_stop);
  
  mm1.add_menu(&mm1_m1);

  mm1_m2_start.add_item(&mm1_m2_start_HH, &on_mm1_m2_start_HH);
  mm1_m2_start.add_item(&mm1_m2_start_MM, &on_mm1_m2_start_MM);
  mm1_m2_start.add_item(&mm1_m2_start_SS, &on_mm1_m2_start_SS);
  mm1_m2_start.add_item(&m_SAVE, &on_m_SAVE);
  mm1_m2_start.add_item(&m_BACK, &on_m_BACK);
  
  mm1_m2_stop.add_item(&mm1_m2_stop_HH, &on_mm1_m2_stop_HH);
  mm1_m2_stop.add_item(&mm1_m2_stop_MM, &on_mm1_m2_stop_MM);
  mm1_m2_stop.add_item(&mm1_m2_stop_SS, &on_mm1_m2_stop_SS);
  mm1_m2_stop.add_item(&m_SAVE, &on_m_SAVE);
  mm1_m2_stop.add_item(&m_BACK, &on_m_BACK);

  mm1_m2.add_menu(&mm1_m2_start);
  mm1_m2.add_menu(&mm1_m2_stop);

  mm1.add_menu(&mm1_m2);

  mm1_m3.add_item(&mm1_m3_HH, &on_mm1_m3_HH);
  mm1_m3.add_item(&mm1_m3_MM, &on_mm1_m3_MM);
  mm1_m3.add_item(&mm1_m3_SS, &on_mm1_m3_SS);
  mm1_m3.add_item(&m_SAVE, &on_m_SAVE);
  mm1_m3.add_item(&m_BACK, &on_m_BACK);

  mm1.add_menu(&mm1_m3);
  
  rm.add_menu(&mm1);

  mm2_m1.add_item(&mm2_m1_DD, &on_mm2_m1_DD);
  mm2_m1.add_item(&mm2_m1_MM, &on_mm2_m1_MM);
  mm2_m1.add_item(&mm2_m1_YY, &on_mm2_m1_YY);
  mm2_m1.add_item(&m_SAVE, &on_m_SAVE);
  mm2_m1.add_item(&m_BACK, &on_m_BACK);

  mm2.add_menu(&mm2_m1);

  mm2_m2.add_item(&mm2_m2_HH, &on_mm2_m2_HH);
  mm2_m2.add_item(&mm2_m2_MM, &on_mm2_m2_MM);
  mm2_m2.add_item(&mm2_m2_SS, &on_mm2_m2_SS);
  mm2_m2.add_item(&m_SAVE, &on_m_SAVE);
  mm2_m2.add_item(&m_BACK, &on_m_BACK);

  mm2.add_menu(&mm2_m2);

  mm2.add_item(&m_BACK, &on_m_BACK);

  rm.add_menu(&mm2);

  rm.add_item(&m_RUN, &on_m_RUN);

  ms.set_root_menu(&rm);

  screen->begin( 20, 4 );
  screen->setBacklightPin(3, POSITIVE);
  screen->setBacklight(HIGH);
  screen->clear();
  screen->home();


  EEPROM.get(configAddr, configured);

  if ( configured != 255) {
    screen->setCursor(1, 0);
    screen->print("Device configured");

    digitalWrite(TEST_LED1, HIGH );
    delay(100);
    digitalWrite(TEST_LED1, LOW );
    delay(100);
    digitalWrite(TEST_LED1, HIGH );
    delay(100);
    digitalWrite(TEST_LED1, LOW );

    EEPROM.get(dataAddr, data);

    if ( data.processRunning == true ) {
      digitalWrite(TEST_LED1, HIGH );
    }
  }
  else {
    screen->home();
    screen->print("Device not configured");

    digitalWrite(TEST_LED0, HIGH );
    delay(100);
    digitalWrite(TEST_LED0, LOW );
    delay(100);
    digitalWrite(TEST_LED0, HIGH );
    delay(100);
    digitalWrite(TEST_LED0, LOW );

  }

  delay(1000);

  displayMenu();

}


// Main loop
void loop() {

  if (!saveTime) {

    setDay = day();
    setMonth = month();
    setYear = year();
    setHour = hour();
    setMin = minute();
    setSec = second();
  }

  handleKeys();

  unsigned long currentMillis = millis();

  if ((backlightCounter != 0) && (backlightOn == true)) {

    if (currentMillis - backlightPreviousMillis > backlightInterval) {
      backlightPreviousMillis = currentMillis;
      backlightCounter--;

      if (backlightCounter == 0) {
        backlightOn = false;
        screen->off();
      }
    }
  }

  if (currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;

    Serial.print("Hour: ");
    Serial.print(hour());
    Serial.print(":");
    Serial.print(minute());
    Serial.print(":");
    Serial.print(second());
    Serial.print(" ");
    Serial.print(day());
    Serial.print("/");
    Serial.print(month());
    Serial.print("/");
    Serial.println(year());

    if ( data.processRunning ) {

      int morningStartHours = data.morningStartHour;
      int morningStartMins = data.morningStartMin;
      int morningStartSecs = data.morningStartSec;

      int morningStopHours = data.morningStopHour;
      int morningStopMins = data.morningStopMin;
      int morningStopSecs = data.morningStopSec;

      int nightStartHours = data.nightStartHour;
      int nightStartMins = data.nightStartMin;
      int nightStartSecs = data.nightStartSec;

      int nightStopHours = data.nightStopHour;
      int nightStopMins = data.nightStopMin;
      int nightStopSecs = data.nightStopSec;

      if (morningStartMins > 59) {
        morningStartHours++;
      }
      if (morningStartHours > 23) {
        morningStartHours = 0;
      }

      if (morningStopMins > 59) {
        morningStopHours++;
      }
      if (morningStopHours > 23) {
        morningStopHours = 0;
      }
      
      if (nightStartMins > 59) {
        nightStartHours++;
      }
      if (nightStartHours > 23) {
        nightStartHours = 0;
      }

      if (nightStopMins > 59) {
        nightStopHours++;
      }
      if (nightStopHours > 23) {
        nightStopHours = 0;
      }

      if ((hour() == morningStartHours) and (minute() == morningStartMins) and (second() == morningStartSecs)) {
        Serial.println("Start Fading MORNING");

        hoursInSecs = data.fadeHour * 3600;
        minutesInSecs = data.fadeMin * 60;
        maxSecs = hoursInSecs + minutesInSecs;

        fade = 0;
        fadeInc = (float)pwmResolution / (float)maxSecs;
        startFadingInMorning = 1;

        Serial.print("FADE IN TIME :");
        Serial.print(maxSecs);
        Serial.print(" steps of ");
        Serial.println(fadeInc);
      }

      if (startFadingInMorning) {
        if (secs < maxSecs) {
          Serial.print("Fading in morning : ");
          Serial.println(fade);
          analogWrite(PWM_LEDS, fade);
          fade += fadeInc;
          secs += 0.1;
        }
        else {
          Serial.println("Fading in morning : 4095");
          secs = 0;
          startFadingInMorning = 0;
          startFadingOutMorning = 0;
          analogWrite(PWM_LEDS, 4095);
        }
      }

      if ((hour() == morningStopHours) and (minute() == morningStopMins) and (second() == morningStopSecs)) {
        Serial.println("Stop Fading MORNING");

        hoursInSecs = data.fadeHour * 3600;
        minutesInSecs = data.fadeMin * 60;
        maxSecs = hoursInSecs + minutesInSecs;

        fade = pwmResolution;
        fadeDec = (float)pwmResolution / (float)maxSecs;
        startFadingOutMorning = 1;

        Serial.print("FADE OUT TIME :");
        Serial.print(maxSecs);
        Serial.print(" steps of ");
        Serial.println(fadeDec);
      }

      if (startFadingOutMorning) {
        if (secs < maxSecs) {
          Serial.print("Fading out morning : ");
          Serial.println(fade);
          analogWrite(PWM_LEDS, fade);
          fade -= fadeDec;
          secs += 0.1;
        }
        else {
          Serial.println("Fading out morning : 0");
          secs = 0;
          startFadingOutMorning = 0;
          startFadingInMorning = 0;
          analogWrite(PWM_LEDS, 0);
        }
      }
      if ((hour() == nightStartHours) and (minute() == nightStartMins) and (second() == nightStartSecs)) {
        Serial.println("Start Fading NIGHT");

        hoursInSecs = data.fadeHour * 3600;
        minutesInSecs = data.fadeMin * 60;
        maxSecs = hoursInSecs + minutesInSecs;

        fade = 0;
        fadeInc = (float)pwmResolution / (float)maxSecs;
        startFadingInNight = 1;

        Serial.print("FADE IN TIME :");
        Serial.print(maxSecs);
        Serial.print(" steps of ");
        Serial.println(fadeInc);
      }

      if (startFadingInNight) {
        if (secs < maxSecs) {
          Serial.print("Fading in night : ");
          Serial.println(fade);
          analogWrite(PWM_LEDS, fade);
          fade += fadeInc;
          secs += 0.1;;
        }
        else {
          Serial.println("Fading in night: 4095");
          secs = 0;
          startFadingInNight = 0;
          startFadingOutNight = 0;
          analogWrite(PWM_LEDS, 4095);
        }
      }

      if ((hour() == nightStopHours) and (minute() == nightStopMins) and (second() == nightStopSecs)) {
        Serial.println("Stop Fading NIGHT");

        hoursInSecs = data.fadeHour * 3600;
        minutesInSecs = data.fadeMin * 60;
        maxSecs = hoursInSecs + minutesInSecs;

        fade = pwmResolution;
        fadeDec = (float)pwmResolution / (float)maxSecs;
        startFadingOutNight = 1;

        Serial.print("FADE OUT TIME :");
        Serial.print(maxSecs);
        Serial.print(" steps of ");
        Serial.println(fadeDec);
      }

      if (startFadingOutNight) {
        if (secs < maxSecs) {
          Serial.print("Fading out night : ");
          Serial.println(fade);
          analogWrite(PWM_LEDS, fade);
          fade -= fadeDec;
          secs += 0.1;;
        }
        else {
          Serial.println("Fading out night : 0");
          secs = 0;
          startFadingOutNight = 0;
          startFadingInNight = 0;
          analogWrite(PWM_LEDS, 0);
        }
      }
    }
  }

  if (drawSetup) {
    drawConfig();
  }
}

// Read Serial port
void serialEvent() {
  char keyPressed;
  if ((keyPressed = Serial.read()) > 0) {
    handleMenu(keyPressed);
  }
}
