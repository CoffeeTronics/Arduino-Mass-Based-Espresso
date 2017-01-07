This code reads the Vcc voltage rails and uses that value for the calculations of the 
thermistor using the ADC.

It reads the ADC an odd number of times into an array (currently 51, however this should 
be able to be changed in the menu).

It then sorts the readings in acending order and rejects the top and bottom 15% (This value
should alse be able to be changed in the menu) to avaoid sensor jitter).

I want to also add in a software Schmidt Trigger which is a comparator with two threshold 
points, so that when a smoothed reading oscillates between 0.5 and 0.6, or 0.4 and 0.5, 
the display does not flicker back and forth.

*********************************************
Work to be done on this
********************************************

Software Schmidtt Trigger
Communicate with menu to change the number of ADC readings
Communicate with menu for percent rejection


