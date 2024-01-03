# ZLFP10Thermostat
 Arduino code for thermostatic control of heaters using the ZLFP10 controller

This is code for overriding the thermostatic control on hydronic fan coil units that use the ZLFP10, such as the Chiltrix CXI series.

In order to use this you need: 
* An Arduino
* A MAX485 transceiver module (such as https://www.amazon.com/gp/product/B088Q8TD4V/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) 
* A DHT22 temperature and humidity sensor ( such as https://www.amazon.com/gp/product/B073F472JL/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
* An electrolytic capacitor. I'm not quite sure how small it can be, I'm currently using 680 uF



The code is set to use the following pin assignments: 

DHT sensor -- pin 6
MAX485 Tx- pin 18
MAX485 DE -- pin 7
MAX485 RE -- pin 8
MAX485 data is on Serial1 (pins 1 and 2)
Fan coil unit temperature input -- pin 9


The program reads the temperature setting of the fan coil unit. It then controls the fan to try and hold that setpoint, raising the fan speed when the temperature falls below the set point and lowering it when it rises above. If the unit is on the lowest fan speed and the temperature is over the setpoint it turns the unit off. 

It does this by spoofing the temperature sensor on the unit. In automatic fan mode, the unit reads the room temperature and sets the fan speed depending on the difference between the set point and the room temperature. If the difference is 1c the fan is on the lowest setting, for each degree of additional difference the setting goes up step. If the difference is zero the fan turns off. 

There are two wires going to the temperature sensor on the unit. One is connected to ground. The other one is cut and connected to a PWM pin (pin 9 by default) with the capacitor between the pin and ground.

On the ZLFP10 controller board there is a 5V pin that is used to power the Arduino. You also need attachments to the Modbus pins, and the ground that is next to them.
