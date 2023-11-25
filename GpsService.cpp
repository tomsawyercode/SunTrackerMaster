//struct time { int year, month, day, hh, mm, ss;}; //defined in TrackMaster.h

#include "ESPTelnet.h" 
#include "HardwareSerial.h" 

#include <TinyGPS++.h>
static const int RXD2 = 16, TXD2 = 17;//ESP32-S 16: azul, 17:verde
TinyGPSPlus gps;
TinyGPSCustom satsInView(gps, "GPGSV", 3);  // $GPGSV sentence, third element //satsInView.value()



#include <Wire.h>
#include "i2c_utils.h"
#include "commands.h"

#include "GpsService.h"
 //-----------------------------------------------
// GPS Time Service 
//---------------------------------------------
//:_origin(SLAVE)

//GpsService::GpsService():_origin(SLAVE){// When is slave does not need SERIAL}

GpsService::GpsService(gpsOrigin origin){
 
   _origin=origin;

   if  (_origin==MASTER)  Serial2.begin(9600, SERIAL_8N1, 16,17);//RXD2,TXD2
  
}

void GpsService::retrieve(){
  if  (_origin==MASTER) while (Serial2.available()){gps.encode(Serial2.read());}
}

bool GpsService::getStatus(int & v1,int & v2,int & v3){

 if (_origin==MASTER)
   {
    v1= gps.location.isValid() && gps.date.isValid() && gps.time.isValid();
    v2= atoi(satsInView.value());
    v3= gps.satellites.value();// in use
    return true;
    }
   else
   {
   return getSlaveStatus(v1,v2,v3);
   }

}


void GpsService::getSats(int & v1,int & v2)
{  
    //memcpy(v1, &satsInView.value(),4); ;// in view    
    v1 = atoi(satsInView.value());
    
    /*
    Serial.println("  getSats:");
    Serial.printf("  v1: %ld \n" ,v1); 
    //Serial.print("   v1:");Serial.println(v1);        
    Serial.print("   SatsInView:");Serial.println(satsInView.value());
*/
    
     v2 = gps.satellites.value();// in use
}


// Request GPS status data from the slave device
bool GpsService::getSlaveStatus(int & v1,int & v2,int & v3)
{
  if(getValue3I(GET_GPS_STATUS,v1,v2,v3))  //getValue3I(isValid,inView,inUse)
 {    
    //Serial.printf("  getSlaveStatus: isvalid: %d, inView: %d, inUse: %d  \n" ,v1,v2,v3);    
    return true;
    }
  else
    {   
   Serial.println("FAIL to receive GPS is valid");
    return false;
    }
}


bool GpsService::getLocation(float & longitude,float & latitude,float & elevation){

 if (_origin==MASTER)
   {
    longitude = gps.location.lng();
    latitude =  gps.location.lat();
    elevation = gps.altitude.meters();
    return true;
    }
   else
   {
    return getSlaveLocation(longitude,latitude,elevation);
   }
}


// Populate SPA time struct
bool GpsService::getUtcDateTime(byte * p){

   if (_origin==MASTER)
   {
    updDateTimeFromMaster(p);
    return true; 
    }
   else
   {
   return updDateTimeFromSlave(p);
   }
}
// Populate SPA time struct
bool GpsService::updDateTimeFromMaster(byte * p){

  // Copy values to time struct
 
  int year = gps.date.year();        
  int month = gps.date.month();
  int day = gps.date.day();
  
  int hour = gps.time.hour();
  int minute = gps.time.minute();
  int second = gps.time.second();
  //https://cplusplus.com/reference/cstring/memcpy/?kw=memcpy
  memcpy(p, & year,4);     // year
  memcpy(p+4,& month,4);   // month
  memcpy(p+8,& day,4);     // day
  memcpy(p+12,& hour,4);   // hour
  memcpy(p+16,& minute,4); // minutes
  memcpy(p+20,& second,4); // seconds
  
  return true;
  
}

bool GpsService::updDateTimeFromSlave(byte * p){
   
 return (getSlaveDate(p) && getSlaveTime(p));
  
}


// Write in the same buffer --> time struct, defined in TrackMaster.h

