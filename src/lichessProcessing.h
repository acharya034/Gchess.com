#pragma once

#include <Arduino.h>
#include <httpClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>


// global variables
// those are for event stream connection
WiFiClient eventClient; 
HTTPClient eventHttp; 

// those are for all requests that gest reused and the wifi connection will be kept alive for all requests.
WiFiClient requestClient;
HTTPClient requestHttp;


