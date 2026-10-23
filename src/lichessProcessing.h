#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// --------------------------------------------------
// Global variables
// --------------------------------------------------

WiFiClientSecure eventClient;
HTTPClient eventHttp;

WiFiClientSecure requestClient;
HTTPClient requestHttp;

String gameId;

requestType currentRT = NONE;
requestType lastRT[2] = { NONE, NONE };

JsonDocument eventDoc;
JsonDocument requestDoc;
JsonDocument chatDoc;
JsonDocument accountDoc[3];

String opponentUsername;

String challengeId;

bool challengeRated = false;
uint16_t challengeClockLimit = 300;
uint16_t challengeClockIncrement = 1;
String challengeVariant = "standard";
String challengeFen = "";
String challengeColor = "random";

uint8_t aiLevel = 1;

String move;

String seekRatingRange = "";

String chatMessage;
String chatRoom = "player";

// Request queue
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

// Queue management
bool lichessRequest(requestType newRt = NONE);
bool lichessSendRequest();

// JSON documents
extern JsonDocument eventDoc;
extern JsonDocument requestDoc;
extern JsonDocument chatDoc;
extern JsonDocument accountDoc[3];

// Event stream
bool getEvent();

// Authentication
extern String lichessToken;

// Usernames
extern String selfUsername;
extern String opponentUsername;
extern String accountUsername;

// Challenge settings
extern String challengeId;
extern bool challengeRated;
extern uint16_t challengeClockLimit;
extern uint16_t challengeClockIncrement;
extern String challengeVariant;
extern String challengeFen;
extern String challengeColor;

// AI settings
extern uint8_t aiLevel;
extern String aiColor;
extern uint16_t aiClockLimit;
extern uint16_t aiClockIncrement;

// Move
extern String move;

// Seek settings
extern bool seekRated;
extern uint16_t seekTime;
extern uint16_t seekIncrement;
extern String seekColor;
extern String seekRatingRange;

// Chat settings
extern String chatMessage;
extern String chatRoom;

// Challenge functions
bool createChallenge();
bool acceptChallenge();
bool declineChallenge();
bool challengeAI();

// Game functions
bool sendMove();
bool startSeek();
bool startStream();

bool resignGame();
bool abortGame();

bool offerDraw();
bool acceptDraw();
bool declineDraw();

bool offerTakeback();
bool acceptTakeback();
bool declineTakeback();

bool claimVictory();

// Chat functions
bool sendChatMessage();
bool getChatMessages();

// Account functions
bool getAccountInfo();