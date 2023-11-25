#include "Arduino.h" //import byte and Serial definitions
#include <Wire.h>

#include "i2c_utils.h"

//---------------------------------------------
// I2C
//---------------------------------------------

const int SLAVE_ADDRESS = 42;
/*
int sendCommandByte (int cmd, const int responseSize)
  {
  Wire.beginTransmission (SLAVE_ADDRESS);
  //Wire.write (cmd);
  Wire.write ((byte *) &cmd, sizeof(cmd));
  Wire.endTransmission ();
  
  int received = Wire.requestFrom (SLAVE_ADDRESS, responseSize);  
   Serial.printf("received size:%d \r\n",received);
   return received;

  } 
*/

// Send Command without values, just to retrieve, for GETS
bool sendCommand (int code, int responseSize)
  {
   return sendCommandIFF(code,0.0,0.0,responseSize);
  }  

// Send Command with 2 float values  
bool sendCommandIFF (int code,float v1,float v2, int responseSize)
  {
  struct {int code;float v1;float v2;} command = {code,v1,v2};

  //Serial.printf("Master sendCommand:%d,%0.3f,%0.3f, size: %d \r\n",command.code,command.v1,command.v2,sizeof(command));
  //Serial.printf("time: %d, sendCommandIFF:%d,",millis(),command.code);
  //Serial.printf("sendCommandIFF: %d",command.code);

  Wire.beginTransmission (SLAVE_ADDRESS);
  Wire.write ((byte *) &command, sizeof(command));//4+4+4
  
  if (responseSize==0) return Wire.endTransmission()==0;
     //https://www.arduino.cc/reference/en/language/functions/communication/wire/endtransmission/
     //Returns
     //0: success.
     //1: data too long to fit in transmit buffer.
     //2: received NACK on transmit of address.
     //3: received NACK on transmit of data.
     //4: other error.
     //5: timeout
     
  //responseSize > 0    
  Wire.endTransmission(); 
  // Verify responseSize == receivedSize
  return responseSize == Wire.requestFrom(SLAVE_ADDRESS, responseSize);

  } 
/*
bool sendandreceiveWResponseIFF (int code,float v1,float v2, int responseSize)
  {
  struct {int code;float v1;float v2;} command = {code,v1,v2};

  //Serial.printf("Master sendCommand:%d,%0.3f,%0.3f, size: %d \r\n",command.code,command.v1,command.v2,sizeof(command));
  //Serial.printf("time: %d, sendCommandIFF:%d,",millis(),command.code);
  //Serial.printf("sendCommandIFF: %d",command.code);

  Wire.beginTransmission (SLAVE_ADDRESS);
  Wire.write ((byte *) &command, sizeof(command));//4+4+4
  
  if (Wire.endTransmission()!==0)
     return false
     //https://www.arduino.cc/reference/en/language/functions/communication/wire/endtransmission/
     //Returns
     //0: success.
     //1: data too long to fit in transmit buffer.
     //2: received NACK on transmit of address.
     //3: received NACK on transmit of data.
     //4: other error.
     //5: timeout     
  
  // Verify responseSize == receivedSize
  return responseSize == Wire.requestFrom(SLAVE_ADDRESS, responseSize);

  } 
  */
//---------------------------------------------
// I2C Setters and Getters 
//---------------------------------------------

//setValue return ack

