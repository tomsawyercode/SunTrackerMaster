#if defined(WITH_WEB) || defined(WITH_TELNET)

bool connectToWiFi(const char* ssid, const char* password, int timeout_minutes=1) {

  Serial.println("Connect to WiFi");    
 
  WiFi.mode(WIFI_STA);


  int start=millis();
    
  WiFi.begin(ssid, password);
  
   while (!(WiFi.status() == WL_CONNECTED))
   {
    delay(1000);
    Serial.printf("%02d:%02d \r\n",(millis()/60000) %60,millis()/1000%60);
  
    if ((millis()-start)> (timeout_minutes*60000) ) 
    {
      break;
    }    
   }
  
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  
  if ( WiFi.status() == WL_CONNECTED) {      
          Serial.print("  WiFi Connected: "); Serial.println(WiFi.localIP()); 
          return true;  
  }
  else
  {
         Serial.println("  Fail to Connect WIFI ");
         return false;
  }

  
  
}
#endif