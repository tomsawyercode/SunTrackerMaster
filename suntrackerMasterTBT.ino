
#include "configurations.h"
#include <Wire.h>
#if defined(WITH_TELNET)
  #include "ESPTelnet.h" // WiFi.h is also included https://github.com/LennartHennigs/ESPTelnet/blob/main/src/ESPTelnetBase.h#L18C2-L18C32
 
#endif

#if defined(WITH_WEB)
  #include <WiFi.h>
  
#endif

#include "wifi_utils.h" // used in WEB or for TELNET


#if defined(WITH_BLUETOOTH)
  #include "BluetoothSerial.h"
#endif




#include "TrackerMaster.h"
#include "GpsService.h"


//---------------------------------------------
// Struct time  defined in TrackerMaster.h
//---------------------------------------------

struct time NowDT={2020,12,10,12,0,0}; //6*4=24 year, month, day, hh, mm, ss;
//byte * ptNowDT = (byte*) &NowDT;


void printDateTime(struct time & t){
   //Serial.printf("nowDT(t): %2d \r\n",sizeof(nowDT));

   Serial.printf(" Date: %4d/%02d/%02d, Time: %02d:%02d:%02d \r\n",t.year,t.month,t.day,t.hh,t.mm,t.ss); 
}

//---------------------------------------------
// GPS Service Origin, MODE can be: MASTER OR SLAVE
// set it on configuration.h
//---------------------------------------------
#if defined(GPS_MODE_MASTER)
  GpsService gpsService(MASTER);
#endif

#if defined(GPS_MODE_SLAVE)  
  GpsService gpsService(SLAVE);
#endif

//---------------------------------------------

 
Tracker tracker;// without ()      
//Tracker * ptTracker =  &tracker;

#if defined(WITH_TELNET)
  #include "telnet_utils.h" //needs tracker
#endif

#if defined(WITH_BLUETOOTH)
  #include "bluetooth_utils.h"   
#endif
//--------------------------------------------
// JSON code
//--------------------------------------------

//"lblSatsInUse":"vSatsInUSe",

String jsonVarsTemplate =  R"=====({"action":"getVars",
    "lblState":"vState","lblDate":"vDate",
    "lblTime":"vTime","lblLon":"vLon","lblLat":"vLat","lblelev":"vElev",   
    "lblEndE":"vEndE","lblEndA":"vEndA","lblResolution":"vResolution",
    "lblEndESteps":"vStepsLooseE",
    "lblEndASteps":"vStepsLooseA",
    "lblShiftA":vShiftA,"lblShiftE":vShiftE,    
    "lblTargetA":vTargetA,"lblTargetE":vTargetE,    
    "lblMode":"vMode",    
    "Sa":vSunA,"Ta":vTargetA,"Ma":vMirrorA,
    "Se":vSunE,"Te":vTargetE,"Me":vMirrorE,
    
    })=====";


String jsonVarsBuffer;


//---------------------------------------------
// time interval
//---------------------------------------------
  // Update position interval
  // Sun speed in: degrees/min
  // 360/(24hs*60min)=0.25 degrees/min, 4 min for 1 degress.
  unsigned long prevTime = 0;
  unsigned long nowTime = 0;   

