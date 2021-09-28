#include "constants.h"
#include <ESP.h>

#if defined(ESP8266)
    #include <ESP8266WiFi.h>
#elif defined(ESP32)
    #include <WiFi.h>
#endif
#include <FastLED.h>
#include <WiFiUDP.h>
#include "reactive_common.h"

#define READ_PIN 0
#define BUTTON_PIN D1

#define NUMBER_OF_CLIENTS 3

const int checkDelay = 5000;
const int buttonDoubleTapDelay = 200;
const int numOpModes = 3;

unsigned long lastChecked;
unsigned long buttonChecked;
bool buttonClicked = false;
bool queueDouble = false;
bool clickTrigger;
bool doubleTapped;
WiFiUDP UDP;

struct led_command {
  uint8_t opmode;
  uint32_t data;
};

bool heartbeats[NUMBER_OF_CLIENTS];

static int opMode = 1;

void setup()
{
  pinMode(READ_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT );

  /* WiFi Part */
  Serial.begin(115200);
  Serial.println();
  setupWifi();
  UDP.begin(7171); 
  resetHeartBeats();
  waitForConnections();
  lastChecked = millis();
  buttonChecked = 0;
}


void loop()
{
  uint32_t analogRaw;
  buttonCheck();
  if (millis() - lastChecked > checkDelay) {
    if (!checkHeartBeats()) {
      waitForConnections();
    }
    lastChecked = millis();
  }

  switch (opMode) {
    case 1:
      analogRaw = analogRead(READ_PIN);
      if (analogRaw <= 3)
        break;
      sendLedData(analogRaw, opMode);
      break;
    case 2:
      sendLedData(0, opMode);
      delay(10);
      break;
    case 3:
      sendLedData(0, opMode);
      delay(10);
      break;
  }
  delay(4);
}

void sendLedData(uint32_t data, uint8_t op_mode) 
{
 struct led_command send_data;
 send_data.opmode = op_mode; 
 send_data.data = data; 
 for (int i = 0; i < NUMBER_OF_CLIENTS; i++) 
 {
    IPAddress ip(192,168,4,2 + i);
    UDP.beginPacket(ip, 7001); 
    UDP.write((char*)&send_data,sizeof(struct led_command));
    UDP.endPacket();
 }
}

void setupWifi() {
  // AP mode password
  const char WiFiAPPSK[] = "123456789";
  //  // Set Hostname.
    String hostname(soundreactive);
  
    uint64_t chipid = ESP.getEfuseMac();
    uint16_t long1 = (unsigned long)((chipid & 0xFFFF0000) >> 16 );
    uint16_t long2 = (unsigned long)((chipid & 0x0000FFFF));
    String hex = String(long1, HEX) + String(long2, HEX); // six octets
    hostname += hex;
  
    char hostnameChar[hostname.length() + 1];
    memset(hostnameChar, 0, hostname.length() + 1);
  
    for (uint8_t i = 0; i < hostname.length(); i++)
      hostnameChar[i] = hostname.charAt(i);
  
    WiFi.setHostname(hostnameChar);
  
    // Print hostname.
    Serial.println("Hostname: " + hostname);
  //  if (StMode)
  //  {
  //WiFi.mode(WIFI_STA);
  //Serial.printf("Connecting to %s\n", ssid);
  //if (String(WiFi.SSID()) != String(ssid)) {
  //  WiFi.begin(ssid, password);
  //}
  //  }
  //  else
  //  {
      WiFi.mode(WIFI_AP);
      WiFi.softAP(hostnameChar, WiFiAPPSK);
      Serial.printf("Connect to Wi-Fi access point: %s\n", hostnameChar);
      Serial.println(WiFi.softAPIP());
      //Serial.println("and open http://192.168.4.1 in your browser");
  //  }

}

void waitForConnections() {
  while(true) {
      readHeartBeat();
      if (checkHeartBeats()) {
        return;
      }
      delay(checkDelay);
      resetHeartBeats();
  }
}

void resetHeartBeats() {
  for (int i = 0; i < NUMBER_OF_CLIENTS; i++) {
   heartbeats[i] = false;
  }
}

void readHeartBeat() {
  struct heartbeat_message hbm;
 while(true) {
  int packetSize = UDP.parsePacket();
  if (!packetSize) {
    break;
  }
  UDP.read((char *)&hbm, sizeof(struct heartbeat_message));
  if (hbm.client_id > NUMBER_OF_CLIENTS) {
    Serial.println("Error: invalid client_id received");
    continue;
  }
  heartbeats[hbm.client_id - 1] = true;
 }
}

bool checkHeartBeats() {
  for (int i = 0; i < NUMBER_OF_CLIENTS; i++) {
   if (!heartbeats[i]) {
    return false;
   }
  }
  resetHeartBeats();
  return true;
}


void buttonCheck()
{
  int but = digitalRead(BUTTON_PIN);
  if (but == 0) {
    if (millis() - buttonChecked < buttonDoubleTapDelay && buttonClicked == false ) {
      doubleClicked();
      doubleTapped = true;
    }
    clickTrigger = true;
    buttonClicked = true; 
    buttonChecked = millis();
  }

  else if (but == 1) {
    if (millis() - buttonChecked > buttonDoubleTapDelay && clickTrigger) {
      if (!doubleTapped) {
        clicked();
      }
      clickTrigger = false;
      doubleTapped = false;
    }
    buttonClicked = false;
  }
}

void clicked() {
  if (opMode == numOpModes)
    opMode = 1;
  else
    opMode++;
  Serial.printf("Setting opmode %d \n", opMode);
}

void doubleClicked() {

}
