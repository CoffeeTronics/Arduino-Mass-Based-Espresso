// Thermistor4.cpp member functions of class. For Arduino 0018 or newer.
// Thermistor class to consolidate various variables and functions.
// (c)20101017 by MODAT7 under GNU GPLv1

#include "Thermistor4.h"

//The classes would normally use read() and set() commands and "private" the data,
//but space is at a premium here in an MCU. This has been written for flexibility.
//"this->" allows for better clarity and less confusion if global variables have
//the same name as class variables.

//constructor. This could be more but I want to code for size small.
/* Space is at a premium and this isn't necessary since it is taken care of in setup().
Thermistor4::Thermistor4() {
  this->Pin=0;
  this->BitResolution=10;
  this->SteinhartA1=0.0;
  this->SteinhartA2=0.0;
  this->SteinhartA3=0.0;
  this->SteinhartA4=0.0;
  this->Offset=0.0;
}
*/

//Set up and make sure the Arduino analog pin is in the proper mode.
//Use your own custom SetUp() for an external ADC.
void Thermistor4::SetUp() {
//Take the pin out of pull-up mode.
//Arduino analog pins are 14-19 when accessed in digital mode.
pinMode((14 + this->Pin), INPUT);
digitalWrite((14 + this->Pin), LOW);
//The first analog read of a pin may be bad, so read and discard it.
analogRead(this->Pin);
}

//Read the raw value of the ADC and calculate stuff needed for Steinhart-Hart equation.
//ExternalReading is 0 for the Arduino's internal ADC's. Reading 0 on an ADC
//thermistor is a very unlikely condition.
//For an external ADC: ExternalReading is whatever the external ADC's return number is.
//You will have to write your own code to get this and call ReadADC() and
//CalculateTemperature() afterwards instead of the usual ReadCalculate().
void Thermistor4::ReadADC(unsigned int ExternalReading) {
if(ExternalReading == 0) {
  this->ADCReading = analogRead(this->Pin); //takes about 100uS to read an analog Arduino pin.
  }
else {this->ADCReading = ExternalReading;} //set to the provided external ADC value.
//Calculate the current voltage at Pin.
//Voltage Divider Equation: VMidPoint = (RBottom / (RTop+RBottom)) * VSupply
//...solving for: RTop = ((VSupply*Rbottom)/VMidPoint) - RBottom
this->VoltageReading = ((double)this->ADCReading/(double)this->BitResolution) * this->VoltageSupply;
this->ResistanceThermistor = ((this->VoltageSupply*(double)this->ResistanceFixed) / this->VoltageReading) - this->ResistanceFixed;
} //end ReadADC()


/*
//Calculate the ADC reading into temperature in Kelvin. (from http://thermistor.sf.net method)
//I don't know why these look so different than the usual Steinhart-Hart equations.
//These look backwards?
//This code is left here for legacy reasons and as an alternative example.
//It is actually about 5% faster if always doing the full equations.
void Thermistor4::CalculateTemperature() {
double LnResist;
LnResist = log(this->ResistanceThermistor); // no reason to calculate this multiple times.
//Build up the number in the stages.
this->Temperature = this->SteinhartA4;
this->Temperature = this->Temperature * LnResist + this->SteinhartA3;
this->Temperature = this->Temperature * LnResist + this->SteinhartA2;
this->Temperature = this->Temperature * LnResist + this->SteinhartA1;
//Final part is to invert
this->Temperature = (1.0 / this->Temperature) + this->Offset;
} //end CalculateTemperature()
*/


//Calculate the ADC reading into temperature in Kelvin. (official Steinhart-Hart equations)
//This is split up (Selection) to allow for comparisons and easy selection of the
//Steinhart-Hart complexity levels.
//Steinhart-Hart Thermistor Equations (in Kelvin) Selections:
//1: Simplified: Temp = 1 / ( A + B(ln(R)) )
//2: Standard:   Temp = 1 / ( A + B(ln(R)) + D(ln(R)^3) )
//3: Extended:   Temp = 1 / ( A + B(ln(R)) + C(ln(R)^2) + D(ln(R)^3) )
//Obviously lower numbers are less accurate but are much faster.
void Thermistor4::CalculateTemperature(unsigned char Selection) {
double LnResist;
LnResist = log(this->ResistanceThermistor); // no reason to calculate this multiple times.
//build up the Steinhart-Hart equation depending on Selection.
//Level 1 is used by all.
this->Temperature = this->SteinhartA1 + (this->SteinhartA2 * LnResist);
//If 2, add in level 2.
if(Selection>=2) { this->Temperature = this->Temperature + (this->SteinhartA3 * LnResist * LnResist * LnResist); }
//If 3, add in level 3.
if(Selection>=3) { this->Temperature = this->Temperature + (this->SteinhartA4 * LnResist * LnResist); }
//Final part is to invert
this->Temperature = (1.0 / this->Temperature) + this->Offset;
} //end CalculateTemperature()


//Read and calculate temperature. This combines ReadADC() and CalculateTemperature() for
//convenience. This one is the one that will generally be used. Do not use this if using an
//external ADC.
void Thermistor4::ReadCalculate(unsigned char Selection) {
this->ReadADC(0); // 0 means use internal ADC.
this->CalculateTemperature(Selection);
} //end ReadCalculate()


/* Note:
Centigrade and Farenheit aren't stored internally for size reasons. In most cases these functions
won't be called more than once per reading, so this is a non-issue. If they would be called more
than once, create an external double variable and assign the value to it.
*/


