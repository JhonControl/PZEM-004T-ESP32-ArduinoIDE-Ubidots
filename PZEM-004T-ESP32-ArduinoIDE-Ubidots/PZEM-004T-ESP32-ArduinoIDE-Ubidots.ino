/*************************************************************************************************
 * This Example sends harcoded data to Ubidots and serves as example for users that have devices
 * based on ESP8266 chips
 *
 * This example is given AS IT IS without any warranty
 *
 * Made by Jose García., adapted from the original WiFiClient ESP8266 example
 * 
 * Modified by PDAControl for ESP32 modules and PZEM Power Meters   
 *************************************************************************************************/

/********************************
 * Libraries included
 *******************************/

#include <WiFi.h>
#include <WiFiMulti.h>

#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

WiFiMulti WiFiMulti;
 
/********************************
 * Libraries PZEM-004 Meter
 *******************************/
#include <PZEM004T.h>                               // https://github.com/olehs/PZEM004T
PZEM004T pzem(&Serial2);                        //  UART2 at pins IO-16 (RX2) and IO-17 (TX2)
IPAddress ip(192,168,1,1);

/********************************
 * Internal Temperature sensor in ESP32
 *******************************/
#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();


/********************************
 * Constants and objects
 *******************************/

char const * SSID_NAME = "***********";                            // Put here your SSID name
char const * SSID_PASS = "***********";                               // Put here your password

char* TOKEN = "**************";                                             // Put here your TOKEN

char* DEVICE_LABEL = "*********************";         // Your Device label example=   esp32-arduino-pzem-004

/* Put here your variable's labels*/
char const * VARIABLE_LABEL_1 = "volts";
char const * VARIABLE_LABEL_2 = "amperes";
char const * VARIABLE_LABEL_3 = "power";
char const * VARIABLE_LABEL_4 = "energy";
char const * VARIABLE_LABEL_5 = "state_meter";
char const * VARIABLE_LABEL_6 = "rssi";
char const * VARIABLE_LABEL_7 = "temperature_internal";
char const * VARIABLE_LABEL_8 = "scan_data";

 float v_prev,i_prev,p_prev,e_prev; 
 float v_now,i_now,p_now,e_now; 
 long state_meter, scan_connection;

/* HTTP Settings */
char const * HTTPSERVER = "industrial.api.ubidots.com";
const int    HTTPPORT = 80;
char const * USER_AGENT = "ESP32";
char const * VERSION = "2.0";

WiFiClient clientUbi;

/********************************
 * Auxiliar Functions
 *******************************/

void ReconnectDevices() {

    while (true) {
      Serial.println("Connecting to PZEM...");  
      if(pzem.setAddress(ip))
        break;
      delay(1000);
   }
 
  /* Connects to AP */
   WiFiMulti.addAP(SSID_NAME, SSID_PASS);
 
    while(WiFiMulti.run() != WL_CONNECTED) {
        Serial.print(".");                                  
        delay(100);
    }

  WiFi.setAutoReconnect(true);
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());


  if (clientUbi.connect(HTTPSERVER, HTTPPORT)) {
    Serial.println("connected to Ubidots cloud");
  } else {
    Serial.println("could not connect to Ubidots cloud");
  }
}
 
void SendToUbidots(char* payload) {

  int contentLength = strlen(payload);

  /* Connecting the client */
  clientUbi.connect(HTTPSERVER, HTTPPORT);

  if (clientUbi.connected()) {
    /* Builds the request POST - Please reference this link to know all the request's structures https://ubidots.com/docs/api/ */
    clientUbi.print(F("POST /api/v1.6/devices/"));
    clientUbi.print(DEVICE_LABEL);
    clientUbi.print(F(" HTTP/1.1\r\n"));
    clientUbi.print(F("Host: "));
    clientUbi.print(HTTPSERVER);
    clientUbi.print(F("\r\n"));
    clientUbi.print(F("User-Agent: "));
    clientUbi.print(USER_AGENT);
    clientUbi.print(F("/"));
    clientUbi.print(VERSION);
    clientUbi.print(F("\r\n"));
    clientUbi.print(F("X-Auth-Token: "));
    clientUbi.print(TOKEN);
    clientUbi.print(F("\r\n"));
    clientUbi.print(F("Connection: close\r\n"));
    clientUbi.print(F("Content-Type: application/json\r\n"));
    clientUbi.print(F("Content-Length: "));
    clientUbi.print(contentLength);
    clientUbi.print(F("\r\n\r\n"));
    clientUbi.print(payload);
    clientUbi.print(F("\r\n"));

    Serial.println(F("Making request to Ubidots:\n"));
    Serial.print("POST /api/v1.6/devices/");
    Serial.print(DEVICE_LABEL);
    Serial.print(" HTTP/1.1\r\n");
    Serial.print("Host: ");
    Serial.print(HTTPSERVER);
    Serial.print("\r\n");
    Serial.print("User-Agent: ");
    Serial.print(USER_AGENT);
    Serial.print("/");
    Serial.print(VERSION);
    Serial.print("\r\n");
    Serial.print("X-Auth-Token: ");
    Serial.print(TOKEN);
    Serial.print("\r\n");
    Serial.print("Connection: close\r\n");
    Serial.print("Content-Type: application/json\r\n");
    Serial.print("Content-Length: ");
    Serial.print(contentLength);
    Serial.print("\r\n\r\n");
    Serial.print(payload);
    Serial.print("\r\n");
  } else {
    Serial.println("Connection Failed ubidots - Try Again");
  }

  /* Reach timeout when the server is unavailable */
  int timeout = 0;
  while (!clientUbi.available() && timeout < 5000) {
    timeout++;
    delay(1);
    if (timeout >= 5000) {
      Serial.println(F("Error, max timeout reached"));
      break;
    }
  }

  /* Reads the response from the server */
  Serial.println(F("\nUbidots' Server response:\n"));
  while (clientUbi.available()) {
    char c = clientUbi.read();
    Serial.print(c); // Uncomment this line to visualize the response on the Serial Monitor
  }

  /* Disconnecting the client */
  clientUbi.stop();
}

