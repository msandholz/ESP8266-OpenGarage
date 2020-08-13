#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>                   // Include the WebServer library
#include <ESP8266WebServerSecure.h>
#include <Wire.h>
#include "ESP8266TrueRandom.h"

const char* ssid = "--SSID--";                      // Enter SSID here
const char* password = "--password--";  // Enter Password here
byte uuidNumber[16];                            // UUIDs in binary form are 16 bytes long
String uuidString;
boolean debug = false;                          // Debugging on serial port 

const boolean isD1mini = true;                 // Model of ESP ("ESP8266-01" or "D1mini")
byte switchOff[] = {0xA0, 0x01, 0x00, 0xA1};          // switch on command for "ESP8266-01"
byte switchOn[] = {0xA0, 0x01, 0x01, 0xA2};           // switch off command for "ESP8266-01"     
const int GPIO = 5;                             // switch on/off pin of "D1mini" (GPIO5 / Pin D1)

// Key for Garage
const String key = "--some key--";
// Key for Door
//const String key = "--some key--";



static const char serverCert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)EOF";

static const char serverKey[] PROGMEM =  R"EOF(
-----BEGIN RSA PRIVATE KEY-----
-----END RSA PRIVATE KEY-----
)EOF";


ESP8266WebServer server(80);
//BearSSL::ESP8266WebServerSecure server(443);    



void setup() {

  Serial.begin(9600);
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(GPIO, OUTPUT);
  digitalWrite(GPIO, LOW);

  //connect to your local wi-fi network
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);
   }

   if(debug){Serial.println("WiFi connected!");}

   //server.getServer().setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));
   server.on("/token", HTTP_GET, handle_OnConnect);
   server.on("/switch", HTTP_GET, handle_OnSwitch);
   //server.on("/unsec", HTTP_GET, handle_OnUnSecure);
   server.onNotFound(handle_NotFound);

 
  server.begin();
  if(debug){Serial.println("WebServer started!");}

}

void loop() {
  server.handleClient();
}

void switchRelais(){
  if(isD1mini) {
        // Switch on/off D1mini
        digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(GPIO, HIGH);
          delay(1000);
        digitalWrite(LED_BUILTIN, HIGH);
        digitalWrite(GPIO, LOW);
      
      }else{
        // Switch on/off ESP8266-01
        digitalWrite(LED_BUILTIN, LOW);
        Serial.write(switchOn, sizeof(switchOn));
          delay(1000);
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.write(switchOff, sizeof(switchOff));    
      }
}

String generateKey(int i){

  String key;
  for(int ii=0;ii<i;ii++){
    ESP8266TrueRandom.uuid(uuidNumber);
    key += ESP8266TrueRandom.uuidToString(uuidNumber)+"--";
  }  
  return key;
}


void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

void handle_OnConnect() {
  
  Serial.println("--> Start <--");
  Serial.println("OnConnect -> key:" + server.arg("key"));
  
  if (server.arg("key") == key) {

    uuidString = generateKey(5);
    server.send(200, "text/html", uuidString); 
    
    Serial.println("OnConnect -> 200 accepted / token:" + uuidString);
  
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    
  } else {
    server.send(403, "text/html", "forbidden"); 
    if(debug){Serial.println("OnConnect -> 403 forbidden");}
  }
}


void handle_OnSwitch(){
  
  if(debug){Serial.print("/swtich?token=" + server.arg("token"));}
  
  if (server.arg("token") == uuidString) {
    server.send(200, "text/plain", "accept");
    switchRelais();
    if(debug){Serial.println("OnConnect -> 200 accepted / open");}
    
  } else {
    
    server.send(403, "text/plain", "forbidden");
    if(debug){Serial.println("OnSwitch -> 403 forbidden");}
  }
   uuidString = generateKey(5);
}


void handle_OnUnSecure(){
  server.send(200, "text/plain", "accept");
  switchRelais();
  if(debug){Serial.println("/unsecure");}
}
