
#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// --------------------------------------------------
// HTTP clients
// --------------------------------------------------

extern WiFiClientSecure eventClient;
extern HTTPClient eventHttp;

extern WiFiClientSecure requestClient;
extern HTTPClient requestHttp;

// --------------------------------------------------
// Global game and JSON data
// --------------------------------------------------

extern String gameId;

extern JsonDocument eventDoc;
extern JsonDocument requestDoc;
extern JsonDocument chatDoc;
extern JsonDocument accountDoc[3];

extern String opponentUsername;
extern String selfUsername;
extern String accountUsername;

// --------------------------------------------------
// Authentication
// --------------------------------------------------

extern String lichessToken;

// --------------------------------------------------
// Challenge settings
// --------------------------------------------------

extern String challengeId;

extern bool challengeRated;
extern uint16_t challengeClockLimit;
extern uint16_t challengeClockIncrement;

extern String challengeVariant;
extern String challengeFen;
extern String challengeColor;

// --------------------------------------------------
// AI settings
// --------------------------------------------------

extern uint8_t aiLevel;
extern String aiColor;
extern uint16_t aiClockLimit;
extern uint16_t aiClockIncrement;

// --------------------------------------------------
// Move
// --------------------------------------------------

extern String move;

// --------------------------------------------------
// Seek settings
// --------------------------------------------------

extern bool seekRated;
extern uint16_t seekTime;
extern uint16_t seekIncrement;

extern String seekColor;
extern String seekRatingRange;

// --------------------------------------------------
// Chat settings
// --------------------------------------------------

extern String chatMessage;
extern String chatRoom;

// --------------------------------------------------
// Request queue
// --------------------------------------------------

enum requestType {
    NONE,
    CHALLENGE,
    ACCEPT_CHALLENGE,
    DECLINE_CHALLENGE,
    CHALLENGE_AI,
    MOVE,
    SEEK,
    STREAM,
    RESIGN,
    ABORT,
    OFFER_DRAW,
    ACCEPT_DRAW,
    DECLINE_DRAW,
    OFFER_TAKEBACK,
    ACCEPT_TAKEBACK,
    DECLINE_TAKEBACK,
    CLAM_VICTORY,
    CHAT,
    GET_CHAT,
    GET_ACCOUNT,
    AUTHENTICATION
};

extern requestType currentRT;
extern requestType lastRT[2];
