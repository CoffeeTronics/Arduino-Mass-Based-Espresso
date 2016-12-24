// Thermistor4.h definition of class. For Arduino 0018 or newer.
// Thermistor class to consolidate various variables and functions.
// (c)20101017 by MODAT7 under GNU GPLv1

/*
Purpose: To have higher accuracy readings from various sized negative temperature coefficient
(NTC) thermistors of varying resistances using the Steinhart-Hart equations. If all that is
needed is relative temperature changes from a fixed point, a simpler model is recommended rather
than using this large class. On the flip side, heavier code is often better than having a number
of large look up tables for a number of different thermistors.

In plain English: It's nice to re-use thermistors from dead indoor/outdoor thermometers in
projects. It's also nice to recalibrate thermistors that are "supposed" to be calibrated.
At this date, I haven't seen a consolidated project/class library that does all this. Some will
say there are ways to make this code a bit cleaner and more efficient. It's free so, have at it.
The purpose of this coding style is to be clear and easy to read where many have been so murky.

To make this library universally permanent from the "Sketch / Import Library" menu, go into the
Arduino install directory, "libraries" subdirectory, and create a "Thermistor4" directory. Copy
in Thermistor4.h and Thermistor4.cpp. Restart the Arduino IDE if it is running.

To use this library in a single project, exit the Arduino IDE if it is running and copy in
Thermistor4.h and Thermistor4.cpp into the project directory. It will show up in the extra tabs
when the IDE is relaunched. Add a #include "Thermistor4.h" into the sketch.

A C++ class makes multiple thermistors easy to handle. In the current form, it is also easy to
change certain variables on the fly to keep accuracy (such as if VoltageSupply is a battery that
lowers over time). When setting up and calibrating, a serial routine could be written to tweak
on the fly.

Provisions have been made for external ADC's (serial or parallel, whatever Arduino can interface
with), but you need to program how the raw data gets to this class object yourself. See the
ReadADC() comments for instructions.

Potential usages. I use mine for non-critical medical and high power component monitoring (power
transistors and MOSFET's). Others include indoor/outdoor thermometer weather stations, indoor AC
zone vent control and monitoring (if using home automation), AC thermostats (like for
uncontrolled window units), controlling attic ventilation fans (home automation with relays),
garden greenhouse monitoring, refridgerator and freezer monitoring, cooking (*IF* using a food
grade probe that is rated for those temperatures), computer hardware temperature monitoring (case
zones and chips, maybe add fan controls), and probably many more...

NOTE: Read this and the comments carefully for usage. It isn't very hard but there is A LOT OF
TEDIUM involved in getting this to work correctly.

NOTE: To get the best calibration out of thermistor.sf.net, take at least several evenly spaced
readings and go 5-10% beyond the lowest and highest desired measuring points. I personally use
the freezer (for below freezing), ice water (for freezing), multiple indoor and outdoor temps
(against a glass vial type thermometer), and boiling water.

NOTE: The thermistor.sf.net calculations do NOT seem to produce 100% correct coefficients for me.
The full calculation (CalculateTemperature(3)) seems to work very well, but any of the shorter
equations produce garbage. Check these carefully in your project before usage.

As just mentioned ReadCalculate() and CalculateTemperature() have 3 different levels for the 3
different complexities of the equation. 1 is the fastest/least accurate, and 3 is the
slowest/most accurate. Using another example, I figure the fastest/1 is about 2-3 degrees Kelvin
off from the most accurate.

Choosing "ResistanceFixed". The following equation can be helpful in getting the maximum voltage
swing out of a variable resistor for largest accuracy range.
Vmax = (VoltageSupply * ResistorFixed) / (Thermistor_Min + ResistorFixed)
"Vmax" is what the ADC pin would see when resistance is at "Thermistor_Min". This equation could
also be used to narrow down resolution to a particular range of interest.

Long wires on the thermistor could also contribute to a mild voltage loss. VoltageSupply may need
to be dropped a little.

If multiple thermistors are being used, pick one that is most likely to be the most accurate
and put all the other thermistors close to it. Let them all sit awhile and adjust to room
temperature. Once they stabilize, tweak the thermistor's "Offset" value until they all match the
most accurate one.

Some have mentioned that thermistors will self heat and skew the reading. This is true for lower
resistance thermistors and less of a problem for higher resistance ones (more resistance is less
current is less heat). The equations in this class do not account for self generated heat.
For any unused ADC input pins, it's best to set their pullup resistors so they don't mess with
the others (pinMode(myPin, INPUT); digitalWrite(myPin, HIGH)).

Sources: This came from all over but mainly:
http://thermistor.sourceforge.net
   (thanks to SoftQuadrat, good explanation, use the coeff/simu.txt calibration tools here)
http://en.wikipedia.org/wiki/Thermistor
http://en.wikipedia.org/wiki/Steinhart-Hart_equation
Arduino: ComponentLib/Thermistor2 and some forum discussions.

Observation. While the math for all this is well within an ATmega's capability, it is still
rather heavy for an MCU. If the Arduino is plugged into a PC and since this library is written
in C++, it wouldn't be hard to port the bulk of it over to the PC side. The Arduino hardware
would provide the raw ADC numbers to the PC, and the PC would do the calculations. The PC
version of the library would treat the Arduino hardware like an external ADC.

Physical Circuit:
VoltageSupply---Thermistor---\ADC Pin/---Fixed Resistor---Ground

VoltageSupply needs to be very clean and stable for this to be accurate (especially during ADC
reading). Adding voltage regulation/smoothing and stabilizing capacitors between VoltageSupply
and Ground may help. Adding a small capacitor (low micro-Farad range) between the ADC pin and
ground may also help. Too large a capacitor will slow the meter readings down but should still
work. Larger capacitors will help smooth the jitter out of multi-hour graphs.

Code Usage:

Global:
//Inside thermistor on Arduino ADC pin 0, Outside on pin 1.
#define THERMISTORPinInside 0
#define THERMISTORPinOutside 1
//One temperature monitoring thermistor for Inside and Outside.
Thermistor4 ThermistorInside, ThermistorOutside;
unsigned long ThermistorLastMillis; //last time something was run.
//If using protothreads...
static struct pt ptv_ThermistorReport; //these hold the states of the PT's.

setup():
//My 2 salvaged thermistors are about 30k at room temperature.
ThermistorInside.Pin = THERMISTORPinInside; //Set the pin number.
ThermistorOutside.Pin = THERMISTORPinOutside;
ThermistorInside.SetUp(); //Sets up the analog read pin for internal AVR.
ThermistorOutside.SetUp(); //If using an external ADC, write your own setup here.
//pow() is used elsewhere so might as well be used here.
ThermistorInside.BitResolution=pow(2, 10)-1; //ATmega's have a 10bit ADC (2^10-1).
ThermistorOutside.BitResolution=pow(2, 10)-1; //An external ADC may be lower or higher than 10bits.
ThermistorInside.VoltageSupply=4.95; //My USB powers my ATmega325 chip at 4.95v. Meter this for accuracy.
ThermistorOutside.VoltageSupply=4.95; //An external ADC may have different voltages. Meter this for accuracy.
ThermistorInside.ResistanceFixed=27200; //Fixed resistor in the divider. Measured in ohms. Meter this for accuracy.
ThermistorOutside.ResistanceFixed=27100; //The resistor divider should be calculated to maximize desired range.
ThermistorInside.Offset=0.5; //adjust temperature in Kelvin up or down a little to account for unforseen variations.
ThermistorOutside.Offset=0.5; //This will be by trial and error during final manual calibration.
//These numbers were generated from thermistor.sf.net and aren't quite right unless using the full equation.
ThermistorInside.SteinhartA1=1.560442157476244e-003;  //First Steinhart-Hart coefficient.
ThermistorInside.SteinhartA2=-1.298742723052728e-005; //Second Steinhart-Hart coefficient.
ThermistorInside.SteinhartA3=2.500508035979886e-005;  //Third Steinhart-Hart coefficient.
ThermistorInside.SteinhartA4=-7.698170259653937e-007; //Fourth Steinhart-Hart coefficient.
ThermistorOutside.SteinhartA1=2.975623989921602e-003;  //First Steinhart-Hart coefficient.
ThermistorOutside.SteinhartA2=-4.448067018378571e-004; //Second Steinhart-Hart coefficient.
ThermistorOutside.SteinhartA3=6.848365975770642e-005;  //Third Steinhart-Hart coefficient.
ThermistorOutside.SteinhartA4=-2.217688558250577e-006; //Fourth Steinhart-Hart coefficient.
//If using proto-threads, PT's need initializing before use.
PT_INIT(&ptv_ThermistorReport);

loop():
//PROTOTHREAD VERSION:
//loop forever and let the PT scheduler decide what needs running.
//The PT's will not run until their functions are called.
//If the functions exit, being in the loop will restart the thread.
//Each thread gets its own "pt" state variable.
//Since arguments and local function variables can't be used, use the class object's.
ptf_ThermistorReport(&ptv_ThermistorReport);
//This function exists somewhere else and acts on the thermistor values.

//NORMAL VERSION:
ThermistorInside.ReadCalculate(3);
Serial.print("Inside Temp: "); Serial.println(ThermistorInside.GetFarenheit(), 2);
ThermistorOutside.ReadCalculate(3);
Serial.print("Outside Temp: "); Serial.println(ThermistorOutside.GetFarenheit(), 2);

//Example CSV output that can be extracted from the other information and plotted.
//unix/linux: grep "^CSV," > outfile.csv
ThermistorInside.ReadCalculate(3);
Serial.print("CSV,"); Serial.print(ThermistorInside.GetFarenheit(), 2);
Serial.print(","); Serial.print(ThermistorInside.GetCentigrade(), 2);
ThermistorOutside.ReadCalculate(3);
Serial.print(","); Serial.print(ThermistorOutside.GetFarenheit(), 2);
Serial.print(","); Serial.println(ThermistorOutside.GetCentigrade(), 2);

*/