//---------------------------------------------
//============
int ledPin = 2;//2 Lolin; //13 Arduino MEGA,
void setup() {

    //--------------------------------------
    // Attempt to connect wifi
    // Setup telnet
    //--------------------------------------
  
    Serial.begin(9600); 
    Wire.begin(); /* join i2c bus with Verde SDA 20=D1, Azul SCL 21=D2 of NodeMCU 8266 */
    Serial.println("Begin setup:------------");   
    #if defined(WITH_WEB) || defined(WITH_TELNET)
        
        bool connected= connectToWiFi(WIFI_SERVICE_NAME,PASSWORD);//I in wifi_utils.h -->is shared between Telnet and WebFrontEnd

        #if defined(WITH_TELNET)       
        if (connected) setupTelnet();  
        #endif        
    #endif
    //--------------------------------------
    // Setup Bluetooth
    //--------------------------------------   
      
    #if defined(WITH_BLUETOOTH)
      setupBluetooth("BT_SunTracker");      
    #endif
    
    //--------------------------------------
    // GPS Parameters
    //--------------------------------------  
      
    int isReady=0, inView=0, inUse=0;    
    Serial.println("Waiting for gps:");  
          
    for (int i=0; i <20 ;i++)
    {       
      gpsService.getStatus(isReady,inView,inUse);      
      Serial.printf(" getStatus, at: %d second, is ready: %d \n",i,isReady);  
      if(isReady==1) break;
      delay(1000);
    }

      //--------------------------------------
      // Init the Tracker
      //--------------------------------------  

      // May be check isI2CReady()
      tracker.setModeTracker();//105     
      tracker.setWakeupSleep(START_HOUR,STOP_HOUR);
      tracker.goEndA();
      tracker.goEndE();

      //-------------------------------------
      //Start only if Gps is Ready
      //-------------------------------------
      if(isReady){
        //Serial.println(" getLocation:");     
        float longitude=0,latitude=0,elevation=0;     
        gpsService.getLocation(longitude,latitude,elevation);   
        //Serial.printf(" Setup longitude: %02.4f latitude: %02.4f elevation: %02.4f \n",longitude,latitude,elevation);      
        tracker.setLocation(longitude,latitude,elevation,TIMEZONE);//Longitude,Latitude,Altitude,Timezone  
        // For security begin in STOP mode
        //tracker.setStateStart();
      }     
      //--------------------------------------  

      

    
    Serial.println("End setup:------------");   
    printMenu();
         

    //https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/Esp.h
    /*
    Serial.printf("FlashChipSize: %d \n", ESP.getFlashChipSize());
    Serial.printf("SketchSize: %d \n", ESP.getSketchSize());
    Serial.printf("FreeSketchSpace: %d\n", ESP.getFreeSketchSpace());
    */
  
}

void loop() { 
    //---------------------------------------------
    // Telnet service
    //---------------------------------------------  
     
     #if defined(WITH_TELNET)
      telnet.loop();
     #endif
     
    //---------------------------------------------
    // Read Serial Command 
    //---------------------------------------------
    if (Serial.available())
    {    
      int cod = 0;
      float v1=0,v2=0;
      parseSerial(cod,v1,v2);
      Serial.print("cod:");
      Serial.println(cod);      
      if (cod > 0)  SerialCommand(cod,v1,v2);  

    }


     //---------------------------------------------  
     // Retrieve data from GPS
       gpsService.retrieve();     
     //---------------------------------------------  
     

      
     //---------------------------------------------     
     // Update every updateInterval
     
      nowTime = millis();
      int isReady=0, inView=0, inUse=0; 
      if ((nowTime-prevTime) > 30000)// 0 seconds
      {
        prevTime = nowTime;
        //Serial.println("updateInterval");
        //Serial.println(".");
        //Serial.printf("%02d:%02d update \n",(millis()/60000) %60,millis()/1000%60);       
        //populateJsonVars();
        //Serial.println(jsonVarsBuffer);


        //-------------------------
        // Main Update
        //-------------------------

        gpsService.getStatus(isReady,inView,inUse);  
        if(isReady){ //only if gps is ready
          
          // Update location again
          // sometime GPS take a while to get a precise location an elevation
          float longitude=0,latitude=0,elevation=0;     
          gpsService.getLocation(longitude,latitude,elevation);   
          //Serial.printf(" Setup longitude: %02.4f latitude: %02.4f elevation: %02.4f \n",longitude,latitude,elevation);      
          tracker.setLocation(longitude,latitude,elevation,TIMEZONE);//Longitude,Latitude,Altitude,Timezone  

          // Update NowDT
          gpsService.getUtcDateTime((byte*)&NowDT);
          //printDateTime(NowDT);        
          tracker.printLastSPA();
          tracker.mainUpdate( NowDT); 
          }
        else
        {  
          Serial.println("WARNING: GPS is not Ready impossible to move");
          
          #if defined(WITH_BLUETOOTH) 
          SerialBT.printf("WARNING: GPS is not Ready impossible to move");
          #endif 
          
        }
        
        #if defined(WITH_BLUETOOTH) 
         SerialBT.printf("Loop: %02d:%02d:%02d mainUpdate(NowDt) \n",NowDT.hh,NowDT.mm,NowDT.ss);
        #endif  
        
        
        } 
     
    
        

      
 
}

