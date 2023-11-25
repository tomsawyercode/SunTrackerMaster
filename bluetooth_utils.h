#if defined(WITH_BLUETOOTH)


BluetoothSerial SerialBT;


//-----------------------------------------------
//  BLuetooth command parser
//-----------------------------------------------
void parseBluetooth( char * command, int  & cod,float & out1, float & out2){

          
    //Serial.print("command:");Serial.println(command);      
    
    char commandBuffer [20];
    strcpy (commandBuffer, command);    
    
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
void BluetoothMenu(){
        
        
        
        SerialBT.printf("START   : %d \n",START);               
        SerialBT.printf("STOP    : %d \n",STOP);         
        SerialBT.printf("GO_END_A: %d \n",GO_END_A);
        SerialBT.printf("GO_END_E: %d \n",GO_END_E);
        SerialBT.printf("GO_PARK : %d \n",GO_PARK);
        SerialBT.printf("MOVE_UP : %d,degres\n",MOVE_UP);               
        SerialBT.printf("MOVE_DN : %d,degres\n",MOVE_DN);
        SerialBT.printf("MOVE_LF : %d,degres\n",MOVE_LF);
        SerialBT.printf("MOVE_RG : %d,degres\n",MOVE_RG);
        SerialBT.printf("Set sun position: %d,a,e \n",UPD_SUN_POSITION);
        SerialBT.printf("Led On          : %d\n",LED_ON);
        SerialBT.printf("Led Off         : %d\n",LED_OFF);        
        SerialBT.printf("SET_PARK        : %d,a,e \n",SET_PARK);
        SerialBT.printf("SET_SHIFT       : %d,a,e \n",SET_SHIFT);
        SerialBT.printf("SET_TARGET      : %d,a,e \n",SET_TARGET);
        SerialBT.printf("SET_MODE_MIRROR : %d \n",SET_MODE_MIRROR);
        SerialBT.printf("SET_MODE_TRACKER: %d \n",SET_MODE_TRACKER);     

        SerialBT.println("50: Master vars"); 
        SerialBT.println("51: Slave vars");                 
        SerialBT.println("52: GPS vars");
        SerialBT.println("53: Call MainUpdate(time now)");
        SerialBT.println("60: Calculate SPA");        
        SerialBT.println("99: Print Menu"); 
        SerialBT.println("100: Disconnect"); 
       
}

void printBtMasterVars(){
  //SerialBT.printf(" ----------------:\n");
  SerialBT.printf(" State: %s\n", tracker.getStateStr());
  SerialBT.printf(" Location: \n  longitude: %02.4f\n  latitude: %02.4f\n  elevation: %02.2f \n",tracker.getLongitude(),tracker.getLatitude(),tracker.getElevation());  
  SerialBT.printf(" Time settings:\n  timezone: %d\n  start: %d hs\n  stop: %d hs  \n", tracker.getTimezone(), tracker.getStartHour(),tracker.getStopHour());
  SerialBT.printf(" Last calculated sun position:\n  A: %02.2f \n  E: %02.2f  \n", tracker.getLastSunAzimuth(), tracker.getLastSunElevation());  

}
void printBtSlaveVars(){  
  //SerialBT.printf(" ------------------:\n");
  int i1=10,i2=10;
  tracker.getEndstops(i1,i2);
  SerialBT.printf(" Endstops, A: %d, E: %d \n" ,i1,i2);
    
  SerialBT.printf(" Mode: %s, %d \n" , tracker.getModeStr(), tracker.getMode());
  
  float f1=0.0,f2=0.0;
  
  tracker.getSystemPosition(f1,f2);
  SerialBT.printf(" System from ZERO,  A: %02.2f, E: %02.2f \n" ,f1,f2); 
  
  tracker.getShift(f1,f2);
  SerialBT.printf(" Shift, A: %02.2f, E: %02.2f \n" ,f1,f2);
  tracker.getTarget(f1,f2);
  SerialBT.printf(" Target, A: %02.2f, E: %02.2f \n" ,f1,f2);
  tracker.getPark(f1,f2);
  SerialBT.printf(" Park, A: %02.2f, E: %02.2f \n" ,f1,f2);
  
 
  }
void printBtGps(){
     
     SerialBT.printf(" GPS origin: %s\n",gpsService.getOrigin()==0?"Master":"Slave"); 
     int isValid=0, inView=0, inUse=0; 
     gpsService.getStatus(isValid,inView,inUse);   
     SerialBT.printf(" Status: \n  Is valid: %d\n  Sats in view: %d\n  Sats in use: %d  \n" ,isValid, inView, inUse);
     float longitude=0,latitude=0,elevation=0;     
     gpsService.getLocation(longitude,latitude,elevation);   
     SerialBT.printf(" Update location to :\n  longitude: %02.5f\n  latitude: %02.5f\n  elevation: %02.2f \n",longitude,latitude,elevation);      
     SerialBT.printf(" Date: %4d/%02d/%02d\n Time: %02d:%02d:%02d utc London\r\n",NowDT.year,NowDT.month,NowDT.day,NowDT.hh,NowDT.mm,NowDT.ss); 
 
  }

void mainUpdate(){
   
   gpsService.getUtcDateTime((byte*)&NowDT);
   tracker.mainUpdate( NowDT); 
  
}

void calculateAndPrintBtSPA(){  

   
   gpsService.getUtcDateTime((byte*)&NowDT);   
   SerialBT.printf(" UtcDateTime\n  Date: %4d/%02d/%02d\n  Time: %02d:%02d:%02d\n",NowDT.year,NowDT.month,NowDT.day,NowDT.hh,NowDT.mm,NowDT.ss); 
   int result;
   double outA,outE;
   result = tracker.SPA(NowDT,outA,outE);          
   SerialBT.printf(" Calculated Sun position: \n  A: %.2f \n  E: %.2f\n  Result:%s \n",outA,outE, result==0? "OK":"FAIL");
   
   SerialBT.printf("\n https://www.suncalc.org/#/%.6f,%.6f,16/%d.%d.%d/%d:%d/1/3 \n\n",tracker.getLatitude(), tracker.getLongitude(),NowDT.year,NowDT.month,NowDT.day,NowDT.hh+_timezone,NowDT.mm);


  
  }

//----------------------------------
// Call command from Telnet 
//----------------------------------
void BluetoothCommand(int cod, float v1, float v2  ) {        
    

    Serial.printf("Bluetooth, command-----------------:%d, %f, %f \n",cod,v1,v2);
    // Warning allway read 0, so start from 1
    switch (cod)
    {
        case START: 
        SerialBT.println("Start");tracker.setStateStart(); // only in Master       
        break;

        case STOP:
        SerialBT.println("Stop"); tracker.setStateStop(); // only in Master              
        break;
  
        
        case GO_END_A:
        SerialBT.println("goEndA"); tracker.goEndA();  
        break;

        case GO_END_E:
        SerialBT.println("goEndE"); tracker.goEndE();  
        break;

        case GO_PARK:
        SerialBT.println("GO_PARK"); tracker.goPark(); 
        break;
     
        case MOVE_UP: 
        SerialBT.printf("MOVE_UP by: %02.2f (degrees) \n",v1); tracker.moveUP(v1);
        break;

        case MOVE_DN:        
        SerialBT.printf("MOVE_DN by: %02.2f (degrees) \n",v1); tracker.moveDN(v1);
        break;

        case MOVE_LF:         
        SerialBT.printf("MOVE_LF by: %02.2f (degrees) \n",v1); tracker.moveLF(v1);
        break;

        case MOVE_RG:
        SerialBT.printf("MOVE_RG by:  %02.2f (degrees) \n",v1); tracker.moveRG(v1);        
        break;

  
        case UPD_SUN_POSITION:
        SerialBT.printf("Set sub position to:  A: %02.2f, E: %02.2f \n",v1,v2); tracker.updSunPosition(v1,v2);//Azimut,Elevation
        break;

        case LED_ON:
        SerialBT.printf("LED_ON: bool: %d return: \n", sendCommand(LED_ON,0));   
        break; 
        case LED_OFF:
        SerialBT.printf("LED_OFF: bool: %d return: \n", sendCommand(LED_OFF,0));   
        break; 


        case SET_PARK:
        SerialBT.printf("SET_PARK to: A: %02.2f, E: %02.2f \n",v1,v2);tracker.setParking(v1,v2);
        break;  
        
        case SET_SHIFT:
        SerialBT.printf("SET_SHIFT to: A: %02.2f, E: %02.2f \n",v1,v2); tracker.setShift(v1,v2);
        break;  
        
        case SET_TARGET:
        SerialBT.printf("SET_TARGET to: A: %02.2f, E: %02.2f \n",v1,v2); tracker.setTarget(v1,v2);
        break;  

        case SET_MODE_MIRROR:  
        SerialBT.println("SET_MODE_MIRROR"); tracker.setModeMirror();
        break;

        case SET_MODE_TRACKER:  
        SerialBT.println("SET_MODE_TRACKER");  tracker.setModeTracker();
        break;


        case 50:      
        SerialBT.println("Master Vars:"); printBtMasterVars();        
        break;        
        
        case 51:
        SerialBT.println("Slave Vars:"); printBtSlaveVars();
        break;        

        case 52:
              {
              SerialBT.println("GPS and Update (lon,lat):");  printBtGps();
         
              float longitude=0,latitude=0,elevation=0;     
              gpsService.getLocation(longitude,latitude,elevation);   
              tracker.setLocation(longitude,latitude,elevation,TIMEZONE);//Longitude,Latitude,Altitude,Timezone               
              }
              
        break;  

        case 53:
        SerialBT.println("Call mainUpdate(now) :"); mainUpdate();
        break;  
        
        case 60:
        SerialBT.println("Calculate SPA(now)");                
        calculateAndPrintBtSPA();
        break;

        case 99:
        BluetoothMenu();
        break;        

    
    }      
}


void onBluetoothData(const unsigned char * buf, unsigned int len) {
  
  // Checks for a received command
  
  //String str;
  Serial.printf("mainUpdate, Received len:%d \n",len);
  Serial.print("Text:");

  char * command = strtok((char*)buf, "\n");
  
  Serial.printf("command: %s \n",command);
  //aSerial.printf("String: %s \n",buf);
 
  /*
  char rx= SerialBT.read();
  Serial.print("Bluetooth Received");Serial.println(rx);
  SerialBT.print("Bluetooth Received: ");SerialBT.println(rx);
  */

      
  int cod = 0;
  float v1=0,v2=0;
  parseBluetooth(command,cod,v1,v2);        
  BluetoothCommand(cod,v1,v2);
  
  //if (cod=100) SerialBT.disconnect();//Test it end or  bool disconnect();
  
  }


void setupBluetooth(String device_name ){
  SerialBT.begin(device_name); //Bluetooth device name
  //SerialBT.deleteAllBondedDevices(); // Uncomment this to delete paired devices; Must be called after begin
  Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());

  SerialBT.onData(onBluetoothData);
  
  }


#endif  
