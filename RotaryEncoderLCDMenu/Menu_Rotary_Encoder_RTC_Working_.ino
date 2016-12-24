/*
    Copyright Giuseppe Di Cillo (www.coagula.org)
    Contact: dicillo@coagula.org

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
  IMPORTANT: to use the menubackend library by Alexander Brevig download it at http://www.arduino.cc/playground/uploads/Profiles/MenuBackend_1-4.zip and add the next code at line 195
    void toRoot() {
        setCurrent( &getRoot() );
    }
*/

/*
 * Rotary Encoder Interrupt code ws taken from:
 * http://www.instructables.com/id/Improved-Arduino-Rotary-Encoder-Reading/?ALLSTEPS
 * */
#include <MenuBackend.h>    //MenuBackend library - copyright by Alexander Brevig
#include <LiquidCrystal.h>  //this library is included in the Arduino IDE#include <Wire.h>
#include "RTClib.h"
#define BUTTON_PIN 7

////////  Function Prototypes
static void menuUsed(MenuUseEvent used);
static void menuChanged(MenuChangeEvent changed);
////////////////////////////////

RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char monthsOfTheYear[12][3] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

static int pinA = 2; // Our first hardware interrupt pin is digital pin 2
static int pinB = 3; // Our second hardware interrupt pin is digital pin 3
volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
volatile byte encoderPos = 0; //this variable stores our current value of encoder position. Change to int or uin16_t instead of byte if you want to record a larger range than 0-255
volatile byte oldEncPos = 0; //stores the last encoder position value so we can compare to the current reading and see if it has changed (so we know when to print to the serial monitor)
volatile byte reading = 0; //somewhere to store the direct values we read from our interrupt pins before checking to see if we have moved a whole detent


unsigned long timeNow = 0;
unsigned long timeLast = 0;
boolean moveMenuLeft = false;
boolean moveMenuRight = false;
boolean encoderButtonPressed = false;
int buttonState = 0;
int lastButtonState = 0;


LiquidCrystal lcd(8, 13, 9, 10, 11, 12);

//Menu variables
MenuBackend menu = MenuBackend(menuUsed, menuChanged);
//initialize menuitems
MenuItem menu1Item1 = MenuItem("Item1");
MenuItem menuItem1SubItem1 = MenuItem("Item1SubItem1");
MenuItem menuItem1SubItem2 = MenuItem("Item1SubItem2");
MenuItem menu1Item2 = MenuItem("Item2");
MenuItem menuItem2SubItem1 = MenuItem("Item2SubItem1");
MenuItem menuItem2SubItem2 = MenuItem("Item2SubItem2");
MenuItem menuItem3SubItem3 = MenuItem("Item2SubItem3");
MenuItem menu1Item3 = MenuItem("Item3");




void setup() {
  pinMode(pinA, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(pinB, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  attachInterrupt(0,PinA,RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1,PinB,RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)
  
  Serial.begin(115200);
  if (! rtc.begin()) {
    lcd.setCursor(0,0);
    lcd.print("NO RTC!!");
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  
  //Serial.begin(115200);
  
  lcd.begin(16, 2);
  lcd.print("Espresso Control");
  delay(1000);
  lcd.clear();

  //configure menu
  menu.getRoot().add(menu1Item1);
  menu1Item1.addRight(menu1Item2).addRight(menu1Item3);
  menu1Item1.add(menuItem1SubItem1).addRight(menuItem1SubItem2);
  menu1Item2.add(menuItem2SubItem1).addRight(menuItem2SubItem2).addRight(menuItem3SubItem3);
  menu.toRoot();
  lcd.setCursor(0, 0);
  

}  // setup()...


void loop() {
  timeNow = millis();
  if ((timeNow - timeLast)>=200){
    ds1307RTC();
  }
  readButtons();  //I splitted button reading and navigation in two procedures because
  navigateMenus();  //in some situations I want to use the button for other purpose (eg. to change some settings)

  

} //loop()...


void menuChanged(MenuChangeEvent changed) {

  MenuItem newMenuItem = changed.to; //get the destination menu

  lcd.setCursor(0, 1); //set the start position for lcd printing to the second row

  if (newMenuItem.getName() == menu.getRoot()) {
    lcd.print("Main Menu       ");
  } else if (newMenuItem.getName() == "Item1") {
    lcd.print("Item1           ");
  } else if (newMenuItem.getName() == "Item1SubItem1") {
    lcd.print("Item1SubItem1");
  } else if (newMenuItem.getName() == "Item1SubItem2") {
    lcd.print("Item1SubItem2   ");
  } else if (newMenuItem.getName() == "Item2") {
    lcd.print("Item2           ");
  } else if (newMenuItem.getName() == "Item2SubItem1") {
    lcd.print("Item2SubItem1   ");
  } else if (newMenuItem.getName() == "Item2SubItem2") {
    lcd.print("Item2SubItem2   ");
  } else if (newMenuItem.getName() == "Item2SubItem3") {
    lcd.print("Item2SubItem3   ");
  } else if (newMenuItem.getName() == "Item3") {
    lcd.print("Item3           ");
  }
}

void menuUsed(MenuUseEvent used) {
  lcd.setCursor(0, 0);
  lcd.print("You used        ");
  lcd.setCursor(0, 1);
  lcd.print(used.item.getName());
  delay(3000);  //delay to allow message reading
  lcd.setCursor(0, 0);
  //lcd.print("www.coagula.org");
  menu.toRoot();  //back to Main
}


void  readButtons() { //read buttons status
  buttonState = digitalRead(BUTTON_PIN);
  if(buttonState != lastButtonState){
    if(buttonState == LOW){
      encoderButtonPressed = true;
      Serial.println("Encoder Button Pressed!");
    } 
    else encoderButtonPressed = false;
  }
  
  
  if(oldEncPos != encoderPos){
    if (encoderPos > oldEncPos){
      moveMenuRight = true;
        oldEncPos = encoderPos;

      //Serial.println(encoderPos);
    }
    else if(encoderPos < oldEncPos){
      moveMenuLeft = true;
        oldEncPos = encoderPos;

      //Serial.println(encoderPos);
    }
  }


/*
if(oldEncPos != encoderPos) {
    Serial.println(encoderPos);
    oldEncPos = encoderPos;
  }

*/
  lastButtonState = buttonState;

}
void navigateMenus() {
  MenuItem currentMenu = menu.getCurrent();

  if(moveMenuLeft == true){
    moveMenuLeft = false; //reset
    menu.moveLeft();
  }
  if(moveMenuRight == true){
    moveMenuRight = false;
    menu.moveRight();
  }
  if(encoderButtonPressed == true){
    encoderButtonPressed = false;   // reset
    if(!(currentMenu.moveDown())){  //if the current menu has a child and has been pressed enter then menu navigate to item below
        menu.use();
      }else{  //otherwise, if menu has no child and has been pressed enter the current menu is used
        menu.moveDown();
       }
  }
  
}

void PinA(){
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if(reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos --; //decrement the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00000100) bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void PinB(){
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos ++; //increment the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}


void ds1307RTC(){
  DateTime now = rtc.now();
  lcd.setCursor(0,0);
  lcd.print(now.day());
  lcd.print("/");
  lcd.print(now.month());lcd.print(" ");
  lcd.print(now.hour()); lcd.print(":"); 
  lcd.print(now.minute()); lcd.print(":"); 
  lcd.print(now.second());
  lcd.print("  ");
    
}


