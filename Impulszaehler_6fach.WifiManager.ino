//http://esp8266-server.de/Impulszaehler.html
/*
 *  6-fach-S0-Zähler
 * 
 * Ticker-Lib
 * https://github.com/esp8266/Arduino/tree/master/libraries/Ticker
 * 
 * pins_arduino.h:
 * 
    #define LED_BUILTIN 16
    static const uint8_t D0   = 16;
    static const uint8_t D1   = 5;
    static const uint8_t D2   = 4;
    static const uint8_t D3   = 0;
    static const uint8_t D4   = 2;
    static const uint8_t D5   = 14;
    static const uint8_t D6   = 12;
    static const uint8_t D7   = 13;
    static const uint8_t D8   = 15;
    static const uint8_t D9   = 3;
    static const uint8_t D10  = 1;
 * 
 */
#include <pins_arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>
#include <Ticker.h>
extern "C" {
#include "gpio.h"
#include "user_interface.h"
}

//--- Zuordnung der GPIOs
#define INT_PIN1  5     // D1      
#define INT_PIN2  4     // D2
#define INT_PIN3  2    //  D4
#define INT_PIN4  13   //  D7
#define INT_PIN5  12   //  D6
#define INT_PIN6  14   //  D5
#define LED       BUILTIN_LED
 
unsigned long counter[6];
unsigned long antwort[6];  //  Impulse pro Minute
volatile bool doBlink = false; 
volatile short zaehlerId = -1; 
volatile unsigned long alteZeit[6] , entprellZeit=30;
 
ESP8266WebServer server(80);// Serverport  hier einstellen
ESP8266HTTPUpdateServer httpUpdater;
Ticker Timer;
//------------------------------------------------
void interruptRoutine1() {
  if((millis() - alteZeit[0]) > entprellZeit) 
  {
    counter[0]++;
    doBlink = true; 
    zaehlerId = 0;
    alteZeit[0] = millis(); // letzte Schaltzeit merken      
  }
}
//------------------------------------------------
void interruptRoutine2() {
  if((millis() - alteZeit[1]) > entprellZeit) 
  {
    counter[1]++;
    doBlink = true; 
    zaehlerId = 1;
    alteZeit[1] = millis(); // letzte Schaltzeit merken      
  }
}
//------------------------------------------------
void interruptRoutine3() {
  if((millis() - alteZeit[2]) > entprellZeit) 
  {
    counter[2]++;
    doBlink = true; 
    zaehlerId = 2;
    alteZeit[2] = millis(); // letzte Schaltzeit merken  
  }    
}
//------------------------------------------------
void interruptRoutine4() {
  if((millis() - alteZeit[3]) > entprellZeit) 
  {
    counter[3]++;
    doBlink = true; 
    zaehlerId = 3;
    alteZeit[3] = millis(); // letzte Schaltzeit merken  
 }    
}
//------------------------------------------------
void interruptRoutine5() {
  if((millis() - alteZeit[4]) > entprellZeit) 
  {
    counter[4]++;
    doBlink = true; 
    zaehlerId = 4;
    alteZeit[4] = millis(); // letzte Schaltzeit merken  
  }
}
//------------------------------------------------
void interruptRoutine6() {
  if((millis() - alteZeit[5]) > entprellZeit)   
  {
    counter[5]++;
    doBlink = true; 
    zaehlerId = 5;
    alteZeit[5] = millis(); // letzte Schaltzeit merken  
  }
}
//------------------------------------------------ 
void Ereignis_Index()               // Anfrage vom Webbrowser
{
  String Serverantwort = "";        // Antwort für Webbrowser vorbereiten
  for (int i = 0; i < 6; i++)
  {
    Serverantwort += String(antwort[i]) + ",";
  }
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/plain", Serverantwort);
}
//------------------------------------------------ 
void MinutenTimer()             // Wird jede Minute ausgeführt
{
  for (int i = 0; i < 6; i++)   // Alle 6 counter durchgehen
  {
    antwort[i] = counter[i];  // Zählstand in Ergebnis Variable spechern
    counter[i] = 0;           // Zähler löschen für neue Zählung
    alteZeit[i] =0;
  }
  Serial.println("Timer!");
}
//------------------------------------------------ 
void setup()
{
  pinMode(LED,OUTPUT); 
  digitalWrite(LED,HIGH);  //initially off 
  
  pinMode(INT_PIN1, INPUT_PULLUP);
  pinMode(INT_PIN2, INPUT_PULLUP);
  pinMode(INT_PIN3, INPUT_PULLUP);
  pinMode(INT_PIN4, INPUT_PULLUP);
  pinMode(INT_PIN5, INPUT_PULLUP);
  pinMode(INT_PIN6, INPUT_PULLUP);
 
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Start");
  Serial.println("");
  Serial.println("Warte auf Verbindung");

  //--- WiFiManager
  //--- Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //reset settings - for testing
  if (digitalRead(INT_PIN1)== LOW)
    wifiManager.resetSettings();

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if(!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  } 

  //--- if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");


 
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.println("Mit Wlan verbunden");
    Serial.println("IP Adresse: ");
    Serial.println(WiFi.localIP());
  }
 
  server.on("/", Ereignis_Index);     //  Bechandlung der Ereignissen anschliessen
  httpUpdater.setup(&server);         //  Updater
  server.begin();                     // Starte den Server
  Serial.println("HTTP Server gestartet");

  
 
  Timer.attach(60, MinutenTimer);    // Starte den 60s Timer
 
  // ---------------------- Starte Interrupts ---------------------------------------------
  attachInterrupt(INT_PIN1, interruptRoutine1, FALLING);
  attachInterrupt(INT_PIN2, interruptRoutine2, FALLING);
  attachInterrupt(INT_PIN3, interruptRoutine3, FALLING);
  attachInterrupt(INT_PIN4, interruptRoutine4, FALLING);
  attachInterrupt(INT_PIN5, interruptRoutine5, FALLING);
  attachInterrupt(INT_PIN6, interruptRoutine6, FALLING);
}
//------------------------------------------------ 
void loop()
{
  server.handleClient();

  yield();

  if (doBlink == true)
  {
    blink(); 
    doBlink = false;  
  }

  if (zaehlerId != -1)
  {
    //Serial.println("");
    Serial.print ("counter: ");
    Serial.print(zaehlerId);
    Serial.print("   ");
    Serial.println(counter[zaehlerId]);
    zaehlerId = -1; 
  }
}
//------------------------------------------------
void blink ()
{
  digitalWrite(BUILTIN_LED, LOW);                            
  delay(50);                      
  digitalWrite(BUILTIN_LED, HIGH);  
  delay(100);
} 
//------------------------------------------------
//------------------------------------------------
