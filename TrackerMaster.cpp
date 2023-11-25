
// Does not have access to GPS Services

#include "Arduino.h"
#include "TrackerMaster.h"
#include "spa.h"

#include <Wire.h>
//----------------------------------
// Constructor
//----------------------------------
Tracker::Tracker():_longitude(0.0),_latitude(0.0),_elevation(0.0),_timezone(0),
                  
                   _startHour(0),_stopHour(0),
                   _lastSunAzimuth(0), _lastSunElevation(0),
                    state(States::waiting)

                   
{ 
  //-------------------------------------------
  // 
  //-------------------------------------------
  ;
    
}


void Tracker::mainUpdate(struct time utc)
{
  int localhour =  utc.hh + _timezone; // Warning 17london --3= 20 local
  
  Serial.printf("Main Update: Local time: %02d:%02d:%02d, state: %s \n",localhour,utc.mm,utc.ss,getStateStr());
  
  //wake up only if waiting
  if (state==States::waiting){
      
    if (localhour >= _startHour && localhour < _stopHour)
     {     
      state=States::tracking; //begin of the day     
      Serial.print(" Wake Up, begin of the day\n, state: Tracking");
     }   
     else Serial.println("");
  }  
  // go to sleep
  if (state==States::tracking ){
    if ( localhour >= _stopHour ) //21 > 20 // 
     {     
      state=States::waiting; // end of the day      
      Serial.println(" Go sleep,  end of the day, state: Waiting");
     }
  }     
  
  // if state==tracking just move
  if (state==States::tracking )     
  {
     
     //Calculate sun position
     int result;
     double outA,outE;
     result = SPA(utc,outA,outE);  
     // Store last calculated sun Position
     _lastSunAzimuth=outA; 
     _lastSunElevation=outE;

     Serial.printf("  sun position E: %.2f ,A: %.2f \n",outA,outE);
     //goSunPosition(outA,outE);
     if (result==0)
      {
        updSunPosition(outA,outE);
      }
      else
      {
        Serial.printf("SPA Error Code: %d\n",result);
        
      } 
      
  }  
}
// The slave store values of: mode,shift,target,parking

bool Tracker::updSunPosition(double sunAzimuth,double sunElevation)
{  
    if(sendCommandIFF(UPD_SUN_POSITION,sunAzimuth,sunElevation,0))
   {   
    return true;
   }
  else
    {   
    Serial.println("FAIL to Update sun position");
    return false;
    }

}

bool Tracker::goEndA()
{ 
 state==States::idle; 
 if(sendCommandIFF(GO_END_A,0,0,0))
   {   
    return true;
   }
  else
    {   
    Serial.println("FAIL to goEndA");
    return false;
    } 
}

bool Tracker::goEndE()
{
  state = States::idle;  
  if(sendCommandIFF(GO_END_E,0,0,0))
   {   
    return true;
   }
  else
    {   
    Serial.println("FAIL to goEndE");
    return false;
    }
}
bool Tracker::goPark()
{
  if (state!=States::idle){ state=States::idle;  }

    if(sendCommandIFF(GO_PARK,0,0,0))
   {   
    return true;
   }
  else
    {   
    Serial.println("FAIL to goEndstopE");
    return false;
    }
     
}

//  #goTest
void Tracker::goTest() 
{
  
}

bool Tracker::moveBy(int code,float degrees)
{
    if(sendCommandIFF(code,degrees,0,0))
   {   
    return true;
   }
  else
    {   
    Serial.println("FAIL to MoveBy");
    return false;
    }
}


