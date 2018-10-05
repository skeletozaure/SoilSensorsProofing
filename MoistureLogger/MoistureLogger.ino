//--------------------------
// Debug Activation & macros
//--------------------------

#define DEBUG 0 // Switch debug output on and off by 1 or 0

#if DEBUG
 #define PRINT(s)       { Serial.print (F(s)); }
 #define PRINTLN(s)     { Serial.println (F(s));}
 #define PRINTV(s,v)    { Serial.print (F(s)); Serial.print(F("\t")); Serial.print (v); }
 #define PRINTLNV(s,v)  { Serial.print (F(s)); Serial.print(F("\t")); Serial.println (v); }
 #
 #define PRINTRAW(s)     Serial.print (s)
 #define PRINTLNRAW(s)   Serial.println (s)
 #define PRINTCR()       Serial.println ()
#else
 #define PRINT(s)         
 #define PRINTRAW(s)        
 #define PRINTV(s,v)    
 #define PRINTLNV(s,v)  
 #define PRINTLN(s)
 #define PRINTLNRAW(s)       
 #define PRINTCR()       
#endif

//---------
// Include
//---------

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "RunningMedian.h"

// ----------------------------------------------------------------------
// Mesures réalisées à l'air libre et dans l'eau pour le capteur utilisé
// ---------------------------------------------------------------------
const double cAirValue = 569;   
const double cWaterValue = 153;  
const double rAirValue = 914;   
const double rWaterValue = 225;  

//-------------------------------------------------
// coefficients for affine function y=ax+b 
//-------------------------------------------------
double ca;
double cb;
double ra;
double rb;

// -----------------------------------------------------
// Raw soil moisture values and calculated
// -----------------------------------------------------
int csoilMoistureValue=0;
int csoilMoisturePercent=0;
int rsoilMoistureValue=0;
int rsoilMoisturePercent=0;

//------
// Wifi
//------
const char* ssid = "UrSsid";
const char* password = "UrPwd";
WiFiClient client;

//---------------------
// settings ThingSpeak 
//---------------------
long lastConnectionTime = 0;
boolean lastConnected = false;
int failedCounter = 0;
const  String writeAPIKey = "UrApiKey";
const  unsigned long updateThingSpeakInterval = 60 * 1000 * 5;      // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)
const uint16_t port = 80;
const char * server = "api.thingspeak.com";  

//-----------------------
// Gestion moyenne mobile
//-----------------------

//https://github.com/RobTillaart/Arduino/tree/master/libraries/RunningMedian
RunningMedian csoilMoisturePercentMedian = RunningMedian(11); //Moyenne mobile sur une heure, avec 1 mesure toutes les 5mn
RunningMedian rsoilMoisturePercentMedian = RunningMedian(11);

void setup() {
  Serial.begin(9600); // open serial port, set the baud rate to 9600 bps
  
  // Values for Capactive affine function
  ca = -(100/(cAirValue-cWaterValue));
  cb = (100 * cAirValue) /(cAirValue-cWaterValue);
  
  PRINTV("ca :",ca);
  PRINTLNV("\tcb :",cb);

  // Values for Resistive affine function
  ra = -(100/(rAirValue-rWaterValue));
  rb = (100 * rAirValue) /(rAirValue-rWaterValue);
  
  PRINTV("ra :",ra);
  PRINTLNV("\trb :",rb);

  //Use diodes between output of sensors and A0 to protect sensor each other to interfere
  //see https://www.instructables.com/id/Multiple-Analog-Inputs-on-Only-One-Analoge-Pin/
  //Capactive sensor must be plugged between D3 & D4, and A0 for analog input
  pinMode(D3,OUTPUT); //GND
  pinMode(D4,OUTPUT); //3,3V
  digitalWrite(D3,LOW);
  digitalWrite(D4,LOW);
  //Resistive sensor must be plugged between D6 & D7, and A0 for analog input
  pinMode(D6,OUTPUT); //GND
  pinMode(D7,OUTPUT); //3,3V
  digitalWrite(D6,LOW);
  digitalWrite(D7,LOW); 

  //Connection au Wifi
  PRINTLN("Wifi connection...");
  connectWifi();
  printIPAddress();
}