//Return the temperature in Centigrade.
//Since many will want one system or the other, this is split apart.
double Thermistor4::GetCentigrade() {
return this->Temperature - 273.15;  // from Kelvin to Centigrade
} //end GetCentigrade()


//Return the temperature in Farenheit.
//Since many will want one system or the other, this is split apart.
double Thermistor4::GetFarenheit() {
return (this->Temperature - 273.15) * (9.0 / 5.0) + 32.0; // from Kelvin to Centigrade to Farenheit
} //end GetFarenheit()


#if THERMISTORDEBUG
//Print out the vaule of all the current variables. Mainly for debugging.
//If needed, this function can be disabled to save space after calibration and setup.
//Extra code adds about 2.5k to the binary.
void Thermistor4::Thermistor4SerialPrint() {
  double mytempc, mytempf;
  //print out this class's variables.
  this->ReadCalculate(3);
  //Serial.println("Thermistor4 Values:");
  //Serial.print("Pin: "); Serial.println(this->Pin, DEC);
  //Serial.print("BitResolution: "); Serial.println(this->BitResolution, DEC);
  //Serial.print("ADCReading: "); Serial.println(this->ADCReading, DEC);
  //Serial.print("VoltageSupply: "); Serial.println(this->VoltageSupply, DEC);
  //Serial.print("VoltageReading: "); Serial.println(this->VoltageReading, DEC  );
  //Serial.print("ResistanceFixed: "); Serial.println(this->ResistanceFixed, DEC);
  //Serial.print("ResistanceThermistor: "); Serial.println(this->ResistanceThermistor, DEC);
  //it would be nice if Serial.print() would do doubles as powers...
  //Serial.print("SteinhartA1*1,000,000: "); Serial.println((this->SteinhartA1*1000000), DEC);
  //Serial.print("SteinhartA2*1,000,000: "); Serial.println((this->SteinhartA2*1000000), DEC);
  //Serial.print("SteinhartA3*1,000,000: "); Serial.println((this->SteinhartA3*1000000), DEC);
  //Serial.print("SteinhartA4*1,000,000: "); Serial.println((this->SteinhartA4*1000000), DEC);
  //Serial.print("Temperature Offset: "); Serial.println(this->Offset, DEC);
  Serial.print("Temperature Kelvin: "); Serial.println(this->Temperature, DEC);
  Serial.print("Temperature Centigrade: "); Serial.println(this->GetCentigrade(), DEC);
  Serial.print("Temperature Farenheit: "); Serial.println(this->GetFarenheit(), DEC);
  //Serial.print("sizeof(Thermistor4) object: "); Serial.println(sizeof(Thermistor4),DEC);
  //Show differences between ReadCalculate levels.
  //If the numbers are very different, the coefficients aren't quite right. This seems common
  //for self generated numbers from http://thermistor.sf.net. Use the full version (3) instead
  //of the shorter ones.
  //Serial.println("If the first 2 temps are nonsense, then the Steinhart-Hart coefficients aren't quite right...");
  //Serial.println("You will have to use ReadCalculate(3) for all the temperature readings.");
  //this->ReadCalculate(1);
  //Serial.print("Inside Farenheit accuracy, ReadCalculate 1, 2, 3: ");
  //Serial.print(this->GetFarenheit(), DEC);
  //this->CalculateTemperature(2);
  //Serial.print(", ");
  //Serial.print(this->GetFarenheit(), DEC);
  this->CalculateTemperature(3);
  //Serial.print(", ");
  Serial.print("Temperature Centigrade: "); Serial.println(this->GetCentigrade(), DEC);
  Serial.print("Temperature Farenheit: "); Serial.println(this->GetFarenheit(), DEC);

//  Serial.println(this->GetFarenheit(), DEC);
  //Check ADC resolution/calculation throughout the range.
  //Some will be linear (voltage), others will not (resistance).
  //This is important to know for tight tolerance values.
  //Check the resolution at the bottom of the range.
  this->ReadADC(this->BitResolution/10);
  this->CalculateTemperature(3);
  mytempc=this->Temperature;
  mytempf=this->GetFarenheit();
  this->ReadADC((this->BitResolution/10)+1);
  this->CalculateTemperature(3);
  Serial.print("Temperature Resolution ADC Bottom (K/C, F): ");
  Serial.print((this->Temperature-mytempc), DEC);
  Serial.print(", ");
  Serial.println((this->GetFarenheit()-mytempf), DEC);
  //Check the resolution at the middle of the range.
  this->ReadADC(this->BitResolution/2);
  this->CalculateTemperature(3);
  mytempc=this->Temperature;
  mytempf=this->GetFarenheit();
  this->ReadADC((this->BitResolution/2)+1);
  this->CalculateTemperature(3);
  Serial.print("Temperature Resolution ADC Middle (K/C, F): ");
  Serial.print((this->Temperature-mytempc), DEC);
  Serial.print(", ");
  Serial.println((this->GetFarenheit()-mytempf), DEC);
  //Check the resolution at the top of the range.
  this->ReadADC(this->BitResolution*9/10);
  this->CalculateTemperature(3);
  mytempc=this->Temperature;
  mytempf=this->GetFarenheit();
  this->ReadADC((this->BitResolution*9/10)+1);
  this->CalculateTemperature(3);
  Serial.print("Temperature Resolution ADC Top (K/C, F): ");
  Serial.print((this->Temperature-mytempc), DEC);
  Serial.print(", ");
  Serial.println((this->GetFarenheit()-mytempf), DEC);
} //end Thermistor4SerialPrint()
#endif

//end of Thermistor4() class.