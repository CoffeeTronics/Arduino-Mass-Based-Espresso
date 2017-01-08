#include <Arduino.h>

#include <HX711.h>
#include <Wire.h>
#include <I2C_Anything.h>

#define BREW_SWITCH 10
#define PUMP_CONTROL 9

const byte SLAVE_TWO_ADDRESS =  43;   // slave address of mass sender
volatile double mass;               // 4 bytes
volatile int brewSwitchState;       // 2 bytes
volatile boolean extractionComplete;// 1 byte

int numPulses = 2;
int pulseDuration = 1000;
int pulseGap = 2000;
int maxShotTime = 10000;

int test = 0;
unsigned long timeNow = 0;
unsigned long timeLast = 0;
unsigned long readingTimeNow = 0;
unsigned long readingTimeLast = 0;


HX711 scale;
double reading;


void setup() {
  Serial.begin(115200);
  Wire.begin(SLAVE_TWO_ADDRESS);
  Wire.onRequest(requestEvent);

  pinMode(BREW_SWITCH, INPUT);
  pinMode(PUMP_CONTROL, OUTPUT);

  scale.begin(A0, A1);
  scale.set_scale();
  scale.tare();

  double getUnits = scale.get_units(10);
  //double parameter = getUnits / 56.70;
  double parameter = 1283.95;
  scale.set_scale(parameter);
  scale.tare();
  reading = 0.00;

}


void loop() {
  brewSwitchState = digitalRead(BREW_SWITCH);
  //Serial.print("Switch state : ");
  //Serial.println(brewSwitchState);

  if (!brewSwitchState) {           // if brew switch ON
    scale.tare();   // tare scale ready for extraction

    for (int i = 0; i < numPulses; i++) {
      extractionComplete = false;

      Serial.println("Pulsing pump for 1 sec");
      digitalWrite(PUMP_CONTROL, HIGH);           // preinfusion
      delay(pulseDuration);                       // routine
      Serial.println("Pump off for 2 sec");
      digitalWrite(PUMP_CONTROL, LOW);
      delay(pulseGap);

    }
    timeLast = millis();
    timeNow = timeLast;
    readingTimeNow = timeLast;

    while ((timeNow - timeLast) <= 3000) {  // run pump for 10s or until
      digitalWrite(PUMP_CONTROL, HIGH);     // user turns off brew switch
      extractionComplete = false;
      //brewSwitchState = digitalRead(BREW_SWITCH);
      brewSwitchState = 0;
      for (int i = 0; i < 10; i++) {
        brewSwitchState += digitalRead(BREW_SWITCH);
        delay(10);
      }
      Serial.print("10 state: ");
      Serial.println(brewSwitchState);
      if (brewSwitchState < 5){
        brewSwitchState = 1;
      }
      else {
        brewSwitchState = 0;
      }
      //brewSwitchState = test;
      
      if (!brewSwitchState) {
        Serial.println(brewSwitchState);
        Serial.println("BREW SWITCH OFF!!!!!");
        digitalWrite(PUMP_CONTROL, LOW);  // turn pump off and
        extractionComplete = true;        // tell master we are finished
        Serial.println("Extraction Complete");
        delay(1000);
        break;                          // break out of while loop
      }
      
      readingTimeNow = millis();

      if ((readingTimeNow - readingTimeLast) >= 1000) { //read mass every second
        readingTimeNow = millis();
        reading = scale.get_units();
        mass = reading;       // read into volatile for sending on I2C
        Serial.print("Mass : ");
        Serial.println(reading);
        readingTimeLast = readingTimeNow;
      }
      //Serial.println("Pump running");
      timeNow = millis();

    }
    //digitalWrite(PUMP_CONTROL, LOW);   // turn off Pump
  }

  else {
    digitalWrite(PUMP_CONTROL, LOW);
    brewSwitchState = 0;
    mass = 0.0;
    //Serial.println("Pump off");
  }
}

void requestEvent() {
  I2C_writeAnything(brewSwitchState);
  I2C_writeAnything(mass);
  I2C_writeAnything(extractionComplete);
}
