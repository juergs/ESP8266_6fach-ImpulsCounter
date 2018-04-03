//http://esp8266-server.de/Impulszaehler.html
/*
 *  6-fach-S0-Zähler, nicht als Access-Point konfiguriert!
 * 
 * Ticker-Lib: https://github.com/esp8266/Arduino/tree/master/libraries/Ticker
 * Entprellen: http://shelvin.de/eine-taste-per-interrupt-einlesen-und-entprellen/
 * 
 */
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <Ticker.h>
extern "C" {
#include "gpio.h"
#include "user_interface.h"
}


#define WLAN_SID <my_wlan_sid>
#define WLAN_PWD <my_wlan_pwd>


//--- Zuordnung der GPIOs
#define INT_PIN1  5     // D1      
#define INT_PIN2  4     // D2
#define INT_PIN3  2    //  D4
#define INT_PIN4  13   //  D7
#define INT_PIN5  12   //  D6
#define INT_PIN6  14   //  D5
#define LED       BUILTIN_LED
 
unsigned long zehler[6];
unsigned long antwort[6];  //  Impulse pro Minute
volatile bool doBlink = false; 
volatile short zaehlerId = -1; 
volatile unsigned long alteZeit[6] , entprellZeit=30;
 
ESP8266WebServer server(80);// Serverport  hier einstellen
ESP8266HTTPUpdateServer httpUpdater;
Ticker Timer;
 
void interruptRoutine1() {
  if((millis() - alteZeit[0]) > entprellZeit) 
  {
    zehler[0]++;
    doBlink = true; 
    zaehlerId = 0;
    alteZeit[0] = millis(); // letzte Schaltzeit merken      
  }
}
void interruptRoutine2() {
  if((millis() - alteZeit[1]) > entprellZeit) 
  {
  zehler[1]++;
  doBlink = true; 
  zaehlerId = 1;
  alteZeit[1] = millis(); // letzte Schaltzeit merken      
  }
}
void interruptRoutine3() {
  zehler[2]++;
  doBlink = true; 
  zaehlerId = 2;
}
void interruptRoutine4() {
  zehler[3]++;
  doBlink = true; 
  zaehlerId = 3;
}
void interruptRoutine5() {
  zehler[4]++;
  doBlink = true; 
  zaehlerId = 4;
}
void interruptRoutine6() {
  zehler[5]++;
  doBlink = true; 
  zaehlerId = 5;
}
 
//------------------------------------------------------------ 
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
//------------------------------------------------------------ 
void MinutenTimer()             // Wird jede Minute ausgeführt
{
  for (int i = 0; i < 6; i++)   // Alle 6 Zehler durchgehen
  {
    antwort[i] = zehler[i];  // Zählstand in Ergebnis Wariable spechern
    zehler[i] = 0;           // Zähler löschen für neue Zählung
    alteZeit[i] =0;
  }
  Serial.println("Timer!");
}
//------------------------------------------------------------ 
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
 
  WiFi.mode(WIFI_STA);                            // station  modus verbinde mit dem Router
  WiFi.begin(WLAN_SID, WLAN_PWD);          // WLAN Login daten
  WiFi.hostname("ESP-Zaehler");
 
  // Warte auf verbindung
  int timout = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("O");
    timout++;
    if  (timout > 20) // Wenn Anmeldung nicht möglich
    {
      Serial.println("");
      Serial.println("Wlan verbindung fehlt");
      break;
    }
  }  
 
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
//------------------------------------------------------------ 
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
    Serial.println(zehler[zaehlerId]);
    zaehlerId = -1; 
  }
}

void blink ()
{
  digitalWrite(BUILTIN_LED, LOW);                            
  delay(50);                      
  digitalWrite(BUILTIN_LED, HIGH);  
  delay(100);
} 
//------------------------------------------------------------
//------------------------------------------------------------
