double tempC, tempF;

int reading;
int tempPin = 1;
double average;

void setup()
{
analogReference(INTERNAL);
Serial.begin(115200);
}

void loop()
{
reading = analogRead(tempPin);
tempC = reading / 9.309253398;
tempF = ((9.0/5.0)*tempC) + 32.0;
average=0.00;
for(int i=0; i<=999; i++){
  average += tempF;
  //Serial.print(i);
}
Serial.println("");
average/=1000;
Serial.println(average);
delay(1000);
}

