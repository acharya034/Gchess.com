#include "lichessProcessing.h"
#include <general.h>
#include <lichessAccount.h>
#include <chessEngine.h>

// --------------------------------------------------
// HTTP clients
// --------------------------------------------------

WiFiClientSecure eventClient;
HTTPClient eventHttp;

WiFiClientSecure requestClient;
HTTPClient requestHttp;

// --------------------------------------------------
// Global game and JSON data
// --------------------------------------------------

String gameId;

JsonDocument eventDoc;
JsonDocument requestDoc;
JsonDocument chatDoc;
JsonDocument accountDoc[3];

String opponentUsername;
String selfUsername;
String accountUsername;

// --------------------------------------------------
// Authentication
// --------------------------------------------------

// Define this here only if no other .cpp file defines it.
String lichessToken;

// --------------------------------------------------
// Challenge settings
// --------------------------------------------------

String challengeId;

bool challengeRated = false;
uint16_t challengeClockLimit = 300;
uint16_t challengeClockIncrement = 1;

String challengeVariant = "standard";
String challengeFen = "";
String challengeColor = "random";

// --------------------------------------------------
// AI settings
// --------------------------------------------------

uint8_t aiLevel = 1;
String aiColor = "random";
uint16_t aiClockLimit = 300;
uint16_t aiClockIncrement = 1;

// --------------------------------------------------
// Move
// --------------------------------------------------

String move;

// --------------------------------------------------
// Seek settings
// --------------------------------------------------

bool seekRated = false;
uint16_t seekTime = 300;
uint16_t seekIncrement = 1;

String seekColor = "random";
String seekRatingRange = "";

// --------------------------------------------------
// Chat settings
// --------------------------------------------------

String chatMessage;
String chatRoom = "player";

// --------------------------------------------------
// Request queue
// --------------------------------------------------

requestType currentRT = NONE;
requestType lastRT[2] = { NONE, NONE };

// --------------------------------------------------
// Queue management
// --------------------------------------------------

bool lichessRequest(requestType newRt) {
    if (newRt == NONE) {
        currentRT = lastRT[0];
        lastRT[0] = lastRT[1];
        lastRT[1] = NONE;
        return true;
    }

    if (currentRT == NONE) {
        currentRT = newRt;
        return true;
    }

    if (lastRT[0] == NONE) {
        lastRT[0] = newRt;
        return true;
    }

    if (lastRT[1] == NONE) {
        lastRT[1] = newRt;
        return true;
    }

    return false;
}

// --------------------------------------------------
// URL encoding
// --------------------------------------------------

static String urlEncode(const String& value) {
    String result;
    const char hex[] = "0123456789ABCDEF";

    for (size_t i = 0; i < value.length(); i++) {
        uint8_t c = (uint8_t)value[i];

        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' ||
            c == '.' || c == '~') {
            result += (char)c;
        } else {
            result += '%';
            result += hex[c >> 4];
            result += hex[c & 0x0F];
        }
    }

    return result;
}

// --------------------------------------------------
// Form body builder
// --------------------------------------------------

static String makeForm(
    const String keys[],
    const String values[],
    size_t count
) {
    String body;

    for (size_t i = 0; i < count; i++) {
        if (i > 0) {
            body += '&';
        }

        body += urlEncode(keys[i]);
        body += '=';
        body += urlEncode(values[i]);
    }

    return body;
}

// --------------------------------------------------
// Shared HTTP request function
// Stores ordinary responses in the supplied document.
// --------------------------------------------------

static bool httpRequest(
    const String& method,
    const String& path,
    const String& body,
    JsonDocument& response
) {
    response.clear();

    if (WiFi.status() != WL_CONNECTED) {
        response["error"] = "WiFi is disconnected";
        return false;
    }

    if (lichessToken.isEmpty()) {
        response["error"] = "Missing Lichess token";
        return false;
    }

    String url = "https://lichess.org" + path;

    requestHttp.setReuse(true);

    if (!requestHttp.begin(requestClient, url)) {
        response["error"] = "HTTP begin failed";
        return false;
    }

    requestHttp.setConnectTimeout(10000);
    requestHttp.setTimeout(15000);

    requestHttp.addHeader(
        "Authorization",
        "Bearer " + lichessToken
    );

    requestHttp.addHeader("Accept", "application/json");

    int status;

    if (method == "GET") {
        status = requestHttp.GET();
    } else {
        requestHttp.addHeader(
            "Content-Type",
            "application/x-www-form-urlencoded"
        );

        status = requestHttp.sendRequest(
            method.c_str(),
            (uint8_t*)body.c_str(),
            body.length()
        );
    }

    String responseBody;

    if (status > 0) {
        responseBody = requestHttp.getString();
    }

    requestHttp.end();

    if (status <= 0) {
        response["httpStatus"] = status;
        response["error"] = "HTTP transport error";
        return false;
    }

    // Empty responses are valid for some successful requests.
    if (responseBody.isEmpty()) {
        response["httpStatus"] = status;
        return status >= 200 && status < 300;
    }

    DeserializationError error =
        deserializeJson(response, responseBody);

    if (error) {
        response.clear();
        response["httpStatus"] = status;
        response["body"] = responseBody;
    } else {
        response["_httpStatus"] = status;
    }

    return status >= 200 && status < 300;
}