//-----------------------------------------------
//  Serial command parser
//-----------------------------------------------
void parseSerial(int  &cod,float & out1, float & out2){

    char commandBuffer[20]={NULL};         
    //int bytesCount = Serial.readBytesUntil('\n',commandBuffer,20);
     Serial.readBytesUntil('\n',commandBuffer,20);
    //Serial.println("-----------------------------------");    
    //Serial.print("bytesCount:");Serial.println(bytesCount);
    //Serial.print("Buffer:");Serial.println(commandBuffer);          
    //cod
    char * token = strtok(commandBuffer, ","); // not use char ','       
    cod = token==NULL ? 0 : atoi(token);   
    //out1      
    token = strtok(NULL,",");       
    //Serial.print("token: ");Serial.println(token);
    out1 = (token==NULL) ? 0 : atof(token);   
    //out2
    token = strtok(NULL,",");      
    //Serial.print("token: ");Serial.println(token);
    out2 = token==NULL ? 0 : atof(token);
        
}
//============
void printMenu(){

        Serial.print("Local IP: ");Serial.println(WiFi.localIP());
        
        Serial.printf("START   : %d \n",START);               
        Serial.printf("STOP    : %d \n",STOP); 
        Serial.printf("GO_END_A: %d \n",GO_END_A);
        Serial.printf("GO_END_E: %d \n",GO_END_E); 
        Serial.printf("GO_PARK : %d \n",GO_PARK);
        Serial.printf("MOVE_UP : %d,value\n",MOVE_UP);               
        Serial.printf("MOVE_DN : %d,value\n",MOVE_DN);
        Serial.printf("MOVE_LF : %d,value\n",MOVE_LF);
        Serial.printf("MOVE_RG : %d,value\n",MOVE_RG);        

        
        Serial.printf("Set sun position to: %d,value,value\n",UPD_SUN_POSITION);// Move to specific sun position
        Serial.printf("Slave Led On    : %d,value,value\n",LED_ON);
        Serial.printf("Slave Led Off   : %d,value,value\n",LED_OFF);        
        Serial.printf("SET_PARK        : %d,value,value\n",SET_PARK);
        Serial.printf("SET_SHIFT       : %d,value,value\n",SET_SHIFT);
        Serial.printf("SET_TARGET      : %d,value,value\n",SET_TARGET);        
        Serial.printf("SET_MODE_MIRROR : %d \n",SET_MODE_MIRROR);
        Serial.printf("SET_MODE_TRACKER: %d \n",SET_MODE_TRACKER);        
     
        
        Serial.println("50: Master vars"); 
        Serial.println("51: Slave vars");                 
        Serial.println("52: GPS vars");
        Serial.println("53: Call MainUpdate(time now)");
        Serial.println("60: Calculate SPA");        
        Serial.println("61: Print & Update utcDateTime");
        Serial.println("62: TestTracking");
        Serial.println("63: Set Slave Vars");         
        
        Serial.println("99: Print Menu"); 
  
  }