int Tracker::SPA(struct time t,double & outA,double & outE){
   
    int result=0;
        
    spa_data spa;  //declare the SPA structure
    
    spa.year          = t.year;
    spa.month         = t.month;
    spa.day           = t.day;
    spa.hour          = t.hh+_timezone;//Hora local
    spa.minute        = t.mm;
    spa.second        = t.ss;
    spa.timezone      = _timezone;
    spa.delta_ut1     = 0;
    spa.delta_t       = 67;
    spa.longitude     = _longitude;
    spa.latitude      = _latitude;
    spa.elevation     = _elevation;
    spa.pressure      = 820;
    spa.temperature   = 11;
    spa.slope         = 0;
    spa.azm_rotation  = 0;
    spa.atmos_refract = 0.5667;
    spa.function      = SPA_ZA_INC;

    //Serial.printf("timeZone:%d, Lon:%.2f, Lat:%.2f ",_timezone, _longitude,_latitude);
    

    //call the SPA calculate function and pass the SPA structure
    
    result = spa_calculate(&spa);// take 22 milliseconds
    
    //char msg[100]={0};
    //sprintf(msg,"SPA %d/%2d/%2d, %2d:%02d:%02d, A:%.6f, E:%.6f\n",t.year,t.month,t.day,t.hh,t.mm,t.ss,spa.azimuth,spa.zenith);
    //Serial.print(msg);

    //Print time
    //Serial.printf("--SPA Argument: Date: %d/%02d/%02d, Time: %2d:%02d:%02d\n",t.year,t.month,t.day,t.hh+_timezone,t.mm,t.ss);
    

    //https://www.torsten-hoffmann.de/apis/suncalcmooncalc/link_en.html
    Serial.println("SPA:");
    Serial.printf(" https://www.suncalc.org/#/%.6f,%.6f,16/%d.%d.%d/%d:%d/1/3 \n",_latitude, _longitude,t.year,t.month,t.day,t.hh+_timezone,t.mm);
    

    //Valor Crudo
    //Serial.printf(" crudo  A:%02.6f, E:%02.6f\n",spa.azimuth,spa.zenith);

    // Move SPA Azimuth reference to coincide with mechanical reference system
    // Values are for SOUTH hemisphere
    // SPA Reference system is:
    /*
                  
                 North
                 360-0
                    |
                    |
       West 270 ----+---- 90 East
                    |
                    |
                   180
                  South

     Mechanical Reference system:       
                  
                 North
                   90
            2/4     |    1/4
                    |
       West 180 ----+---- 0 East
                    |
            3/4     |    4/4
                   270
                  South


    */

    //  1 of 4:
    // Mechanics system: 0 to 90
    // SPA 90-0 is reversed
    if (0 < spa.azimuth && spa.azimuth<90.00)
    {
        outA=90-spa.azimuth;
        Serial.printf("1/4:  0 < %00.2f <90, output: %00.2f \n",spa.azimuth,outA);
    }
    //  2 of 4:  
    //  Mechanics system A: 90 to 180
    //  SPA between: 270,360 
    if (270 < spa.azimuth && spa.azimuth<360.00)
    {        
        outA=450-spa.azimuth;//360+90-spa.azimuth;
        Serial.printf("2/4:  270< %0.2f <360, output: %00.2f \n ",spa.azimuth,outA);
    }
    //  3 of 4:
    //  Mechanics system A: greater than 180
    //  SPA between:180, 270 
    if ( 180 < spa.azimuth && spa.azimuth<270.00)
    {
        outA=450-spa.azimuth;// 270-spa.azimuth + 180
        Serial.printf("3/4:  180< %0.2f <270, output: %00.2f \n ",spa.azimuth,outA);        
    }

    //  4 of 4: 
    //  Mechanics system A: 0 to - 90
    //  SPA between:90, 180
    
    if ( 90 < spa.azimuth && spa.azimuth<180.00)
    {
        outA=90-spa.azimuth;// 270-spa.azimuth + 180
        //outA=0;//
        Serial.printf("4/4:  90< %0.2f <180, output: %00.2f \n ",spa.azimuth,outA);        
    }



    
    // Move Elevation reference system

    outE=90.00-spa.zenith;
    
     
    // Valor Corregido
    // Serial.printf(" system A:%02.6f, E:%02.6f \n",outA,outE);    
    // Web
    // Serial.printf(" web     A:%02.6f, E:%02.6f\n",spa.azimuth,outE);
    

   
   
   return result;   


}