void loop() {

  while(client.connected()) {
     while(client.available()) {
      char c = client.read();
      PRINTRAW(c);
     }
  }
  
  // Disconnect from ThingSpeak
  if (!client.connected() && lastConnected)
  {
    PRINTCR();
    PRINTLN("...disconnected");
    PRINTCR();

    client.stop();
  }
 

  //-------------------------------------------------
  // measure soil moisture every second in debug mode
  //-------------------------------------------------

  #if DEBUG 
    if (!client.connected()) {
      readAllSensors();
      delay(1000); //debug only (pour afficher une mesure toutes les secondes)
    }
  #endif

  //-------------------
  // Update ThingSpeak
  //-------------------
  if (!client.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval))
  {

    //------------------------
    //--      Mesures       --
    //------------------------

    readAllSensors();

    String sensorValues = "field1=" + String(csoilMoisturePercentMedian.getMedian(),DEC);
    sensorValues+="&field2="+String(rsoilMoisturePercentMedian.getMedian(), DEC);
    Serial.println(F("Sending data to ThingSpeak"));
    updateThingSpeak(sensorValues);
  }

  // Check if Wifi needs to be restarted
  if (failedCounter > 3) {
    connectWifi();
  }

  lastConnected = client.connected();
}

void updateThingSpeak(String tsData)
{
  if (client.connect(server, port))
  {
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + writeAPIKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");
    client.print(tsData);

    lastConnectionTime = millis();

    if (client.connected())
    {
      PRINTLN("Connecting to ThingSpeak...");
      PRINTCR();
      failedCounter = 0;
    }
    else
    {
      failedCounter++;
      PRINTRAW("Connection to ThingSpeak failed (" +  String(failedCounter, DEC) + ")");
      PRINTCR();
    }
  }
  else
  {
    failedCounter++;
    PRINTRAW("Connection to ThingSpeak Failed (" + String(failedCounter, DEC) + ")");
    PRINTCR();
    lastConnectionTime = millis();
  }
}

void readAllSensors () {

  //-----------------
  //Capacitive sensor
  //-----------------
  digitalWrite(D4,HIGH);
  delay(10);
  //Read twice to let circuit stabilize and throw away first measure
  csoilMoistureValue = analogRead(A0);  
  delay(10);
  csoilMoistureValue = analogRead(A0);  
  digitalWrite(D4,LOW);
  
  csoilMoisturePercent = ca * (double)csoilMoistureValue + cb;
  if(csoilMoisturePercent > 100) {
    csoilMoisturePercent = 100;
  }
  if(csoilMoisturePercent < 0) {
    csoilMoisturePercent = 0;
  }

  //Add to running median
  csoilMoisturePercentMedian.add(csoilMoisturePercent);

  PRINTV("Capactive Moist %   :",csoilMoisturePercent);
  PRINTV("\tCapactive Raw Value :",csoilMoistureValue);
  PRINTLNV("\tCap. Moist Median % :",csoilMoisturePercentMedian.getMedian());
  
  //-----------------
  //Resistive sensor
  //-----------------
  digitalWrite(D7,HIGH);
  delay(10);
  //Read twice to let circuit stabilize and throw away first measure
  rsoilMoistureValue = analogRead(A0);  
  delay(10);
  rsoilMoistureValue = analogRead(A0);  
  digitalWrite(D7,LOW);
  
  rsoilMoisturePercent = ra * (double)rsoilMoistureValue + rb;
  if(rsoilMoisturePercent > 100) {
    rsoilMoisturePercent = 100;
  }
  if(rsoilMoisturePercent < 0) {
    rsoilMoisturePercent = 0;
  }

  //Add to running median
  rsoilMoisturePercentMedian.add(rsoilMoisturePercent);

  PRINTV("Resistive Moist %   :",rsoilMoisturePercent);
  PRINTV("\tResistive Raw Value :",rsoilMoistureValue);
  PRINTLNV("\tRes. Moist Median % :",rsoilMoisturePercentMedian.getMedian());

}

void connectWifi() {
  // We start by connecting to a WiFi network

  client.stop();

  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  PRINTCR();
  PRINTCR();
  PRINT("Wait for WiFi... ");

  while (WiFi.status() != WL_CONNECTED) {
    PRINT(".");
    delay(500);
  }
}
  
void printIPAddress()
{
  PRINTCR();
  PRINTLN("WiFi connected");
  PRINTLN("IP address: ");
  PRINTLNRAW(WiFi.localIP());
  PRINTCR();
}
