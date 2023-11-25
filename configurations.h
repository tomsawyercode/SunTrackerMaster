#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

// Sun speed in degrees/min
// 360/(24*60)=0.25 degrees/min, 4 min for 1 degress

#define WITH_BLUETOOTH // Bluetooth is a heavy weight librarie
#define WITH_TELNET
#define WITH_WEB  // Web Server interface

#define WIFI_SERVICE_NAME  "YOUR-WIFI"
#define PASSWORD "VERY-SECRET"


// GPS Service Origin, MODE can be: MASTER OR SLAVE
#define GPS_MODE_SLAVE
//#define GPS_MODE_MASTER

#define START_HOUR  9  //hs Localtime 
#define STOP_HOUR  17 //hs  Localtime 

#define TIMEZONE -3 // TODO calculate from longitude and a table


#endif // __CONFIGURATION_H__