void Tracker::printCalculatedSPA(struct time t) 
{

    int result;    
    //char msg[50]={0};
    
    //delay(0);
    //struct time t ={2022,1,1,8,0,0};
    
    double outE=0.0;
    double outA=0.0;

    result = SPA(t, outA,  outE);
    
    if (result == 0)  //check for SPA errors
    {
      //display the results inside the SPA structure
      Serial.printf("Date: %d/%2d/%2d, Time: %2d:%02d:%02d utc London,\n A:%02.6f, E:%02.6f\n",t.year,t.month,t.day,t.hh,t.mm,t.ss,outA,outE);                    
      //Serial.printf("SPA A:%.6f, E:%.6f\n",outA,outE);

    } else
    {
      Serial.printf("Result Error Code: %d\n",result);
    } 
 

}

void Tracker::printLastSPA() 
{
  Serial.printf(" Sun Position:  A: %02.2f,  E: %02.2f  \n", _lastSunAzimuth, _lastSunElevation);
}

void Tracker::printState()
{
   if(state==States::idle) Serial.println(" idle"); 
   if(state==States::tracking) Serial.println(" tracking"); 
   if(state==States::parking) Serial.println(" parking"); 
   if(state==States::waiting) Serial.println(" waiting"); 
}

char * Tracker::getStateStr()
{
   if(state==States::idle) return "idle"; 
   if(state==States::tracking) return "tracking"; 
   if(state==States::parking) return  "parking"; 
   if(state==States::waiting) return  "waiting"; 
}

bool Tracker::getEndstops(int & a,int & e){  
  
  return getValue2I(GET_END_STOPS,a,e);
}


char * Tracker::getModeStr(){
  int i1=0,i2=0;
  getValue2I(GET_MODE,i1,i2);  
  if(i1==0) return "Mirror";
  if(i1==1) return "Tracker";
  if(i1!=0 && i1!= 1 ) return "Unknow";   
  
}

int Tracker::getMode(){
  int i1=0,i2=0;
  getValue2I(GET_MODE,i1,i2);  
  return  i1;
  
  //if(i1==1) return "Tracker";
  //if(i1!=0 && i1!= 1 ) return "Unknow";   
  
}

bool Tracker::getSystemPosition(float & a, float & e){   
   //float a1=0,e1=0;
   return getValue2F(GET_SYSTEM_POSITION,a,e);
   
   
}

bool Tracker::getShift(float & a, float & e){
  return getValue2F(GET_SHIFT,a,e);
}

bool Tracker::getTarget(float & a, float & e){
  return getValue2F(GET_TARGET,a,e);
}

bool Tracker::getPark(float & a, float & e){
  return  getValue2F(GET_PARK,a,e);
}


void Tracker::testSetters(){
  
  //Serial.println("Print Slave Vars:");
  bool ack; 

  ack = setValue2F(SET_PARK, 11, 11);
  Serial.printf("SET_PARK, ack: %d, code: %d\n" ,ack,SET_PARK); 
  
  ack = setValue2F(SET_SHIFT, 11, 11);  
  Serial.printf("SET_SHIFT, ack: %d, code: %d\n" ,ack, SET_SHIFT); 
   
  ack = setValue2F(SET_TARGET,22,22);
  Serial.printf("SET_TARGET, ack: %d,  code: %d\n" ,ack,SET_TARGET);  
  
  ack = setValue2F(SET_MODE_MIRROR,0,0);
  Serial.printf("SET_MODE_MIRROR, ack: %d, code: %d\n" ,ack,SET_MODE_MIRROR); 
  
  ack = setValue2F(SET_MODE_TRACKER,0,0); 
  Serial.printf("SET_MODE_TRACKER, ack: %d, code: %d\n" ,ack,SET_MODE_TRACKER);  
  
  
  
 
  
        
}         
void Tracker::printMasterVars(){
  Serial.printf(" ----------------:\n");
  Serial.printf(" State: %s\n", getStateStr());
  Serial.printf(" Location: longitude: %02.4f, latitude: %02.4f elevation: %02.4f \n",_longitude,_latitude,_elevation);
  
  Serial.printf(" Time settings: timezone: %d, start: %d hs, stop: %d hs  \n", _timezone, _startHour,_stopHour);
  Serial.printf(" Last calculated sun position:  A: %02.2f,  E: %02.2f  \n", _lastSunAzimuth, _lastSunElevation);  
  
}        

