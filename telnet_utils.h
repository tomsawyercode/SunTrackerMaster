#if defined(WITH_TELNET)
#include "commands.h"
 
 
 //https://github.com/LennartHennigs/ESPTelnet

ESPTelnet telnet;

//Tracker * ptTracker
//void (*) function_callback  = void (*)(char*); //alias function pointer lo use para llamar a serial commadn

//https://isocpp.org/wiki/faq/pointers-to-members#typedef-for-ptr-to-memfn
//https://isocpp.org/wiki/faq/pointers-to-members#fnptr-vs-memfnptr-types

//typedef size_t  (ESPTelnet::*typefpointer)(const char* format, ...);
 //typefpointer fp = &ESPTelnet::printf;

//https://www.cs.cmu.edu/~guna/15-123S11/Lectures/Lecture14.pdf


// size_t  (*fpointer)(const char* format, ...);
/* ------------------------------------------------- */
/* ------------------------------------------------- */

//-----------------------------------------------
//  Serial command parser
//-----------------------------------------------
void parseTelnet( String command, int  & cod,float & out1, float & out2){

    // https://www.arduino.cc/reference/en/language/variables/data-types/stringobject/
      
    //Serial.print("command:");Serial.println(command);      
    
    char commandBuffer [20];
    strcpy (commandBuffer, command.c_str ());    
    
    char * token = strtok(commandBuffer, ","); // not use char ','       
    cod = token==NULL ? 0 : atoi(token);   
    //Serial.print("token 0: ");Serial.println(token);
    //out1      
    token = strtok(NULL,",");       
    //Serial.print("token 1: ");Serial.println(token);
    out1 = (token==NULL) ? 0 : atof(token);   
    //out2
    token = strtok(NULL,",");      
    //Serial.print("token 2: ");Serial.println(token);
    out2 = token==NULL ? 0 : atof(token);
        
}
  

/* ------------------------------------------------- */

void telnetMenu(){
        
        telnet.println("100  to disconnect.");
        telnet.printf("START   : %d\n",START);               
        telnet.printf("STOP    : %d\n",STOP);         
        telnet.printf("GO_END_A: %d\n",GO_END_A);
        telnet.printf("GO_END_E: %d\n",GO_END_E);
        telnet.printf("GO_PARK : %d\n",GO_PARK);
        telnet.printf("MOVE_UP : %d,d\n",MOVE_UP);               
        telnet.printf("MOVE_DN : %d,d\n",MOVE_DN);
        telnet.printf("MOVE_LF : %d,d\n",MOVE_LF);
        telnet.printf("MOVE_RG : %d,d\n",MOVE_RG);
      

        telnet.printf("Set sun position: %d,a,e\n",UPD_SUN_POSITION);
        telnet.printf("Led On          : %d\n",LED_ON);
        telnet.printf("Led Off         : %d\n",LED_OFF);        
        telnet.printf("SET_PARK        : %d,a,e\n",SET_PARK);
        telnet.printf("SET_SHIFT       : %d,a,e\n",SET_SHIFT);
        telnet.printf("SET_TARGET      : %d,a,e\n",SET_TARGET);
        telnet.printf("SET_MODE_MIRROR : %d\n",SET_MODE_MIRROR);
        telnet.printf("SET_MODE_TRACKER: %d\n",SET_MODE_TRACKER);   
                     
        telnet.println("50: Master vars ");         
        telnet.println("51: Slave vars"); 
        telnet.println("52: GPS vars");       
        telnet.println("60: Calculate Sun Position SPA");         
        telnet.println("99: Menu"); 
        telnet.println("100: Disconnect");




}  
void printMasterVars(){
  
  telnet.printf(" State: %s\n", tracker.getStateStr());
  telnet.printf(" Location: lon: %02.4f, lat: %02.4f, elev: %02.4f\n",tracker.getLongitude(),tracker.getLatitude(),tracker.getElevation());  
  telnet.printf(" Time settings: timezone: %d, start: %d hs, stop: %d hs\n", tracker.getTimezone(), tracker.getStartHour(),tracker.getStopHour());
  telnet.printf(" Last calculated sun position:\n  A: %02.2f \n  E: %02.2f  \n", tracker.getLastSunAzimuth(), tracker.getLastSunElevation());  

}
void printSlaveVars(){  
  telnet.printf(" ------------------:\n");
  int i1=10,i2=10;
  tracker.getEndstops(i1,i2);
  telnet.printf(" Endstops, A: %d, E: %d\n" ,i1,i2);
    
  telnet.printf(" Mode: %s, %d\n" , tracker.getModeStr(), tracker.getMode());
  
  float f1=0.0,f2=0.0;
  
  tracker.getSystemPosition(f1,f2);
  telnet.printf(" Position, A: %02.2f, E: %02.2f\n" ,f1,f2); 
  
  tracker.getShift(f1,f2);
  telnet.printf(" Shift, A: %02.2f, E: %02.2f\n" ,f1,f2);
  tracker.getTarget(f1,f2);
  telnet.printf(" Target, A: %02.2f, E: %02.2f\n" ,f1,f2);
  tracker.getPark(f1,f2);
  telnet.printf(" Park, A: %02.2f, E: %02.2f\n" ,f1,f2);
  



 
  }
