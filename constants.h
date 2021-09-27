#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "config.h"
#include <FastLED.h>

// Network constants
#define SUBSCRIBE_INTERVAL_MS 1000
#define MAX_CONNECTION_LOSS_MS 5000

// Command constants
#define SUBSCRIBE_COMMAND "S:"

// Status constants

// Network event constants
#if defined(ESP8266)
    #define EVENT_CONNECTED WIFI_EVENT_STAMODE_CONNECTED
    #define EVENT_GOT_IP WIFI_EVENT_STAMODE_GOT_IP
    #define EVENT_DISCONNECTED WIFI_EVENT_STAMODE_DISCONNECTED
#elif defined(ESP32)
    #define EVENT_CONNECTED SYSTEM_EVENT_STA_CONNECTED
    #define EVENT_GOT_IP SYSTEM_EVENT_STA_GOT_IP
    #define EVENT_DISCONNECTED SYSTEM_EVENT_STA_DISCONNECTED
#endif

#endif CONSTANTS_H