void Tracker::printSlaveVars(){
  
  //Serial.println("Print Slave Vars:");
  int i1=10,i2=10;
  getEndstops(i1,i2);
  Serial.printf(" getEndstops, A: %d, E: %d \n" ,i1,i2);
  i1=99;i2=99;
  getValue2I(GET_END_STOPS,i1,i2);
  Serial.printf(" GET_END_STOPS, A: %d, E: %d \n" ,i1,i2);
  getValue2I(GET_END_STOPS_STEPS,i1,i2);
  Serial.printf(" GET_STEPS_LOOSE, A: %d, E: %d \n" ,i1,i2);
  getValue2I(GET_MODE,i1,i2);  
  if(i1==0) Serial.printf(" Mode: %d, Mirror, Received: %d \n" ,i1,i2); //mirror=0,tracker=1
  if(i1==1) Serial.printf(" Mode: %d, Tracker, Received: %d  \n" ,i1,i2);
  if(i1!=0 && i1!= 1 ) Serial.printf(" GET_MODE: %d, Unknow, Received: %d \n" ,i1,i2);
  
  
  float f1=0.0,f2=0.0;
  getValue2F(GET_SYSTEM_POSITION,f1,f2);
  Serial.printf(" System from ZERO, A: %02.2f, E: %02.2f \n" ,f1,f2);
  getValue2F(GET_SHIFT,f1,f2);
  Serial.printf(" Shift, A: %02.2f, E: %02.2f \n" ,f1,f2);
  getValue2F(GET_TARGET,f1,f2);
  Serial.printf(" Target, A: %02.2f, E: %02.2f \n" ,f1,f2);

  getValue2F(GET_PARK,f1,f2);
  Serial.printf(" Park, A: %02.2f, E: %02.2f \n" ,f1,f2);
  
  float out1=80.80,out2=90.90;
  setgetValue2F(I2C_ROUND_TRIP,10,20,out1,out2);
  Serial.printf(" I2C_ROUND_TRIP Send: 10 20, Receive: %02.4f, %02.4f \n" ,out1,out2);
  /*
  Serial.printf(" sizeof(int): %d",sizeof(int));
  Serial.printf(" sizeof(long): %d",sizeof(long));
  Serial.printf(" sizeof(float): %d",sizeof(float));
  Serial.printf(" sizeof(double): %d",sizeof(double));
  */

 Serial.printf(" Location: longitude: %02.4f latitude: %02.4f elevation: %02.4f \n",_longitude,_latitude,_elevation);
 Serial.printf(" Time settings: timezone: %d, start: %d hs, stop: %d hs", _timezone, _startHour,_stopHour);

}  


void Tracker::setLocation(float longitude,float latitude,float elevation, int timezone)
{
  _longitude=longitude;
  _latitude=latitude;
  _elevation=elevation;
  _timezone = timezone;  
}


void Tracker::setWakeupSleep(int start, int stop){
  _startHour=start;
  _stopHour=stop;
}

void Tracker::setStateStop(){
  if (state!=States::idle) state=States::idle;
  //Serial.printf("setStateStop state: %s ",getStateStr());
}

void Tracker::setStateStart(){
  if (state!=States::tracking) state=States::tracking;
  //Serial.printf("setStateStart state: %s ",getStateStr());
}