// --------------------------------------------------
// Game ID validation
// --------------------------------------------------

static bool hasGameId() {
    if (gameId.isEmpty()) {
        requestDoc.clear();
        requestDoc["error"] = "Game ID is not available";
        return false;
    }

    return true;
}

// --------------------------------------------------
// Challenge another player
// --------------------------------------------------

bool createChallenge() {
    if (opponentUsername.isEmpty()) {
        requestDoc.clear();
        requestDoc["error"] = "Opponent username is missing";
        return false;
    }

    String keys[] = {
        "rated",
        "clock.limit",
        "clock.increment",
        "variant",
        "color"
    };

    String values[] = {
        challengeRated ? "true" : "false",
        String(challengeClockLimit),
        String(challengeClockIncrement),
        challengeVariant,
        challengeColor
    };

    String body = makeForm(keys, values, 5);

    if (!challengeFen.isEmpty()) {
        body += "&fen=" + urlEncode(challengeFen);
    }

    bool ok = httpRequest(
        "POST",
        "/api/challenge/" + urlEncode(opponentUsername),
        body,
        requestDoc
    );

    if (ok) {
        challengeId =
            requestDoc["challenge"]["id"] |
            requestDoc["id"] |
            "";
    }

    return ok;
}

// --------------------------------------------------
// Accept challenge
// --------------------------------------------------

bool acceptChallenge() {
    if (challengeId.isEmpty()) {
        requestDoc.clear();
        requestDoc["error"] = "Challenge ID is missing";
        return false;
    }

    return httpRequest(
        "POST",
        "/api/challenge/" + urlEncode(challengeId) + "/accept",
        "",
        requestDoc
    );
}

// --------------------------------------------------
// Decline challenge
// --------------------------------------------------

bool declineChallenge() {
    if (challengeId.isEmpty()) {
        requestDoc.clear();
        requestDoc["error"] = "Challenge ID is missing";
        return false;
    }

    return httpRequest(
        "POST",
        "/api/challenge/" + urlEncode(challengeId) + "/decline",
        "",
        requestDoc
    );
}

// --------------------------------------------------
// Challenge the Lichess AI
// --------------------------------------------------

bool challengeAI() {
    String keys[] = {
        "level",
        "clock.limit",
        "clock.increment",
        "variant",
        "color"
    };

    String values[] = {
        String(constrain(aiLevel, 1, 8)),
        String(challengeClockLimit),
        String(challengeClockIncrement),
        challengeVariant,
        challengeColor
    };

    String body = makeForm(keys, values, 5);

    if (!challengeFen.isEmpty()) {
        body += "&fen=" + urlEncode(challengeFen);
    }

    bool ok = httpRequest(
        "POST",
        "/api/challenge/ai",
        body,
        requestDoc
    );

    if (ok) {
        gameId = requestDoc["id"] | "";
        loadFEN(requestDoc["fen"]);
        setSideToMove(requestDoc["player"]);
        updatePage(101);
    }
    
    return ok;
}

// --------------------------------------------------
// Send a move
// --------------------------------------------------

bool sendMove() {
    if (!hasGameId()) return false;

    if (move.isEmpty()) {
        requestDoc["error"] = "Move is missing";
        return false;
    }

    return httpRequest(
        "POST",
        "/api/board/game/" + gameId +
            "/move/" + urlEncode(move),
        "",
        requestDoc
    );
}

bool sendMove(String givenmove) {
    move = givenmove;
    return sendMove();
}
// --------------------------------------------------
// Start seeking a game
// --------------------------------------------------