void printGps(){
     
     telnet.printf("GPS origin: %s\n",gpsService.getOrigin()==0?"M":"S"); 
     int isValid=0, inView=0, inUse=0; 
     gpsService.getStatus(isValid,inView,inUse);   
     telnet.printf(" Status: valid: %d, S in view: %d, S in use: %d\n" ,isValid, inView, inUse);
     float longitude=0,latitude=0,elevation=0;     
     gpsService.getLocation(longitude,latitude,elevation);   
     telnet.printf(" Loc: lon: %02.4f, lat: %02.4f, ele: %02.4f\n",longitude,latitude,elevation);     
     telnet.printf(" Update location to :\n  longitude: %02.5f\n  latitude: %02.5f\n  elevation: %02.2f \n",longitude,latitude,elevation); 
     gpsService.getUtcDateTime((byte*)&NowDT);   
     telnet.printf(" Date: %4d/%02d/%02d\n Time: %02d:%02d:%02d utc London\r\n",NowDT.year,NowDT.month,NowDT.day,NowDT.hh,NowDT.mm,NowDT.ss); 

       
 
  }

void printSPA(){  

   
   gpsService.getUtcDateTime((byte*)&NowDT);   
   telnet.printf(" Date: %4d/%02d/%02d, Time: %02d:%02d:%02d utc London\r\n",NowDT.year,NowDT.month,NowDT.day,NowDT.hh,NowDT.mm,NowDT.ss); 
   int result;
   double outA,outE;
   result = tracker.SPA(NowDT,outA,outE);          
   telnet.printf(" Sun E: %.2f ,A: %.2f, res:%s\n",outA,outE, result==0? "OK":"FAIL");
  
  }
