# ZLFP10Thermostat
 Arduino code for thermostatic control of heaters using the ZLFP10 controller

This is code for overriding the thermostatic control on hydronic fan coil units that use the ZLFP10, such as the Chiltrix CXI series.

In order to use this you need: 
* An Arduino
* A MAX485 transceiver module (such as https://www.amazon.com/gp/product/B088Q8TD4V/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) 
* A DHT22 temperature and humidity sensor ( such as https://www.amazon.com/gp/product/B073F472JL/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) 



The code is set to use the following pin assignments: 

DHT sensor -- pin 6
MAX485 Tx- pin 18
MAX485 DE -- pin 7
MAX485 RE -- pin 8
MAX485 data is on Serial1 (pins 1 and 2)


The program reads the temperature setting of the fan coil unit, and subtracts three degrees C. This is done to override the internal thermostatic control which is flaky. It then controls the fan to try and hold that setpoint, raising the fan speed when the temperature falls below the set point and lowering it when it rises above. If the unit is on the lowest fan speed and the temperature is over the setpoint it turns the unit off, this is done by switching modes from heating to automatic. The heating setpoint for automatic mode has to be well below the setpoint in order for this to work.