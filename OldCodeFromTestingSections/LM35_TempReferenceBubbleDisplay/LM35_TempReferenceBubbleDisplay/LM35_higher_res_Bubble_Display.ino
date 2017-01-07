#include <Arduino.h>

/*
   This code is made from two sources
   http://randomnerdtutorials.com
   http://playground.arduino.cc/ComponentLib/Thermistor4
*/
//#include <Thermistor4.h>

//Inside thermistor on Arduino ADC pin 0
//#define THERMISTORPinInside 0

float tempC;
//int reading;
int tempPin = A1;

unsigned long reading;
unsigned long sum;


// instance of Thermistor Object
//Thermistor4 ThermistorInside;
//various temp variables for testing.
//unsigned int i, ADCAverage;
//double tempC;
//double tempF;

const int digitPins[4] = {9, 10, 11, 12}; //4 common CATHODE pins of the display.
const int clockPin = 2;    //74HC595 Pin 11
const int latchPin = 3;    //74HC595 Pin 12
const int dataPin = 4;     //74HC595 Pin 14

//As seen on SparkFun's SevSeg Library
const byte digit[10] =      //seven segment digits
{
  0b11111100, // 0
  0b01100000, // 1
  0b11011010, // 2
  0b11110010, // 3
  0b01100110, // 4
  0b10110110, // 5
  0b10111110, // 6
  0b11100000, // 7
  0b11111110, // 8
  0b11110110  // 9
};

int digitBuffer[4] = {0};
int digitScan = 0;
double counter;

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 4; i++)
  {
    pinMode(digitPins[i], OUTPUT);
  }
  //pinMode(tempPin, INPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  analogReference(INTERNAL);
  //Serial.begin(115200);

  //  ThermistorInside.Pin = THERMISTORPinInside; //Set the pin number.

  //  ThermistorInside.SetUp(); //Sets up the analog read pin for internal AVR.

  //pow() is used elsewhere so might as well be used here.
  //  ThermistorInside.BitResolution = pow(2, 10) - 1; //ATmega's have a 10bit ADC (2^10-1).

  //  ThermistorInside.VoltageSupply = 4.944;   //4.4481;  // Metered supply across voltage divider
  //  ThermistorInside.ResistanceFixed = 8250;   ///Fixed resistor in the divider. Measured in ohms. Meter this for accuracy.

  //  ThermistorInside.Offset = 1.1; //adjust temperature in Kelvin up or down a little to account for unforseen variations.

  // Steinhart-Hart coefficients. Taken from datasheet provided from manufacturer
  //  ThermistorInside.SteinhartA1 = 5.99357907117746e-004;   // First Steinhart-Hart coefficient.
  //  ThermistorInside.SteinhartA2 = 2.31247850239102e-004;   // Second Steinhart-Hart coefficient.
  //  ThermistorInside.SteinhartA3 = 5.61924102167737e-008;   // Third Steinhart-Hart coefficient.
  //  ThermistorInside.SteinhartA4 = 3.23406799250025e-011;   // Fourth Steinhart-Hart coefficient.


}   // end setup





void loop() {
  /******************************
     Used for debugging purposes.
     Shows resolution at bottom, middle and top of ADC range
    #if THERMISTORDEBUG
    //Show the debugging information:
    Serial.println("");
    Serial.println("ThermistorInside Debug: ");
    ThermistorInside.Thermistor4SerialPrint();
    Serial.println("");
    //Serial.println("ThermistorOutside Debug: ");
    //ThermistorOutside.Thermistor4SerialPrint();
    //Serial.println("");
    #endif
  */

  //  ThermistorInside.ReadCalculate(3);

  //Example of averaging 10 reads. Note that analogRead() isn't very fast.
  //Part of this code has to be done manually, then call ReadADC() with the averaged number.
  //Note that an unsigned int is only 16 bits. Do not exceed it or it will roll over and return trash.

  //  ADCAverage = 0; //reset the variable for each loop.
  //  for (i = 0; i < 20; i++) {
  //    ADCAverage += analogRead(THERMISTORPinInside);
  //    delayMicroseconds(1); //add an extra delay to spread the average out a little more
  //  }
  //  ThermistorInside.ReadADC(ADCAverage / 20); //call ReadADC() with the averaged value.

  //Report as normal from here on out.
  //Serial.print("ThermistorInside Averaged (Centigrade, Farenheit): ");
  //Serial.print(ThermistorInside.GetCentigrade(), 4);
  //Serial.print(", ");
  //Serial.println(ThermistorInside.GetFarenheit(), 4);

  //printDisp(ThermistorInside.GetFarenheit(), 2000);
  //printDisp(ThermistorInside.GetCentigrade(), 2000);

  sum = 0; reading = 0;

  for (int i=0; i<99; i++){
    reading = analogRead(tempPin);
    sum += reading;
  }

tempC = (sum/100) / 9.31;

  //reading = analogRead(tempPin);
  //tempC = reading / 9.31;
  Serial.println(tempC);
  //delay(1000);
  printDisp(tempC, 2000);


}       // end loop


void printDisp(float value, int msec) {
  //Serial.println("Gets to printDisplay()");
  clearDisp();
  //int digitThree, digitTwo, digitOne, digitZero;
  digitBuffer[3] = int(value / 100); // return hundreds value
  digitBuffer[2] = int(((value - (digitBuffer[3] * 100))) / 10);  // return tens value
  digitBuffer[1] = int ((value - (digitBuffer[3] * 100) - (digitBuffer[2] * 10))); // return units value
  digitBuffer[0] = int ((value - (digitBuffer[3] * 100) - (digitBuffer[2] * 10) - (digitBuffer[1])) * 10);  // return first decimal place value
  /*
    for(int i=3; i>=0; i--){
      Serial.print(digitBuffer[i]);

    }
    Serial.println();
  */


  //Get it displayed until msec Milliseconds passed
  long ti = millis();
  long tf = millis();
  while (tf - ti < msec) {
    tf = millis();
    updateDisp();
  }
}

//writes the temperature on display
void updateDisp() {
  for (int i = 0; i < 4; i++) {
    clearDisp();
    digitalWrite(digitPins[i], LOW); //Changed to LOW for turning the leds on.

    if (i == 1) //Add decimal dot
      shiftOut(dataPin, clockPin, LSBFIRST, digit[digitBuffer[i]] | 0b00000001);
    else
      shiftOut(dataPin, clockPin, LSBFIRST, digit[digitBuffer[i]]);

    digitalWrite(latchPin, HIGH);
    digitalWrite(latchPin, LOW);

    delay(7); //If not delayed, digits are seen brurry, if the value is 8 you migth see the display frickering.
  }
}

void clearDisp() {
  for (byte j = 0; j < 4; j++) {
    digitalWrite(digitPins[j], HIGH); // Turns the display off. Changed to HIGH
  }
}