#ifndef THERMISTOR4_H
#define THERMISTOR4_H

//Bring in the Arduino stuff since it gets called.
//#include <WProgram.h>
#include <Arduino.h>
//for log() (natural logarithm, not log10).
#include <math.h>

//Set to 1 to include debug code, set to 0 to exclude.
//Extra prints add about 1k to the binary.
//This is convenient for calibrating, but usually unnecessary for a final project.
#define THERMISTORDEBUG 1

// Thermistor class to consolidate various variables and functions.
class Thermistor4 {

public:
//note: floats and doubles are both 4 bytes in avr-gcc.
unsigned char Pin; // analog pin number on the Arduino board the thermistor circuit is connected to.
unsigned int BitResolution; // such as an 8bit, 10bit, or 12bit ADC. Most newer Arduino boards are 10bit.
unsigned int ADCReading; // last reading from ADC.
double VoltageSupply; // supply voltage of the thermistor-divider. Manually meter this for accuracy.
double VoltageReading; // current voltage in the middle point of the thermistor-divider.
unsigned long ResistanceFixed; // fixed resistor between thermistor and ground, measured in ohms. Meter this for accuracy.
unsigned long ResistanceThermistor; // last calculated resistance of the thermistor, measured in ohms.
double SteinhartA1; // Calibrate/recalibrate the thermistor from: http://thermistor.sourceforge.net
double SteinhartA2; // SF and real SH coefficients are somehow different.
double SteinhartA3; // These 4 Steinhart-Hart coefficients are different for every thermistor...
double SteinhartA4; // ...and will help make sure the thermistor calculations are accurate over a wide range.
double Temperature; // calculated temperature in Kelvin.
double Offset; // adjust temperature in Kelvin up or down a little to account for unforseen variations.

//Thermistor4(); //Constructor, removed for size.
void SetUp();
void ReadADC(unsigned int);
void CalculateTemperature(unsigned char);
void ReadCalculate(unsigned char);
double GetCentigrade();
double GetFarenheit();
#if THERMISTORDEBUG
void Thermistor4SerialPrint();
#endif
};

#endif

//end of Thermistor4.h