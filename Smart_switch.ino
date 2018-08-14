#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <string.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
#include <BlynkSimpleEsp8266.h>

IPAddress    apIP(42, 42, 42, 42);  // Defining a static IP address: local & gateway
                                    // Default IP in AP mode is 192.168.4.1

#define TRIGGER_PIN 0
int LED = HIGH;
/* This are the WiFi access point settings. Update them to your likin */
const char *ssid = "ESP8266";
const char *password = "ESP8266Test";

// Define a web server at port 80 for HTTP
ESP8266WebServer server(80);
char authe[] = "a07659b7550a4a31ab53253fb6fec1e2";
WiFiServer serverc(81);

long timeout=0;
char estado=0;

const int ledPin = LED_BUILTIN; // an LED is connected to NodeMCU pin D1 (ESP8266 GPIO5) via a 1K Ohm resistor

bool ledState = false;

void handleRoot() {
  digitalWrite (ledPin, server.arg("led").toInt());
  ledState = digitalRead(ledPin);

 /* Dynamically generate the LED toggle link, based on its current state (on or off)*/
  char ledText[80];
  
  if (ledState) {
    strcpy(ledText, "SWITCH is OFF. <p><a href=\"/?led=0\">Turn it ON!</a></p>");
  }

  else {
    strcpy(ledText, "SWITCH is ON. <p><a href=\"/?led=1\">Turn it OFF!</a></p>");
  }
 
  ledState = digitalRead(ledPin);

  char html[1000];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;


// Build an HTML page to display on the web-server root address
  snprintf ( html, 1000,

"<html>\
  <head>\
    <meta http-equiv='refresh' content='1'/>\
    <title>Smart Hub Connect</title>\
    <style>\
      body { font-family: Arial, Helvetica, Sans-Serif; font-size: 1.5em; Color: #000000; }\
    </style>\
  </head>\
  <body>\
  <center>\
    <h1>SMART HUB</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <p>%s<p>\
    </center>\
  </body>\
</html>",

    hr, min % 60, sec % 60,
    ledText
  );
  server.send ( 200, "text/html", html );
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}

void setup() {
  pinMode ( ledPin, OUTPUT );
  pinMode(TRIGGER_PIN, INPUT);
  digitalWrite ( ledPin, 0 );
  
  delay(1000);
  //set-up the custom IP address
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));   // subnet FF FF FF 00  
  
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
 
  server.on ( "/", handleRoot );
  server.on ( "/led=1", handleRoot);
  server.on ( "/led=0", handleRoot);
  server.on ( "/inline", []() {
    server.send ( 200, "text/plain", "this works as well" );
  } );
  server.onNotFound ( handleNotFound );
  
  server.begin();
  
  Blynk.config(authe);
}

void loop() {
  server.handleClient();
  if ( digitalRead(TRIGGER_PIN) == LOW ) {
    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

    //reset settings - for testing
    //wifiManager.resetSettings();

    //sets timeout until configuration portal gets turned off
    //useful to make it all retry or go to sleep
    //in seconds
    //wifiManager.setTimeout(120);

    //it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration

    //WITHOUT THIS THE AP DOES NOT SEEM TO WORK PROPERLY WITH SDK 1.5 , update to at least 1.5.1
    //WiFi.mode(WIFI_STA);
    
    if (!wifiManager.startConfigPortal(ssid,password)) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }
    //if you get here you have connected to the WiFi
    
  }

}