bool setValue2F(int code,float v1,float v2){
  
 struct {int v1;int v2; } buffer2I ={0,0}; //Esp32: int=4 and long=4
 int const responseSize = 8;// Mega2560: long=4 * 2
 
 if(sendCommandIFF(code,v1,v2,responseSize))//8 force request, expect (1,code)
   {   
    Wire.readBytes((byte*)&buffer2I,responseSize);    
    Serial.printf(" setValue2F, ack:%d, code:%d \r\n" ,buffer2I.v1,buffer2I.v2);    
    return buffer2I.v1==1;
    
   }
  else
    {   
    Serial.println("    FAIL to setValue");
    return false;
    }
}
//---------------------------------------------------
// For I2C_ROUND_TRIP, send and receive same values+PI
bool setgetValue2F(int code,float v1,float v2, float & out1,float & out2){

 struct {float v1;float v2; } buffer2F ={0,0};
 
 int const responseSize = 8;// Mega2560 sizeof(float) * 2
 
 if(sendCommandIFF(code,v1,v2,responseSize))
   {   
     Wire.readBytes((byte*)&buffer2F,responseSize);    
    //Serial.printf(" getValue: %f:%f \r\n" ,buffer2F.v1,buffer2F.v2);
    out1 = buffer2F.v1;
    out2 = buffer2F.v2;
    return true;
   }
  else
    {   
    Serial.println("   FAIL to setgetValue2F");
    return false;
    }
}



//---------------------------------------------------
bool getValue2F(int code,float & f1,float & f2){

 struct {float v1;float v2; } buffer2F ={0,0};
 int const responseSize = 8;// Mega2560 sizeof(float) * 2

 if(sendCommand(code,responseSize))
   {
    Wire.readBytes((byte*)&buffer2F,responseSize);    
    //Serial.printf(" getValue: %f:%f \r\n" ,buffer2F.v1,buffer2F.v2);
    f1 = buffer2F.v1;
    f2 = buffer2F.v2;
    return true;
   }
  else
    {   
    Serial.printf("   FAIL to getValue2F, code: %d \r\n", code);
    return false;
    }
}
//---------------------------------------------------
bool getValue3F(int code,float & f1,float & f2,float & f3 ){

 struct {float v1;float v2;float v3; } buffer3F ={0,0,0};
 int const responseSize = 12;// Mega2560 sizeof(float) 4 * 3

 if(sendCommand(code,responseSize))
   {
    Wire.readBytes((byte*)&buffer3F,responseSize);    
    //Serial.printf("   getValue3F: %f:%f:%f \r\n" ,buffer3F.v1,buffer3F.v2,buffer3F.v3);
    f1 = buffer3F.v1;
    f2 = buffer3F.v2;
    f3 = buffer3F.v3;
    return true;
   }
  else
    {   
    Serial.printf("   FAIL to getValue2F, code: %d \r\n", code);
    return false;
    }
}
//---------------------------------------------------
//GET: Mode, EndStops, Losed Steps
bool getValue2I(int code,int & a, int & b){

 struct {int v1;int v2; } buffer2I ={0,0}; //Esp32 int=4 and long=4
 int const responseSize = 8;// Mega2560 sizeof(long) * 2
 if(sendCommand(code,responseSize))
   {
    Wire.readBytes((byte*)&buffer2I,responseSize);    
    //Serial.printf(" getValue2I: %d:%d \r\n" ,buffer2I.v1,buffer2I.v2);
    a = buffer2I.v1;
    b = buffer2I.v2;
    return true;
   }
  else
    {   
    Serial.printf("   FAIL to getValue2I, code: %d \r\n", code);
    return false;
    }
}

bool getValue3I(int code, int & a, int & b,  int & c){

 struct {int v1; int v2; int v3; } buffer3I ={0,0,0}; //Esp32 int=4 and long=4
 int const responseSize = 12;// Mega2560 sizeof(long 4 ) * 3
 if(sendCommand(code,responseSize))
   {
    Wire.readBytes((byte*)&buffer3I,responseSize);    
    //Serial.printf(" getValue3I buffer3I: %d:%d:%d \r\n" ,buffer3I.v1,buffer3I.v2,buffer3I.v3);
    a = buffer3I.v1;
    b = buffer3I.v2;
    c = buffer3I.v3;
    return true;
   }
  else
    {   
    Serial.printf("   FAIL to getValue3I, code: %d \r\n", code);
    return false;
    }
}
//bool getValue2I(int code,int & a, int & b, int & c){

//}