bool startSeek() {
    String keys[] = {
        "rated",
        "time",
        "increment",
        "variant",
        "color"
    };

    String values[] = {
        seekRated ? "true" : "false",
        String(challengeClockLimit),
        String(challengeClockIncrement),
        challengeVariant,
        challengeColor
    };

    String body = makeForm(keys, values, 5);

    if (!seekRatingRange.isEmpty()) {
        body += "&ratingRange=" +
                urlEncode(seekRatingRange);
    }

    return httpRequest(
        "POST",
        "/api/board/seek",
        body,
        requestDoc
    );
}

// --------------------------------------------------
// Start game event stream
// --------------------------------------------------

bool startStream() {
    if (!hasGameId()) return false;

    if (WiFi.status() != WL_CONNECTED ||
        lichessToken.isEmpty()) {
        requestDoc.clear();
        requestDoc["error"] =
            "WiFi disconnected or token missing";
        return false;
    }

    eventHttp.end();

    if (!eventHttp.begin(
            eventClient,
            "https://lichess.org/api/board/game/stream/" +
                gameId)) {
        requestDoc.clear();
        requestDoc["error"] = "Stream begin failed";
        return false;
    }

    eventHttp.setConnectTimeout(10000);
    eventHttp.setTimeout(15000);

    eventHttp.addHeader(
        "Authorization",
        "Bearer " + lichessToken
    );

    eventHttp.addHeader(
        "Accept",
        "application/x-ndjson"
    );

    int status = eventHttp.GET();

    requestDoc.clear();
    requestDoc["httpStatus"] = status;

    if (status != HTTP_CODE_OK) {
        requestDoc["error"] = "Could not start game stream";
        eventHttp.end();
        return false;
    }

    return true;
}

// --------------------------------------------------
// Resign game
// --------------------------------------------------

bool resignGame() {
    if (!hasGameId()) return false;

    return httpRequest(
        "POST",
        "/api/board/game/" + gameId + "/resign",
        "",
        requestDoc
    );
}

// --------------------------------------------------
// Abort game
// --------------------------------------------------

bool abortGame() {
    if (!hasGameId()) return false;

    return httpRequest(
        "POST",
        "/api/board/game/" + gameId + "/abort",
        "",
        requestDoc
    );
}

// --------------------------------------------------
// Offer draw
// --------------------------------------------------

bool offerDraw() {
    if (!hasGameId()) return false;

    return httpRequest(
        "POST",
        "/api/board/game/" + gameId + "/draw",
        "yes=true",
        requestDoc
    );
}

// --------------------------------------------------
// Accept draw
// --------------------------------------------------

bool acceptDraw() {
    if (!hasGameId()) return false;

    return httpRequest(
        "POST",
        "/api/board/game/" + gameId + "/draw",
        "yes=true",
        requestDoc
    );
}

// --------------------------------------------------
// Decline draw
// --------------------------------------------------

bool declineDraw() {
    if (!hasGameId()) return false;

    return httpRequest(
        "POST",
        "/api/board/game/" + gameId + "/draw",
        "yes=false",
        requestDoc
    );
}

// --------------------------------------------------
// Offer takeback
// --------------------------------------------------

bool offerTakeback() {
    if (!hasGameId()) return false;

    return httpRequest(
        "POST",
        "/api/board/game/" + gameId + "/takeback",
        "yes=true",
        requestDoc
    );
}

// --------------------------------------------------
// Accept takeback
// --------------------------------------------------

bool acceptTakeback() {
    if (!hasGameId()) return false;

    return httpRequest(
        "POST",
        "/api/board/game/" + gameId + "/takeback",
        "yes=true",
        requestDoc
    );
}

// --------------------------------------------------
// Decline takeback
// --------------------------------------------------

bool declineTakeback() {
    if (!hasGameId()) return false;

    return httpRequest(
        "POST",
        "/api/board/game/" + gameId + "/takeback",
        "yes=false",
        requestDoc
    );
}

// --------------------------------------------------
// Claim victory
// --------------------------------------------------

bool claimVictory() {
    if (!hasGameId()) return false;

    return httpRequest(
        "POST",
        "/api/board/game/" + gameId + "/claim-victory",
        "",
        requestDoc
    );
}

// --------------------------------------------------
// Send chat message
// --------------------------------------------------

bool sendChatMessage() {
    if (!hasGameId()) {
        chatDoc.clear();
        chatDoc["error"] = "Game ID is not available";
        return false;
    }

    if (chatMessage.isEmpty()) {
        chatDoc.clear();
        chatDoc["error"] = "Chat message is empty";
        return false;
    }

    String keys[] = { "room", "text" };
    String values[] = { chatRoom, chatMessage };

    String body = makeForm(keys, values, 2);

    return httpRequest(
        "POST",
        "/api/board/game/" + gameId + "/chat",
        body,
        chatDoc
    );
}

