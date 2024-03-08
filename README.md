# ZLFP10Thermostat
 Arduino code for thermostatic control of heaters using the ZLFP10 controller

This is code for overriding the thermostatic control on hydronic fan coil units that use the ZLFP10, such as the Chiltrix CXI series.

This is a branch from the original project, this has been modified to use a different Modbus library and a software serial port, which allows it to work on a wider variety of Arduino boards. 

In order to use this you need: 
* An Arduino. So far the only boards I have found that it doesn't work on are ones that run at 3.3V like the Nano ESP32, the MAX485 transceiver requires 5V. I have tested it on Uno R1, R2 Wifi, R3 and R4.
* A MAX485 transceiver module (such as https://www.amazon.com/gp/product/B088Q8TD4V/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) 
* A DHT22 temperature and humidity sensor ( such as https://www.amazon.com/gp/product/B073F472JL/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
* An electrolytic capacitor. I'm not quite sure how small it can be, I'm currently using 680 uF



The code is set to use the following pin assignments: 

DHT sensor -- pin 6
MAX485 DE -- pin 7
MAX485 RE -- pin 8
MAX485 data is pins 1 and 2
Fan coil unit temperature input -- pin 9


The program reads the temperature setting of the fan coil unit. It then controls the fan to try and hold that setpoint, raising the fan speed when the temperature falls below the set point and lowering it when it rises above. If the unit is on the lowest fan speed and the temperature goes over the setpoint it turns the unit off. 

It does this by spoofing the temperature sensor on the unit. In automatic fan mode, the unit reads the room temperature and sets the fan speed depending on the difference between the set point and the room temperature. If the difference is 1c the fan is on the lowest setting, for each degree of additional difference the setting goes up step. If the difference is zero the fan turns off. 

There are two wires going to the temperature sensor on the unit. One is connected to ground and should be connected to the ground on the Arduino. The other one is cut and connected to a PWM pin (pin 9 by default) with the capacitor between the pin and ground.

On the ZLFP10 controller board there is a 5V pin that we will use to power the Arduino. You also need attachments to the Modbus pins.

The code also works in cooling mode. Currently in cooling mode the fan is never turned off, ultra-low is the lowest setting. This allows for continuous dehumidification.   


Construction notes: 
I built this using an Arduino Uno. 

I find a couple of things helpful while prototyping. One is a collection of breadboard connectors, like this: https://www.amazon.com/gp/product/B01EV70C78/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1
The connectors on these fit 18 gauge thermostat wire, so you can use a female-to-female connector to connect thermostat wire to a pin on a board. 

Another is insulation displacing crimp connectors, like these: https://www.amazon.com/gp/product/B008EAK2VK/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1
Even though they say 19 is the maximum wire size, I have no problem using them with 18 AWG thermostat wire.

I generally use 18 AWG thermostat wire throughout. The remote sensor takes 3 connectors so I use a 18/3 wire for it: 
https://www.amazon.com/gp/product/B07SZ5121N/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1

There are five connections coming off of the fan coil unit, so I use a 18/5 cable for it, just to keep the wire neat, even though I mount the arduino right next door: 
https://www.amazon.com/gp/product/B0BXCDZXWC/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1

My Arduino only has one 5V pin, so I make up a "bus" by crimping together everything that connects to Vcc. I also make a ground bus similarly.

Review of all of the connections. I list each connection twice, once on each end. This is to make it easier to double-check the wiring.

There are five connections coming out of the fan coil unit: 
* There is a gray 4-wire terminal block on the PCB, the connections are labeled 12V, 485A, 485B, and ground. Three of them are used:
  ** Ground goes to the ground "bus"
  ** 485 A goes to the "A" screw terminal on the 485Max chip (On my unit 485A and B came with wires attached and Wago connectors on them. The A wire is gray and he B wire is Green. Gray goes to A on the 485Max chip and Green goes to B.) 

  ** 485 B goes to the "B" screw terminal on the 485Max chip
* There is a four-pin connector about a half inch away from that terminal block
   ** Pin 1 is labeled 5V, it connects to the Vin pin of the Arduino
* The temperature sensor has two wires. One is connected to ground, the other one to the input. The input one should be cut and connected to Pin 9 of the Arduino and the positive side of the capacitor (use a crimp to join the two to a male jumper that goes into pin 9).
   I couldn't tell which was which, I cut them both and measured the voltage across them with a voltmeter, the one that was positive is the sensor input.
  After cutting these, I cut a female/female jumper in half and crimped one half onto each lead. I did the same thing with a male/male jumper on the other ends, so I could restore original operation by just plugging the male jumpers into the female sockets. I cut another male/male in half and crimped the two ends onto the positive terminal of the capacitor, one lead went into the female receptical on the sensor, the other lead went to pin 9 of the Arduino.

On the 485Max chip there are eight connections: 
* A goes to 485 A on the FCU, as described above.
* B similarly goes to 485 B
* Vcc goes to the Vcc bus on the Arduino
* Gnd goes to the ground bus on the Arduino
* DI Goes to pin 3 on the Arduino
* DE Goes to pin 8 on the Arduino
* RE Goes to pin 7 on the Arduino
* RO Goes to pin 2 on the Arduino

  The DHT22 Temperature/humidity sensor has three connections
* S (signal) goes to pin 6 on the Arduino
* Vcc goes to the Vcc bus on the Arduino
* Gnd goes to the ground bus on the Arduino

The capacitor should have a crimp on each lead:
* Negative side joins the ground bus
* Positive side is crimped with the temperature sensor input of the fan coil unit and a jumper going to Pin 9 of the Arduino.

  On the Arduino:
  * Pin 2 goes to R0 on the 485Max
  * Pin 3 goes to DI on the 485Max
  * Pin 6 goes to the S pin on the DHT22
  * Pin 7 goes to RE on the 485Max
  * Pin 8 goes to DE on the 485Max
  * Pin 9 goes to a crimp with the positive terminal of the capacitor, a jumper, and the temperature probe input to the fan coil unit
  * Vin goes to the 5V pin from the FCU PCB
  * 5V goes to the Vcc bus
  * Ground goes to the ground bus

Ground bus brings together: 
* Ground wire from the FCU temperature probe
* GND from the 485Max
* GND from the DHT22
* Negative terminal from the capacitor
* Ground from the Arduino

VCC bus brings together: 
* Vcc from the 485Max
* Vcc from the DHT22
* 5V from the Arduino
