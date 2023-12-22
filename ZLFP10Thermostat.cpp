


#include "ZLFP10Thermostat.h"

// Define pin modes for TX and RX


ZLFP10Thermostat::ZLFP10Thermostat(int pDHTSensorPin):sensor(pDHTSensorPin, DHT22)
{

}
// Set the baud rate for the SoftwareSerial object

void ZLFP10Thermostat::setup(HardwareSerial* phwSerial, int pRS485TXPin, int pRS485DEPin, int pRS485REPin, int pDHTSensorPin) {
  //sensor(pDHTSensorPin, DHT22)
    sensor.begin();

    RS485.setPins(pRS485TXPin, pRS485DEPin, pRS485REPin);
    RS485.setSerial(phwSerial);
    Serial.begin(9600);
    Serial.println("starting");

    if (!ModbusRTUClient.begin(9600)) {
        Serial.println("Failed to start Modbus RTU Server!");
        while (1)
            ;

    }
    fanspeed = 1;
    startingmode = true;
    
}

void ZLFP10Thermostat::ReadSettings() {
    int holdingCount = 21;
    int inputCount = 10;
    int holdingFirst = 0x6E8D;
    int inputFirst = 0xB6D1;

    short holdingData[holdingCount];
    short inputData[inputCount];
    ModbusRTUClient.requestFrom(address, HOLDING_REGISTERS, holdingFirst, holdingCount);
    int x;
    for (x = 0; x < holdingCount; x++) {
        holdingData[x] = ModbusRTUClient.read();
        /*   Serial.print(HoldingLabels[x]);
        Serial.print(" = ");
        Serial.println(holdingData[x]);*/

    }
    delay(150);  // need a little bit of time between requests

    ModbusRTUClient.requestFrom(address, INPUT_REGISTERS, inputFirst, inputCount);

    for (x = 0; x < inputCount; x++) {
        inputData[x] = ModbusRTUClient.read();
        /*Serial.print(InputLabels[x]);
     Serial.print(" = ");
     Serial.println(inputData[x]);*/
    }

    actualOnoff = holdingData[0];
    actualMode = holdingData[1];
    settemp = holdingData[10] - 30;

    reportedRoomTemp = inputData[0];
    coiltemp = inputData[1];
    actualFanspeed = inputData[2];
    reportedRPM = inputData[3];
    valveOpen = inputData[4];


}

void ZLFP10Thermostat::ReadTemp() {

    hum = sensor.readHumidity();
    temp = sensor.readTemperature();
}



//////////////////////////////////////////////////////////////////////////////
bool ZLFP10Thermostat::UpdateFanSpeed(float oldtemp) {


    if (upperthreshold > temp + 0.2)
    {
        upperthreshold = temp + 0.2;
        if (upperthreshold < ((float)(settemp + THERMOSTAT_INTERVAL)) / 10.0)
        {
            upperthreshold = ((float)(settemp + THERMOSTAT_INTERVAL)) / 10.0;
        }
    }
    if (lowerthreshold < temp - 0.2)
    {
        lowerthreshold = temp - 0.2;
        if (lowerthreshold > ((float)settemp - THERMOSTAT_INTERVAL) / 10.0)
        {
            lowerthreshold = ((float)settemp - THERMOSTAT_INTERVAL) / 10.0;
        }

    }


    short mode = 4;  // heating
    //mode=3; // vent
    short onoff;
    short setfanspeed;

    // if oldtemp is zero, we're being called for the first time
    if (oldtemp == 0)
    {
        // if we're outside the thermostat band, set to off or highest. Otherwise do nothing, just keep doing what you're doing
        upperthreshold = ((float)(settemp + THERMOSTAT_INTERVAL)) / 10.0;
        lowerthreshold = ((float)settemp - THERMOSTAT_INTERVAL) / 10.0;

        if (temp >= upperthreshold) {

            fanspeed = 0;
        }
        if (temp < lowerthreshold) {

            fanspeed = MAXFANSPEED;
        }
    }
    else
    {
        if (millis() - lastAdjustment > 240000) // don't adust fan more than once every three minutes
        {
            if (upperthreshold - temp < 0.09) {

                // only adjust speed down on rising temp

                fanspeed--;
                upperthreshold += 0.1;
                lastAdjustment = millis();
            }
            if (temp - lowerthreshold < 0.09)
            {
                // only adjust speed up on falling temperature
                fanspeed++;
                lowerthreshold -= 0.1;
                lastAdjustment = millis();
            }
        }
    }

    if (fanspeed < 1) {
        fanspeed = 1;
    }
    if (fanspeed > MAXFANSPEED) {
        fanspeed = MAXFANSPEED;
    }

    if (fanspeed == 1) {

        //onoff = 0;
        mode = 0; // auto
        onoff = 1;

    }
    else {
        // turn on, set fan speed
        onoff = 1;


    }

    // on startup, if we're outside the band we go to highest setting. When we reach the band, throttle back to 2
    if (startingmode)
    {
        if (temp > lowerthreshold)
        {
            if (fanspeed > 2)
                fanspeed = 2;
            startingmode = false;
            Serial.println("Leaving starting mode");
        }
    }

    if (actualMode == mode)
    {
        if (actualFanspeed == fanspeed)
        {
            return true;
        }
        if (fanspeed == 1)
        {
            return true;
        }
    }
    Serial.println();

    setfanspeed = fanspeed;
    int retval;
    retval = ModbusRTUClient.beginTransmission(address, HOLDING_REGISTERS, 0x6E8d, sizeof(setfanspeed) * 3);
    if (!retval) {
        Serial.println("Error in beginTransmission");
        return false;
    }
    retval = ModbusRTUClient.write(onoff);
    if (!retval) {
        Serial.println("Error in write onoff");
        return false;
    }
    retval = ModbusRTUClient.write(mode);
    if (!retval) {
        Serial.println("Error in write mode");
        return false;
    }
    retval = ModbusRTUClient.write(setfanspeed);
    if (!retval) {
        Serial.println("Error in write setfanspeed");
        return false;
    }
    delay(100); // need a slight delay here to insure reliable communications.
    retval = ModbusRTUClient.endTransmission();
    if (!retval) {
        Serial.println("Error in endTransmission");
        Serial.println();
        return false;
    }
    ReadSettings();
    return true;
}


