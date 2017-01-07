/* digitalSmooth
  Paul Badger 2007
  A digital smoothing filter for smoothing sensor jitter
  This filter accepts one new piece of data each time through a loop, which the
  filter inputs into a rolling array, replacing the oldest data with the latest reading.
  The array is then transferred to another array, and that array is sorted from low to high.
  Then the highest and lowest %15 of samples are thrown out. The remaining data is averaged
  and the result is returned.

  Every sensor used with the digitalSmooth function needs to have its own array to hold
  the raw sensor values. This array is then passed to the function, for it's use.
  This is done with the name of the array associated with the particular sensor.
*/

#include <Thermistor4.h>
#include <PID_v1.h>
#include <Wire.h>
#include <I2C_Anything.h>


const byte SLAVE_ONE_ADDRESS =  42;   // slave address of temp sender
volatile double temp;
volatile double output;

//Inside thermistor on Arduino ADC pin 0
#define THERMISTORPin 0
#define RELAY_PIN 7
#define filterSamples   51              // filterSamples should  be an odd number, no smaller than 3
double sensSmoothArray1 [filterSamples];   // array for holding raw sensor values for sensor1

double rawData1, smoothData1;  // variables for sensor1 data

//Define Variables we'll be connecting to
double Setpoint, Input, Output;
//Specify the links and initial tuning parameters
double Kp = 100, Ki = 0.15, Kd = 1;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

int WindowSize = 5000;
unsigned long windowStartTime;

// instance of Thermistor Object
Thermistor4 Thermistor;
//various temp variables for testing.
unsigned int i, ADCAverage;

/////////////////////// used for measuring Vcc
long sum;
double average;
///////////////////

unsigned long timeNow,timeLast;

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
  Wire.begin(SLAVE_ONE_ADDRESS);
  Wire.onRequest(requestEvent);
  
  for (int i = 0; i < 4; i++)
  {
    pinMode(digitPins[i], OUTPUT);
  }
  //pinMode(tempPin, INPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  //////////////////////////////////////////////////
  average = 0.00;
  sum = 0;
  for (int i = 0; i <= 99; i++) {
    if (i == 0) {
      sum = 0;
    } else
      sum += readVcc();
  }
  average = double(sum / 99.0);

  printDisp(average, 2000);
  //Serial.println(average);    //( readVcc(), DEC );

  ///////////////////////////////////////



  Thermistor.Pin = THERMISTORPin; //Set the pin number.

  Thermistor.SetUp(); //Sets up the analog read pin for internal AVR.

  //pow() is used elsewhere so might as well be used here.
  Thermistor.BitResolution = pow(2, 10) - 1; //ATmega's have a 10bit ADC (2^10-1).

  Thermistor.VoltageSupply = average;   //4.885;   //4.4481;  // Metered supply across voltage divider
  Thermistor.ResistanceFixed = 8250;   ///Fixed resistor in the divider. Measured in ohms. Meter this for accuracy.

  Thermistor.Offset = 0.0; //adjust temperature in Kelvin up or down a little to account for unforseen variations.

  // Steinhart-Hart coefficients. Taken from datasheet provided from manufacturer
  Thermistor.SteinhartA1 = 5.99357907117746e-004;   // First Steinhart-Hart coefficient.
  Thermistor.SteinhartA2 = 2.31247850239102e-004;   // Second Steinhart-Hart coefficient.
  Thermistor.SteinhartA3 = 5.61924102167737e-008;   // Third Steinhart-Hart coefficient.
  Thermistor.SteinhartA4 = 3.23406799250025e-011;   // Fourth Steinhart-Hart coefficient.

  windowStartTime = millis();

  //initialize the variables we're linked to
  Setpoint = 150.0;

  //tell the PID to range between 0 and the full window size
  myPID.SetOutputLimits(0, WindowSize);

  //turn the PID on
  myPID.SetMode(AUTOMATIC);
}



