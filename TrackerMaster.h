
//https://en.wikipedia.org/wiki/Analemma

#include "i2c_utils.h"


struct time // moverla a GpsService.h
{
  int year, month, day, hh, mm, ss;
  
};


// system states
enum class States : int {waiting, tracking, idle, parking};

class Tracker
{
public:

Tracker();//constructor




void mainUpdate(struct time t);
bool updSunPosition(double,double);

bool goEndA();
bool goEndE();
bool goPark();

void goTest();
// ------------------------------
// Move functions
// ------------------------------
bool moveBy(int sentido, float degrees);
bool moveUP(float degrees){ return moveBy(MOVE_UP, degrees);};  
bool moveDN(float degrees){ return moveBy(MOVE_DN, degrees);};        
bool moveLF(float degrees){ return moveBy(MOVE_LF, degrees);};         
bool moveRG(float degrees){ return moveBy(MOVE_RG, degrees);}; 
        

// Print
void printMasterVars();
void printSlaveVars();
void testSetters();
void printState();

void printCalculatedSPA(struct time);//Just por printing
void printLastSPA();//Last calculated sun position

// ------------------------------
//  Sun position
// ------------------------------
int SPA(struct time, double &, double &);

// ------------------------------
// Master gets
// ------------------------------

 float getLongitude(){return _longitude;};
 float getLatitude(){return _latitude;};
 float getElevation(){return _elevation;};
 int getTimezone(){return _timezone;};
 int getStartHour(){return _startHour;};
 int getStopHour(){return _stopHour;};
 char * getStateStr(); 


 float getLastSunAzimuth(){return _lastSunAzimuth;}; 
 float getLastSunElevation(){return _lastSunElevation;};

// ------------------------------
// Master sets
// ------------------------------

void setStateStart();
void setStateStop();
void setLocation(float longitude,float latitude,float elevation, int timezone);
void setWakeupSleep(int,int);

// ------------------------------
// Slave Gets
// ------------------------------

bool getEndstops(int &,int &);// usa I2C
int getEndstopASteps(){return 0;}
int getEndstopESteps(){return 0;}

char * getModeStr();
int getMode();//Mirror,Tracker
bool getSystemPosition(float & a, float & e);
bool getShift(float & a, float & e);
bool getTarget(float & a, float & e);
bool getPark(float & a, float & e);
 
// ------------------------------
// Slave sets
// ------------------------------
bool setShift(float a, float e){ return setValue2F(SET_SHIFT, a, e);};
bool setTarget(float a, float e){ return  setValue2F(SET_TARGET, a, e);}; 
bool setParking(float a, float e){  return setValue2F(SET_PARK, a, e);};
bool setModeMirror(){  return setValue2F(SET_MODE_MIRROR,0,0);};
bool setModeTracker(){  return setValue2F(SET_MODE_TRACKER,0,0);}; 
// ------------------------------

private:
  States state;// wait for command
  double	_latitude,_longitude,_elevation;
  int _timezone, _startHour,_stopHour;
  float _lastSunAzimuth, _lastSunElevation ;
 
};