void ZLFP10Thermostat::DisplayStatus() {
    unsigned long hour;
    unsigned long minutes;
    unsigned long seconds;
    unsigned long now = millis();
    seconds = now / 1000;
    minutes = now / 60000;
    seconds -= minutes * 60;

    hour = minutes / 60;
    minutes -= hour * 60;

    Serial.print(hour);
    Serial.print(":");
    if (minutes < 10)
        Serial.print('0');
    Serial.print(minutes);
    Serial.print(":");

    if (seconds < 10)
        Serial.print('0');
    Serial.print(seconds);
    Serial.print("  Temp=");
    Serial.print(temp, 1);
    Serial.print(", Humidity=");
    Serial.print(hum, 0);
    Serial.print(", Setpoint:");
    Serial.print(settemp / 10);
    Serial.print('.');
    Serial.print(settemp % 10);
    Serial.print(", Fan Speed=");
    Serial.print(fanspeed);
    Serial.print(", Reported Room Temp=");
    Serial.print(reportedRoomTemp / 10);
    Serial.print('.');
    Serial.print(reportedRoomTemp % 10);
    Serial.print(", Fan RPM=");
    Serial.print(reportedRPM);

    Serial.print(", Reported Fan setting=");
    Serial.print(actualFanspeed);
    Serial.print(", Coil Temperature=");
    Serial.print(coiltemp / 10);
    Serial.print(", Mode=");
    Serial.print(actualMode);
    Serial.print(", Valve=");
    Serial.print(valveOpen);
    Serial.print(" U=");
    Serial.print(upperthreshold);
    Serial.print(" L=");
    Serial.print(lowerthreshold);

    Serial.print('\r');
}



