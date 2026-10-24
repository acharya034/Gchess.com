
#pragma once

#include <Arduino.h>
#include "lichessGlobal.h"

// --------------------------------------------------
// Request queue
// --------------------------------------------------

bool lichessRequest(requestType newRt = NONE);
bool lichessSendRequest();

// --------------------------------------------------
// Event stream
// --------------------------------------------------

bool getEvent();

// --------------------------------------------------
// Challenge functions
// --------------------------------------------------

bool createChallenge();
bool acceptChallenge();
bool declineChallenge();
bool challengeAI();

// --------------------------------------------------
// Game functions
// --------------------------------------------------

bool sendMove();
bool sendMove(String givenmove);

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

// --------------------------------------------------
// Chat functions
// --------------------------------------------------

bool sendChatMessage();
bool getChatMessages();

// --------------------------------------------------
// Account functions
// --------------------------------------------------

bool getAccountInfo();