//----------------------------------
// Call command from Telnet 
//----------------------------------
void TelnetCommand(int cod, float v1, float v2  ) {        
    

    Serial.printf("Telnet, -------------:%d \n",cod);
    // Warning allway read 0, so start from 1
    switch (cod)
    {
        case START: 
        telnet.println("Start");tracker.setStateStart(); // only in Master       
        break;

        case STOP:
        telnet.println("Stop"); tracker.setStateStop(); // only in Master              
        break;
  
        
        case GO_END_A:
        telnet.println("goEndA"); tracker.goEndA();  
        break;

        case GO_END_E:
        telnet.println("goEndE"); tracker.goEndE();  
        break;

        case GO_PARK:
        telnet.println("GO_PARK"); tracker.goPark(); 
        break;
     
        case MOVE_UP: 
        telnet.printf("MOVE_UP by: %02.2f (degrees) \n",v1); tracker.moveUP(v1);
        break;

        case MOVE_DN:        
        telnet.printf("MOVE_DN by: %02.2f (degrees) \n",v1); tracker.moveDN(v1);
        break;

        case MOVE_LF:         
        telnet.printf("MOVE_LF by: %02.2f (degrees)\n",v1); tracker.moveLF(v1);
        break;

        case MOVE_RG:
        telnet.printf("MOVE_RG by:  %02.2f (degrees)\n",v1); tracker.moveRG(v1);        
        break;

  
        case UPD_SUN_POSITION:
        telnet.printf("Set sun position to: A: %02.2f, E: %02.2f\n",v1,v2); tracker.updSunPosition(v1,v2);//Azimut,Elevation
        break;

        case LED_ON:
        telnet.printf("LED_ON: bool: %d return:\n", sendCommand(LED_ON,0));   
        break; 
        case LED_OFF:
        telnet.printf("LED_OFF: bool: %d return:\n", sendCommand(LED_OFF,0));   
        break; 


        case SET_PARK:
        telnet.printf("SET_PARK: A: %02.2f, E: %02.2f\n",v1,v2);tracker.setParking(v1,v2);
        break;  
        
        case SET_SHIFT:
        telnet.printf("SET_SHIFT: A: %02.2f, E: %02.2f\n",v1,v2); tracker.setShift(v1,v2);
        break;  
        
        case SET_TARGET:
        telnet.printf("SET_TARGET: A: %02.2f, E: %02.2f\n",v1,v2); tracker.setTarget(v1,v2);
        break;  

        case SET_MODE_MIRROR:  
        telnet.println("SET_MODE_MIRROR"); tracker.setModeMirror();
        break;

        case SET_MODE_TRACKER:  
        telnet.println("SET_MODE_TRACKER");  tracker.setModeTracker();
        break;

        case 50:{
        telnet.println("Master"); printMasterVars();
        }
        break;        
        
        case 51:{
        telnet.println("Slave"); printSlaveVars();
        }
        break;        

        case 52:
        telnet.println("GPS");  printGps();
        break;        
        
        case 60:
        telnet.println("SPA");                
        printSPA();
        break;

        case 99:
        telnetMenu();
        break;        

    
    }      
      
    

}

// (optional) callback functions for telnet events
void onTelnetConnect(String ip) {
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.println(" connected");
  
  telnet.println("\nWelcome " + telnet.getIP()); 
  telnetMenu();
}

void onTelnetDisconnect(String ip) {
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.println(" disconnected");
}

void onTelnetReconnect(String ip) {
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.println(" reconnected");
}

void onTelnetConnectionAttempt(String ip) {
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.println(" tried to connected");
}

void onTelnetInput(String str) {
  // checks for a received command
  
  Serial.print("Telnet Received");Serial.println(str);
  telnet.print("Telnet Received: ");telnet.println(str); 
   
  int cod = 0;
  float v1=0,v2=0;

  parseTelnet(str,cod,v1,v2);   
    if (cod == 100) {
    telnet.println("> disconnecting you...");
    telnet.disconnectClient(); // disconnect the client
    return;
    }
 
  
  TelnetCommand(cod,v1,v2);
  

  }


/* ------------------------------------------------- */
// Setup Telnet
// ------------------------------------

void setupTelnet(int port=23) {  
  // passing on functions for various telnet events
  telnet.onConnect(onTelnetConnect);
  telnet.onConnectionAttempt(onTelnetConnectionAttempt);
  telnet.onReconnect(onTelnetReconnect);
  telnet.onDisconnect(onTelnetDisconnect);
  telnet.onInputReceived(onTelnetInput);

  Serial.print("Telnet state: ");
  if (telnet.begin(port)) {
    Serial.println("running");
  } else {
    Serial.println("error: Will reboot...");
    //delay(2000);    ESP.restart();    delay(2000);
  }
}

#endif
