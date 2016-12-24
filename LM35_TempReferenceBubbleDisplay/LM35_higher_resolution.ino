double tempC;
double tempF;
int reading;
int tempPin = 1;
unsigned long sum;
unsigned long average;
void setup()
{
analogReference(INTERNAL);
Serial.begin(115200);
}

void loop()
{ sum = 0; average = 0;
  for(int i=0; i<99; i++){
    reading = analogRead(tempPin);
    sum += reading;
  }
  average = sum / 100;
  
//reading = analogRead(tempPin);
tempC = average / 9.31;      //reading / 9.31;
tempF = tempC * 9.0/5.0 + 32.0;
Serial.print(tempC);
Serial.print("\t");
Serial.println(tempF);
delay(1000);
}