//----------------------------------
// Call command from serial 
//----------------------------------
void SerialCommand(int cod, float v1, float v2  ) {        
  
  

    Serial.printf("Master, command-----------------------:%d \n",cod);
    // Warning allway read 0, so start from 1
    switch (cod)
    {
        case START: 
        Serial.println("Start");tracker.setStateStart(); // only in Master       
        break;

        case STOP:
        Serial.println("Stop"); tracker.setStateStop(); // only in Master              
        break;
    
        case GO_END_A:
        Serial.println("goEndA"); tracker.goEndA();  
        break; 
        
        case GO_PARK:
        Serial.println("GO_PARK"); tracker.goPark(); 
        break;

        case GO_END_E:
        Serial.println("goEndE"); tracker.goEndE();  
        break;
     
         case MOVE_UP: 
        Serial.printf("MOVE_UP by: %02.2f (degrees) \n",v1); tracker.moveUP(v1);
        break;

        case MOVE_DN:        
        Serial.printf("MOVE_DN by: %02.2f (degrees) \n",v1); tracker.moveDN(v1);
        break;

        case MOVE_LF:         
        Serial.printf("MOVE_LF by: %02.2f (degrees) \n",v1); tracker.moveLF(v1);
        break;

        case MOVE_RG:
        Serial.printf("MOVE_RG by:  %02.2f (degrees) \n",v1); tracker.moveRG(v1);        
        break;


        case UPD_SUN_POSITION:
        Serial.printf("Set sun position to:  A: %02.2f, E: %02.2f \n",v1,v2); tracker.updSunPosition(v1,v2);//Azimut,Elevation
        break;

        case LED_ON:
        Serial.printf("LED_ON: bool: %d return: \n", sendCommand(LED_ON,0));   
        break; 
        case LED_OFF:
        Serial.printf("LED_OFF: bool: %d return: \n", sendCommand(LED_OFF,0));   
        break; 


        case SET_PARK:
        Serial.printf("SET_PARK to: A: %02.2f, E: %02.2f \n",v1,v2);tracker.setParking(v1,v2);
        break;  
        
        case SET_SHIFT:
        Serial.printf("SET_SHIFT to: A: %02.2f, E: %02.2f \n",v1,v2); tracker.setShift(v1,v2);
        break;  
        
        case SET_TARGET:
        Serial.printf("SET_TARGET to: A: %02.2f, E: %02.2f \n",v1,v2); tracker.setTarget(v1,v2);
        break;  

        case SET_MODE_MIRROR:  
        Serial.println("SET_MODE_MIRROR"); tracker.setModeMirror();
        break;

        case SET_MODE_TRACKER:  
        Serial.println("SET_MODE_TRACKER");  tracker.setModeTracker();
        break;


       
        break;

        case 50:
        Serial.println("Print Master vars"); tracker.printMasterVars();
        break;
        
        case 51:
        Serial.println("Print Slave vars");  tracker.printSlaveVars();
        break;
        

        case 52:{                
              Serial.println("Print GPS info");  gpsService.printVars();//
              float longitude=0,latitude=0,elevation=0;     
              gpsService.getLocation(longitude,latitude,elevation);   
              tracker.setLocation(longitude,latitude,elevation,TIMEZONE);//Longitude,Latitude,Altitude,Timezone  
        }
        break;

        case 53:{
            Serial.println("Call mainUpdate(NowDT)");  
            gpsService.getUtcDateTime((byte*)&NowDT);
            tracker.mainUpdate( NowDT); 
        }
        break;


   
   


        case 60:
        Serial.println("Calculate SPA---------"); 
        
        gpsService.getUtcDateTime((byte*)&NowDT);//update NowDt from gps in slave
        //printNowDT();               
        tracker.printCalculatedSPA(NowDT);
        break;
        

        // Run some Test

        case 61:        
        Serial.println("updDateTime ");  printUtcDateTime();//
        break;

        case 62:
        Serial.println("Simulate tracking");  simulateTracking();//
        break;

        case 63:
        Serial.println("Set Slave Vars"); tracker.testSetters();
        break;




        case 70:
        Serial.println("pVars");
        populateJsonVars();
        Serial.println(jsonVarsBuffer);

        

        case 99:
        printMenu();
        break;        

    
    }      
      
    

}

/* ------------------------------------------------- */