/********************************
 * Main Functions
 *******************************/

void setup() {
  Serial.begin(115200);  
  ReconnectDevices();
}

void loop() {

  // disable watchdog for the other unused cpu
  TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE; 
  TIMERG0.wdt_feed = 1; 
  TIMERG0.wdt_wprotect = 0;
  
   scan_connection++;      
   Serial.print(scan_connection);   

  // Space to store values to send
  char payload[200];
  
  char str_val_1[30];
  char str_val_2[30];
  char str_val_3[30];
  char str_val_4[30];    
  char str_val_5[30];
  char str_val_6[30];
  char str_val_7[30];
  char str_val_8[30];

  
  //char str_lat[30];
  // char str_lng[30];


  /* Read PowerMeter PZEM-004 or PZEM-004T*/

   float v = pzem.voltage(ip);
   if (v < 0.0) v = 0.0;
   Serial.print(v);Serial.print("V; ");

  float i = pzem.current(ip);
   if(i >= 0.0){ Serial.print(i);Serial.print("A; "); }

  float p = pzem.power(ip);
   if(p >= 0.0){ Serial.print(p);Serial.print("W; "); }

  float e = pzem.energy(ip);
   if(e >= 0.0){ Serial.print(e);Serial.print("Wh; "); }
  Serial.println();

 // float lat = 3.436161936071848;
//  float lng = -76.51575728912671;

  // Important: Avoid to send a very long char as it is very memory space costly, send small char arrays

   /*  RSSI ESP32 */
   long rssi = WiFi.RSSI();
   Serial.print("RSSI; ");
   Serial.print(rssi);
   Serial.println(" rssi");

    /*  Internal Temperature ESP32 */
    float internal_temperature = ((temprature_sens_read() - 32) / 1.8); /// Convert raw temperature in F to Celsius degrees 
    Serial.print("Temperature: ");     
    Serial.print(internal_temperature);
    Serial.println("°C");

  /*---- Transforms the values of the sensors to char type -----*/
  /* 4 is mininum width, 2 is precision; float value is copied onto str_val*/  
  
  dtostrf(rssi, 4, 2, str_val_6);                    //  rssi
  dtostrf(internal_temperature, 4, 2, str_val_7);    // internal_temperature
  dtostrf(scan_connection, 4, 2, str_val_8);         // scan send data ubidots  
 

     /* -- Communication failure with meter -- */   
     /*
      * If the ESP32 has no communication with the meter it will load the last values when connected.
      */
     
    if ((i == -1.00) && (p == -1.00) && (e == -1.00))        
    {    
          /* Previous Measurements */                    
          state_meter=1;                   
          v_prev = v_now;
          i_prev = i_now;
          p_prev = p_now;
          e_prev = e_now;      
                  
          dtostrf(v_prev, 4, 2, str_val_1);    // Volts
          dtostrf(i_prev, 4, 2, str_val_2);   //  Amp
          dtostrf(p_prev, 4, 2, str_val_3);   //  W  -power
          dtostrf(e_prev, 4, 2, str_val_4);   //  Wh  -energy
          dtostrf(state_meter, 4, 2, str_val_5);   //  state_meter
  
      
    }else
    {     
         /* Current Measurement */              
          state_meter=0;
          v_now=v;
          i_now=i;
          p_now=p;
          e_now=e;                    
          dtostrf(v_now, 4, 2, str_val_1);           // Volts
          dtostrf(i_now, 4, 2, str_val_2);           //  Amp
          dtostrf(p_now, 4, 2, str_val_3);           //  W  -power
          dtostrf(e_now, 4, 2, str_val_4);           //  Wh  -energy
          dtostrf(state_meter, 4, 2, str_val_5);     //  state_meter
          
    }
  
        sprintf(payload, "{\"");        
        sprintf(payload, "%s%s\":%s",     payload, VARIABLE_LABEL_1, str_val_1);      // volts  
        sprintf(payload, "%s,\"%s\":%s",  payload, VARIABLE_LABEL_2, str_val_2);      // amp  
        sprintf(payload, "%s,\"%s\":%s",  payload, VARIABLE_LABEL_3, str_val_3);      // power   
        sprintf(payload, "%s,\"%s\":%s ", payload, VARIABLE_LABEL_4, str_val_4);      // energy   
        sprintf(payload, "%s,\"%s\":%s ", payload, VARIABLE_LABEL_5, str_val_5);      // state power meter 
        sprintf(payload, "%s,\"%s\":%s ", payload, VARIABLE_LABEL_6, str_val_6);      // RSS1 
        sprintf(payload, "%s,\"%s\":%s ", payload, VARIABLE_LABEL_7, str_val_7);      // internal_temperature 
        sprintf(payload, "%s,\"%s\":%s ", payload, VARIABLE_LABEL_8, str_val_8);      // scan send data ubidots        
        sprintf(payload, "%s}", payload);  
      
  
  /* Calls the Ubidots Function POST */
  SendToUbidots(payload);

  delay(10000);

  //  Check Wi-Fi connection every 5 min   
  //  300,000 ms = 5 minutes   
  //  300,000 ms /10000 ms = 30 scan
     
   if(scan_connection>=30) {          
        ReconnectDevices();                
        scan_connection=0;                     
   }

  

}