bool GpsService::getSlaveDate(byte * p){
 if(sendCommand(GET_DATE,12)>0)
   {
    Wire.readBytes(p, 12);
    //Serial.printf(" getSlaveDate: %4d/%02d/%02d \r\n" ,*(int*)(p),*(int*)(p+4),*(int*)(p+8));
    return true;
   }
  else
    {   
   Serial.println("FAIL to receive Date");
    return false;
    }
}

// Write in the same buffer plus 3*sizeof(long)=12
// write in time struct, defined in TrackMaster.h
bool GpsService::getSlaveTime(byte * p){
 //Serial.printf("sizeof(buffer3I): %2d",sizeof(buffer3I));
   
 if(sendCommand(GET_TIME,12)>0)
   {
    
    Wire.readBytes(p+12, 12);    
    // type-cast and de-reference the pointer to see the value you want.
    //Serial.printf(" getSlaveTime: %02d:%02d:%02d utc london \r\n" ,*(int*)(p+12),*(int*)(p+16),*(int*)(p+20));
    //Serial.printf("pt Time: %02d:%02d:%02d utc london\r\n" ,t->hh,t->mm,t->ss);
    return true;
   }
  else
    {   
   Serial.println("FAIL to receive Time");
    return false;
    }
}


bool GpsService::getSlaveLocation(float & longitude,float & latitude, float & elevation){
 
 if(getValue3F(GET_LOCATION,longitude,latitude,elevation))
   {
    
    //Serial.printf("getSlaveLocation: Longitude: %0.6f, Latitude: %0.6f, Elevation: %0.2f  \n" ,longitude,latitude,elevation);        
    return true;
   }
  else
    {   
    Serial.println("FAIL to receive Location");
    return false;
    }
}


//https://forum.arduino.cc/t/a-pointer-to-either-serial1-or-serial2/208726/3
//https://forum.arduino.cc/t/a-pointer-to-either-serial1-or-serial2/208726/4
void GpsService::printVars(){   
 
   
   if (_origin==MASTER){
    Serial.println(" GPS origin is MASTER");
    printMasterVars();}
   else
   {
    Serial.println(" GPS origin is SLAVE");
    printSlaveVars();
   }
   
}


void GpsService::printSlaveVars(){



   int isValid,inView,inUse;
   getSlaveStatus( isValid,inView,inUse);
   Serial.printf("  SlaveGpsStatus:  is valid: %d,  Sats in view: %d, Sats in use: %d  \r\n" ,isValid, inView, inUse);
   //hs.printf("  SlaveGpsStatus:  is valid: %d,  Sats in view: %d, Sats in use: %d  \r\n" ,isValid, inView, inUse);      
   
   float longitude=0, latitude=0, elevation=0;
   getSlaveLocation(longitude,latitude,elevation);    
   
   Serial.printf("  Location: lon: %0.6f, lat: %0.6f, ele: %0.6f \r\n" ,longitude,latitude,elevation);     

   byte buf[6*4];
   byte * pt = (byte*) &buf;
   getSlaveDate(pt);
   getSlaveTime(pt); 
   Serial.printf("   Date: %4d/%02d/%02d \n" ,*(int*)(pt),*(int*)(pt+4),*(int*)(pt+8));
   Serial.printf("   Time: %02d:%02d:%02d utc london \n" ,*(int*)(pt+12),*(int*)(pt+16),*(int*)(pt+20));       
  
 }

 void  GpsService::printMasterVars(){


    Serial.printf("  MasterGpsStatus: is valid: %d, Sats in view: %d, Sats In Use: %d  \n" ,gps.location.isValid()&&gps.time.isValid()&&gps.date.isValid(),satsInView.value(),gps.satellites.value());
    
    Serial.printf("   Location: lon: %0.6f, lat: %0.6f, ele: %0.6f \r\n" , gps.location.lng(),gps.location.lat(),gps.altitude.meters());            
    Serial.printf("   Date: %d-%02d-%02d",gps.date.year(), gps.date.month(), gps.date.day());
    Serial.printf("   Time: %02d:%02d:%02d utc london \n",gps.time.hour(), gps.time.minute(),gps.time.second());
        
    
 }
