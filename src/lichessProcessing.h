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

// game id for the current game that is being played and if none then it will be empty string
String gameId;

// variables and functions to handel http request
enum requestType {NONE, CHALLENGE, MOVE, SEEK, STREAM, DRAW, RESIGN, ABORT, OFFER_DRAW, ACCEPT_DRAW, DECLINE_DRAW, OFFER_TAKEBACK, ACCEPT_TAKEBACK, DECLINE_TAKEBACK, CLAM_VICTORY, CHAT, GET_CHAT, GET_ACCOUNT};

requestType currentRT = NONE;
requestType lastRT[2] = {NONE, NONE};


// call the lichessRequest function with parameter when adding task and call it without prameter when the task is done.
bool lichessRequest(requestType newRt = NONE);