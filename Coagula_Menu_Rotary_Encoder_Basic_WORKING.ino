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
#include <MenuBackend.h>    //MenuBackend library - copyright by Alexander Brevig
#include <LiquidCrystal.h>  //this library is included in the Arduino IDE
#define BUTTON_PIN 7

////////  Function Prototypes
static void menuUsed(MenuUseEvent used);
static void menuChanged(MenuChangeEvent changed);
////////////////////////////////


int pulses = 0, A_SIG = 0, B_SIG = 1;
int value = 0, lastValue = 0, lastPulses = 0;
boolean pulseChange = false;
boolean buttonState = false;
boolean lastButtonState = false;
boolean buttonPressed = false;
boolean movingRight = false;
boolean movingLeft = false;

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
  attachInterrupt(0, A_RISE, RISING);
  attachInterrupt(1, B_RISE, RISING);
  Serial.begin(115200);
  lcd.begin(16, 2);
  pinMode(BUTTON_PIN, INPUT);

  //configure menu
  menu.getRoot().add(menu1Item1);
  menu1Item1.addRight(menu1Item2).addRight(menu1Item3);
  menu1Item1.add(menuItem1SubItem1).addRight(menuItem1SubItem2);
  menu1Item2.add(menuItem2SubItem1).addRight(menuItem2SubItem2).addRight(menuItem3SubItem3);
  menu.toRoot();
  lcd.setCursor(0, 0);
  lcd.print("www.coagula.org");
  delay(1000);

}  // setup()...


void loop() {

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
  lcd.print("www.coagula.org");
  menu.toRoot();  //back to Main
}


void  readButtons() { //read buttons status

  if (pulseChange == true) {
    pulseChange = false;        // if clockwise, increment counter
    if (pulses > lastPulses) {
      value++;
      movingRight = true;
      Serial.println(value);
    }
    else if (pulses < lastPulses) { // if counterclockwise, decrement counter
      value--;
      movingLeft = true;
      Serial.println(value);
    }
    lastPulses = pulses;        // save this for next time around
    lastValue = value;
  }


  buttonState = digitalRead(BUTTON_PIN);

  if (buttonState != lastButtonState) {   // if state has changed
    Serial.println(buttonState);
    if (buttonState == LOW) {           // and pin is LOW, then button has been pressed
      Serial.println("Button has been pressed..");
      buttonPressed = true;
      // button has been pressed
    }
  }

  lastButtonState = buttonState;    // save this for next time around




}
void navigateMenus() {
  MenuItem currentMenu = menu.getCurrent();

  /*switch (lastButtonPushed){
    case buttonPinEnter:
      if(!(currentMenu.moveDown())){  //if the current menu has a child and has been pressed enter then menu navigate to item below
        menu.use();
      }else{  //otherwise, if menu has no child and has been pressed enter the current menu is used
        menu.moveDown();
       }
      break;
    case buttonPinEsc:
      menu.toRoot();  //back to main
      break;
    case buttonPinRight:
      menu.moveRight();
      break;
    case buttonPinLeft:
      menu.moveLeft();
      break;
    }

    lastButtonPushed=0; //reset the lastButtonPushed variable
  */
  if (movingRight == true) {
    movingRight = false;
    menu.moveRight();
    Serial.print("Moving right");
  }
  else if (movingLeft == true) {
    movingLeft = false;
    menu.moveLeft();
    Serial.print("Moving left");
  }

  if (buttonPressed == true) {
    buttonPressed = false; // reset it

    Serial.print("BUTTON PRESSED!!");

    if (!(currentMenu.moveDown())) { //if the current menu has a child and has been pressed enter then menu navigate to item below
      menu.use();
    } else { //otherwise, if menu has no child and has been pressed enter the current menu is used
      menu.moveDown();
      Serial.println("Moving down menu");
      //menu.moveDown();
    }
  }
}

void A_RISE() {
  detachInterrupt(0);
  A_SIG = 1;

  if (B_SIG == 0)
    pulses++;//moving forward
  pulseChange = true;

  if (B_SIG == 1)
    pulses--;//moving reverse
  pulseChange = true;
  //Serial.println(pulses);
  attachInterrupt(0, A_FALL, FALLING);
}

void A_FALL() {
  detachInterrupt(0);
  A_SIG = 0;

  if (B_SIG == 1)
    pulses++;//moving forward
  //pulseChange = true;

  if (B_SIG == 0)
    pulses--;//moving reverse
  //pulseChange = true;

  //Serial.println(pulses);
  attachInterrupt(0, A_RISE, RISING);
}

void B_RISE() {
  detachInterrupt(1);
  B_SIG = 1;

  if (A_SIG == 1)
    pulses++;//moving forward
  if (A_SIG == 0)
    pulses--;//moving reverse
  //Serial.println(pulses);
  attachInterrupt(1, B_FALL, FALLING);
}

void B_FALL() {
  detachInterrupt(1);
  B_SIG = 0;

  if (A_SIG == 0)
    pulses++;//moving forward
  if (A_SIG == 1)
    pulses--;//moving reverse
  //Serial.println(pulses);
  attachInterrupt(1, B_RISE, RISING);
}