void populateJsonVars() 
{  
 

    
    jsonVarsBuffer = String(jsonVarsTemplate); 
    //jsonVarsBuffer.replace("vAction","getVars");
    jsonVarsBuffer.replace("vState",tracker.getStateStr());
    char dateBuffer[10]={0};
    sprintf(dateBuffer,"%4d/%02d/%02d",NowDT.year,NowDT.month,NowDT.day);    
    jsonVarsBuffer.replace("vDate",dateBuffer);
    sprintf(dateBuffer,"%02d:%02d:%02d", NowDT.hh,NowDT.mm,NowDT.ss); 
    jsonVarsBuffer.replace("vTime",dateBuffer) ;


    jsonVarsBuffer.replace("vTimeZone",String(tracker.getTimezone())) ;
    jsonVarsBuffer.replace("vTimeStart",String(tracker.getStartHour()));
    jsonVarsBuffer.replace("vTimeStop",String(tracker.getStopHour()));
    
    jsonVarsBuffer.replace("vLon",String(tracker.getLongitude(),2));
    jsonVarsBuffer.replace("vLat",String(tracker.getLatitude(),2));
    jsonVarsBuffer.replace("vElv",String(tracker.getElevation(),2));
    
    double dA =0;
    double dE =0;
    
    // tracker.getLastSPA(outA,outE);//get the last calculated, to avoid calculate 2 times
    // si lo leo desde Tracker la posicion solo se actualiza si esta en Tracking, es decir entre las horas de trabajo
    // y para actualizarlo tendria que pasarlo a start
    // pero evito mostrar posiciones negativas por la noche     
    
    jsonVarsBuffer.replace("vSa",String(dA,2));// o desde Tracker que pasa de noche 
    jsonVarsBuffer.replace("vSe",String(dE,2));

    //jsonVarsBuffer.replace("vSa",String(tracker.getSunA(),3));// o desde Tracker que pasa de noche 
    //jsonVarsBuffer.replace("vSe",String(tracker.getSunE(),3));

    int endA =0;
    int endE =0;
    // TODO procesar retorno true false

    tracker.getEndstops(endA,endE);
    jsonVarsBuffer.replace("vEndA",endA==0?"open":"closed");
    jsonVarsBuffer.replace("vEndE",endE==0?"open":"closed");
    jsonVarsBuffer.replace("vStepsLooseA","N/A");
    jsonVarsBuffer.replace("vStepsLooseE","N/A");
    
    float A=0,E=0;
    tracker.getPark(A,E);
    jsonVarsBuffer.replace("vParkA",String(A,2));
    jsonVarsBuffer.replace("vParkE",String(E,2));


    tracker.getTarget(A,E); // TODO procesar retorno true false 
    jsonVarsBuffer.replace("vTargetA",String(A,2));
    jsonVarsBuffer.replace("vTargetE",String(E,2));

    tracker.getSystemPosition(A,E);    
    jsonVarsBuffer.replace("vMirrorA",String(A,2));
    jsonVarsBuffer.replace("vMirrorE",String(E,2));

    tracker.getShift(A,E);
    jsonVarsBuffer.replace("vShiftA",String(A,2));   
    jsonVarsBuffer.replace("vShiftE",String(E,2)); 
        
    jsonVarsBuffer.replace("vMode",tracker.getStateStr());   

     
}
    

void printUtcDateTime() 
{
 
   gpsService.getUtcDateTime((byte*)&NowDT);   
   Serial.printf(" Struct NowDT Date: %4d/%02d/%02d, Time: %02d:%02d:%02d utc London\r\n",NowDT.year,NowDT.month,NowDT.day,NowDT.hh,NowDT.mm,NowDT.ss); 
}

void simulateTracking(){
      tracker.setStateStart();
      struct time fakeDT = {2023,10,18,0,0,0};
      printDateTime(fakeDT);
      
      for (int i=6;i<20;i++){     //UTC time  
  
        fakeDT.hh =i;    
        fakeDT.mm = 0;          
        printDateTime(fakeDT);
        tracker.mainUpdate( fakeDT); 
        // print position getPosition(longitude,latitude)
                      

        //fakeDT.mm = 30;       
        //printDateTime(fakeDT);
        //tracker.mainUpdate( fakeDT); 
        //delay(500);                 
        }
}