// --------------------------------------------------
// Get chat messages
// --------------------------------------------------

bool getChatMessages() {
    if (!hasGameId()) {
        chatDoc.clear();
        chatDoc["error"] = "Game ID is not available";
        return false;
    }

    return httpRequest(
        "GET",
        "/api/board/game/" + gameId + "/chat",
        "",
        chatDoc
    );
}

// --------------------------------------------------
// Get account and public user profiles
// --------------------------------------------------

bool getAccountInfo() {
    bool ok = true;

    if (!httpRequest(
            "GET",
            "/api/account",
            "",
            accountDoc[0])) {
        ok = false;
    }

    if (!opponentUsername.isEmpty()) {
        if (!httpRequest(
                "GET",
                "/api/user/" + urlEncode(opponentUsername),
                "",
                accountDoc[1])) {
            ok = false;
        }
    } else {
        accountDoc[1].clear();
    }

    if (!accountUsername.isEmpty()) {
        if (!httpRequest(
                "GET",
                "/api/user/" + urlEncode(accountUsername),
                "",
                accountDoc[2])) {
            ok = false;
        }
    } else {
        accountDoc[2].clear();
    }

    return ok;
}

// --------------------------------------------------
// Read event stream
// Kept separate from ordinary HTTP requests.
// --------------------------------------------------

bool getEvent() {
    if (!eventClient.connected() ||
        !eventClient.available()) {
        return false;
    }

    String line = eventClient.readStringUntil('\n');
    line.trim();

    if (line.isEmpty() || !line.startsWith("{")) {
        return false;
    }

    Serial.print("Incoming Clean JSON: ");
    Serial.println(line);

    eventDoc.clear();

    DeserializationError error =
        deserializeJson(eventDoc, line);

    if (error) {
        Serial.print("JSON Parse Error: ");
        Serial.println(error.c_str());
        return false;
    }

    // action part. find the type key and do the task accordingly. 
    // first try at first branch.
    const char* type = eventDoc["type"] | "";

    if (strcmp(type, "gameFull") == 0) {
        // Initialize the game
    }
    else if (strcmp(type, "gameState") == 0) {
        // Compare moves, update clocks and status
    }
    else if (strcmp(type, "opponentGone") == 0) {
        // Update opponent presence
    }
    else if (strcmp(type, "chatLine") == 0) {
        // Optionally display chat
    }
    else if (strcmp(type, "challenge") == 0) {
        // Handle a challenge
    }
    else if (strcmp(type, "challengeCanceled") == 0) {
        // Clear canceled challenge
    }
    else if (strcmp(type, "challengeDeclined") == 0) {
        // Handle declined challenge
    }
    else if (strcmp(type, "gameStart") == 0) {
        // Save game ID and start game stream
    }
    else if (strcmp(type, "gameFinish") == 0) {
        // Display result and return to menu
    }
}

// --------------------------------------------------
// Execute queued request
// Advances the queue after the request completes.
// --------------------------------------------------

bool lichessSendRequest() {
    if (currentRT == NONE) return false;

    lichessRequest(NONE); // this should be done so that even if there is any error it would not send the request in a loop and load the server.

    switch (currentRT) {
        case CHALLENGE: return createChallenge();

        case ACCEPT_CHALLENGE: return acceptChallenge();

        case DECLINE_CHALLENGE: return declineChallenge();

        case CHALLENGE_AI: return challengeAI();

        case MOVE: return sendMove();

        case SEEK: return startSeek();

        case STREAM: return startStream();

        case RESIGN: return resignGame();

        case ABORT: return abortGame();

        case OFFER_DRAW: return offerDraw();

        case ACCEPT_DRAW: return acceptDraw();

        case DECLINE_DRAW: return declineDraw();

        case OFFER_TAKEBACK: return offerTakeback();

        case ACCEPT_TAKEBACK: return acceptTakeback();

        case DECLINE_TAKEBACK: return declineTakeback();

        case CLAM_VICTORY: return claimVictory();

        case CHAT: return sendChatMessage();

        case GET_CHAT: return getChatMessages();

        case GET_ACCOUNT: return getAccountInfo();

        case AUTHENTICATION: return runLichessPairing();
        
        case NONE:
        default: return false;
    }
}