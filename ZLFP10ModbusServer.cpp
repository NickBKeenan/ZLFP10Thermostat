


#include "ZLFP10ModbusServer.h"
#include "ZLFP10Thermostat.h"

void ZLFP10ModbusServer::RequestReaction()
{

  switch(ReceivedFunctionCode())
  {
      case MODBUS04_ReadInputRegisters:
        readInputRegisters();  
        break;
      case MODBUS03_ReadHoldingRegisters:
        readHoldingRegisters(); 
        break;
    case MODBUS10_WriteMultipleRegisters:
      writeMultipleRegisters();
      break;
    default:
    IllegalFunction();

  }

}

void ZLFP10ModbusServer::readHoldingRegisters()
{
  // called when a packet has been received
  uint16_t Register, Response;
  word Count;
  Register=ResponseBufferGetAt(0);
  Count= ResponseBufferGetAt(1);
  Serial.println();
  Serial.print("Reading Holding Registers (03). Register: ");
  Serial.print(Register, HEX);
  Serial.print(" Count: ");
  Serial.print(Count);
  Serial.println();
  

  pClient->readHoldingRegisters(Register,Count);

  int x;
  for(x=0; x< Count; x++)
  {
    
    TransmitBufferPutAt(x, pClient->ResponseBufferGetAt(x));
  }  
  SendFrame(MODBUS03_ReadHoldingRegisters, ReceivedAddress(), Count, Register,false);
}

   
void  ZLFP10ModbusServer::readInputRegisters()
{
      uint16_t Register;
      word Count;
      Register=ResponseBufferGetAt(0);
      Count= ResponseBufferGetAt(1);
      if(Register == 0x9999)
      {
        // get actual room temp
        int temp;
        temp=pParentThermostat->getTemp()*10;
        TransmitBufferPutAt(0, temp);
        Serial.println();
        Serial.print("Reading Temperature: ");
        Serial.print(temp);
        Serial.print(" (raw value: ");
        Serial.print(temp);
        Serial.print(", temperature: ");
        Serial.print(temp/10.0, 1);
        Serial.print("°C)");
        Serial.println();
        Serial.print("DEBUG: Register 0x9999/39321 returning value: ");
        Serial.println(temp);
      
      
      SendFrame(MODBUS04_ReadInputRegisters, ReceivedAddress(), Count, Register,false);
        return;
      }
      Serial.println();
      Serial.print("Reading Input Registers (04). Register: ");
      Serial.print(Register, HEX);
      Serial.print("Count: ");
      Serial.print(Count);
      Serial.println();

      
  pClient->readInputRegisters(Register,Count);

  int x;
  for(x=0; x< Count; x++)
  {
  
    TransmitBufferPutAt(x, pClient->ResponseBufferGetAt(x));
  }
  SendFrame(MODBUS04_ReadInputRegisters, ReceivedAddress(), Count, Register,false);

}


void  ZLFP10ModbusServer::writeMultipleRegisters()
{
  // Parse according to observed buffer layout
  uint16_t Register = ResponseBufferGetAt(0);
  uint16_t Count = ResponseBufferGetAt(1);

  Serial.println();
  Serial.print("Writing multiple registers. Register: 0x");
  Serial.print(Register, HEX);
  Serial.print(" Count: ");
  Serial.print(Count);
  Serial.println();
  
  // Data starts at index 2
  for (int i = 0; i < Count; i++) {
    uint16_t value = ResponseBufferGetAt(2 + i);
    Serial.print(" Register ");
    Serial.print(Register + i);
    Serial.print(" Value: 0x");
    Serial.println(value, HEX);

    //need to put the values in pClient so they are included as part of the frame
    pClient->TransmitBufferPutAt(i, value);
  }

  // Forward the command to the FCU
  pClient->writeMultipleRegisters(Register, Count);
  SendFrame(MODBUS10_WriteMultipleRegisters, ReceivedAddress(), Count, Register, false);
  Serial.println();
}

void ZLFP10ModbusServer::IllegalFunction()
{

}

void ZLFP10ModbusServer::SetupClient(ModbusClient* p_pClient)
{
  pClient=p_pClient;
}

void ZLFP10ModbusServer::SetParentThermostat(MultiStageThermostat * p_pParentThermostat)
{
  pParentThermostat=p_pParentThermostat;
}