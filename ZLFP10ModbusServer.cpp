


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
  Serial.print("Count: ");
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
        Serial.println();
      
      
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
      word Register;
      word Count;
      Register=ResponseBufferGetAt(0);
      Count= ResponseBufferGetAt(1);
      Serial.println();
      Serial.print("Writing multiple registers ");
      Serial.print(Register);
      Serial.print(" ");
      Serial.print(Count);
      Serial.println();

      int x;
      for(x=0; x< Count; x++)
      {
        Serial.print(pClient->ResponseBufferGetAt(x+2), HEX);
        Serial.print(".");
        TransmitBufferPutAt(x, pClient->ResponseBufferGetAt(x+2));
      }
      pClient->writeMultipleRegisters(Register,Count);
      SendFrame(MODBUS10_WriteMultipleRegisters, ReceivedAddress(), Count, Register,false);
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