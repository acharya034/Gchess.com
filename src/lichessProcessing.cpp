#include "lichessProcessing.h"

bool lichessRequest(requestType newRt = NONE) {
    if (newRt == NONE) {
        currentRT = lastRT[0];
        lastRT[0] = lastRT[1];
        lastRT[1] = NONE;
        return true;
    }

    if (currentRT == NONE){
        currentRT = newRt;
        return true;
    }
    else if(lastRT[0] == NONE) {
        lastRT[0] = newRt;
        return true;
    }
    else if(lastRT[1] == NONE) {
        lastRT[1] = newRt;
        return true;
    }
    else {
        return false;
    }
}

bool getEvent(JsonDocument& doc, String& type) {
    // 1. Verify if the raw TCP socket has data waiting to be read
    if (!eventClient.connected() || !eventClient.available()) {
        return false;
    }
    
    // 2. Read the raw text stream until a newline break
    String line = eventClient.readStringUntil('\n');
    line.trim(); // Trim spaces and empty characters
    
    // 3. FIX CHUNKS: If line is a heartbeat or a chunk size length (e.g. 1ff, 260), completely skip it
    if (line.length() == 0 || !line.startsWith("{")) {
        return false; 
    }

    // Debugging printout to see clean JSON arriving in your Serial Monitor
    Serial.print("Incoming Clean JSON: ");
    Serial.println(line);

    // 4. Reset JSON document memory tracking safely
    doc.clear();

    // 5. Run standard deserialization
    DeserializationError error = deserializeJson(doc, line);
    if (error) {
        Serial.print("JSON Parse Error: ");
        Serial.println(error.c_str());
        return false;
    }

    // 6. Safely extract standard type markers securely without creating null pointer exceptions
    type = doc["type"].is<const char*>() ? doc["type"].as<String>() : "unknown";
        
    return true;
}