void ZLFP10Thermostat::loop() {

    // once a second read temp, if changed then 
    // once a minute, and on change in temp, read settings
    float oldtemp = temp;
    delay(1000);
    ReadTemp();
    if (!isnan(temp))
    {
        unsigned long now = millis();
        if (oldtemp != temp || now - lastcheck > 60000 || lastcheck == 0)
        {
            ReadSettings();

            if (UpdateFanSpeed(oldtemp))
            {
                lastcheck = now;
            }
            else
            {
                Serial.println();
                temp = oldtemp;
            }
        }
        DisplayStatus();
    }
    else
    {
        temp = oldtemp;
    }


}
/*

void get(int reg, char * desc)
{

      //ModbusRTUClient.requestFrom(address, HOLDING_REGISTERS, reg, 1);
      ModbusRTUClient.requestFrom(address, INPUT_REGISTERS, reg, 1);


  delay(200);
    short rawdata = ModbusRTUClient.read();

      Serial.print(desc);
      Serial.print(" = ");
      Serial.println(rawdata);



}
short settemp=250;
short fanspeed=0;
void loop2() {

  Serial.println(" Loop");
  //int retval=ModbusRTUClient.holdingRegisterWrite(15, 0x6E8F, 4); fancoild doesn't support this
/*  ModbusRTUClient.beginTransmission(15, HOLDING_REGISTERS, 0x6E97, sizeof(settemp));
  ModbusRTUClient.write(settemp);
  int retval=ModbusRTUClient.endTransmission();
  Serial.println(retval);
  Serial.println(settemp);
  Serial.println(".");
  delay(1000);*/
  /*

  if(fanspeed >0)
  {
  short mode=3; // ventilate
  int retval;
  retval=ModbusRTUClient.beginTransmission(15, HOLDING_REGISTERS, 0x6E8e, sizeof(fanspeed)*2);
  Serial.println(retval);
  retval=ModbusRTUClient.write(mode);
  Serial.println(retval);
  retval=ModbusRTUClient.write(fanspeed);
  Serial.println(retval);
  retval=ModbusRTUClient.endTransmission();
  Serial.println(retval);
  Serial.println(fanspeed--);
  Serial.println(".");

  }
delay(30000);
  int holdingFirst=0x6E8D;
  int inputFirst=0xB6D1;
  int holdingCount=21;
  int inputCount=10;


  ModbusRTUClient.requestFrom(15, HOLDING_REGISTERS, holdingFirst, holdingCount);
  int x;
  for(x=0; x< holdingCount; x++)
  {
      short rawdata = ModbusRTUClient.read();
      Serial.print(HoldingLabels[x]);
      Serial.print(" = ");
      Serial.println(rawdata);

  }
  ModbusRTUClient.requestFrom(15, INPUT_REGISTERS, inputFirst, inputCount);

  for(x=0; x< inputCount; x++)
  {
      short rawdata = ModbusRTUClient.read();
      Serial.print(InputLabels[x]);
      Serial.print(" = ");
      Serial.println(rawdata);

  }
  delay(1000);
  /*
   get(0x6E8D, "On/Off");

  get(0x6E8E, "Mode");
  get(0x6E97, "heating set temperature ");
  get(0x6E8F, "Fanspeed");
  get(0xB6D3,"Current fan speed");
  get(0xB6D2, "Coil temp");

  get(0xB6D1,"Room temperature" );
  get(0x6EA0,"Master/Slave");

  get(0x6EA1,"Unit address");
  get(0xB6D4,"Fan revolution");

 */
 /*

if(fanspeed > 5)
 while (1);
}*/

/*
const char * HoldingLabels[]={"On/Off",
"Mode",
"Fanspeed",
"","",
"Timer off",
"Timer off",
"Max. set temperature",
"Min. set temperature",
"Cooling set temperature",
"heating set temperature",
"Cooling set temperature at auto mode",
"heating set temperature at auto mode",
"Anti-cooling wind setting temperature",
"Whether to start anti-hot wind function",
"Whether to start ultra-low wind function",
"Whether to use vavle",
"Whether to use floor heating",
"Whether to use Fahrenheit",
"Master/Slave",
"Unit address"
};

char * InputLabels[]={"Room temperature",
"Coil temperature",
"Current fan speed",
"Fan revolution",
"Electromagnetic Valve",
"Remote on/off",
"Simulation signal",
"Fan speed signal feedback fault",
"Room temperature sensor fault",
"Coil temperature sensor fault"
};

void SetAutoTemps()
{
  int retval;
  short heattemp=200;
  short cooltemp=300;
  delay(100);

  Serial.print(" ");
  Serial.println("updating auto temps");
  retval = ModbusRTUClient.beginTransmission(address, HOLDING_REGISTERS, 0x6E98, sizeof(heattemp)*2 );
  if (!retval) {
    Serial.println("Error in beginTransmission");
  }
  retval = ModbusRTUClient.write(cooltemp);
  retval = ModbusRTUClient.write(heattemp);
  if (!retval) {
    Serial.println("Error in write mode");
  }
  delay(100);
  retval = ModbusRTUClient.endTransmission();

  if (!retval) {
    Serial.println("Error in endTransmission");
    delay(2000);
  }
  delay(100);
}
*/
/*void SetPlus5()
// set thermostat setpoint to 22.5
{
    int retval;
    short data5 = settemp + 5;
    delay(100);
    Serial.print(settemp);
    Serial.print(" ");
    Serial.println(settemp % 10);
    Serial.println("updating thermostat");
    retval = ModbusRTUClient.beginTransmission(address, HOLDING_REGISTERS, 0x6E97, sizeof(data5));
    if (!retval) {
        Serial.println("Error in beginTransmission");
    }
    retval = ModbusRTUClient.write(data5);
    if (!retval) {
        Serial.println("Error in write mode");
    }
    retval = ModbusRTUClient.endTransmission();
    delay(100);
    if (!retval) {
        Serial.println("Error in endTransmission");
        delay(2000);
    }
    delay(100);
}

*/