void loop() {      // test the digitalSmooth function
  Thermistor.ReadCalculate(3);
  
  Thermistor.ReadADC(analogRead (Thermistor.Pin));
  rawData1 = Thermistor.GetFarenheit();

  

  //rawData1 = analogRead(THERMISTORPin);                        // read sensor 1
  //smoothData1 = digitalSmooth(rawData1, sensSmoothArray1);  // every sensor you use with digitalSmooth needs its own array
  Input = digitalSmooth(rawData1, sensSmoothArray1);  // every sensor you use with digitalSmooth needs its own array
  
  temp = Input;     // read PID Input & Output
  output = Output;  // into volatiles used for I2C comms

  
  
  myPID.Compute();
  /************************************************
     turn the output pin on/off based on pid output
   ************************************************/
  if (millis() - windowStartTime > WindowSize)
  { //time to shift the Relay Window
    windowStartTime += WindowSize;
  }
  if (Output < millis() - windowStartTime) digitalWrite(RELAY_PIN, HIGH);
  else digitalWrite(RELAY_PIN, LOW);

  timeNow = millis();
  if ((timeNow - timeLast) >= 1000){
    Serial.print(Input);
    Serial.print("\t");
    Serial.println(Output);
    timeLast = timeNow;
  }
  
  
  //Serial.print(rawData1);
  //Serial.print("   ");
  //Serial.println(Input);
  
  printDisp(Input, 1000);
  //  rawData2 = analogRead(SensorPin2);                        // read sensor 2
  //  smoothData2 = digitalSmooth(rawData2, sensSmoothArray2);  // every sensor you use with digitalSmooth needs its own array

}

double digitalSmooth(double rawIn, double *sensSmoothArray) {    // "int *sensSmoothArray" passes an array to the function - the asterisk indicates the array name is a pointer
  int j, k;//, temp, top, bottom;
  double temp, top, bottom;
  //long total;
  double total;
  static int i;
  // static int raw[filterSamples];
  static double sorted[filterSamples];
  boolean done;

  i = (i + 1) % filterSamples;    // increment counter and roll over if necc. -  % (modulo operator) rolls over variable
  sensSmoothArray[i] = rawIn;                 // input new data into the oldest slot

  // Serial.print("raw = ");

  for (j = 0; j < filterSamples; j++) { // transfer data array into anther array for sorting and averaging
    sorted[j] = sensSmoothArray[j];
  }

  done = 0;                // flag to know when we're done sorting
  while (done != 1) {      // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (filterSamples - 1); j++) {
      if (sorted[j] > sorted[j + 1]) {    // numbers are out of order - swap
        temp = sorted[j + 1];
        sorted [j + 1] =  sorted[j] ;
        sorted [j] = temp;
        done = 0;
      }
    }
  }

  /*
    for (j = 0; j < (filterSamples); j++){    // print the array to debug
      Serial.print(sorted[j]);
      Serial.print("   ");
    }
    Serial.println();
  */

  // throw out top and bottom 15% of samples - limit to throw out at least one from top and bottom
  bottom = max(((filterSamples * 15)  / 100), 1);
  top = min((((filterSamples * 85) / 100) + 1  ), (filterSamples - 1));   // the + 1 is to make up for asymmetry caused by integer rounding
  k = 0;
  total = 0.00;
  for ( j = bottom; j < top; j++) {
    total += sorted[j];  // total remaining indices
    k++;
    // Serial.print(sorted[j]);
    // Serial.print("   ");
  }

  //  Serial.println();
  //  Serial.print("average = ");
  //  Serial.println(total/k);
  return total / k * 1.00;    // divide by number of samples
}


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
  if (tf - ti < msec) {
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


long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL;
  result |= ADCH << 8;
  result = 1125300L / result; // Back-calculate AVcc in mV
  return result;
}


void requestEvent() {
  I2C_writeAnything(temp);
  I2C_writeAnything(output);
}


