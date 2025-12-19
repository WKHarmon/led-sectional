#include "metar_listener.h"

MetarListener::MetarListener()
    : callback(nullptr)
    , arrayDepth(0)
    , objectDepth(0)
    , metarCount(0)
    , inCloudsArray(false) {
}

void MetarListener::setCallback(MetarCallback cb) {
    callback = cb;
}

void MetarListener::reset() {
    currentMetar.reset();
    currentKey = "";
    arrayDepth = 0;
    objectDepth = 0;
    metarCount = 0;
    inCloudsArray = false;
}

int MetarListener::getMetarCount() const {
    return metarCount;
}

void MetarListener::whitespace(char c) {
    // Ignore whitespace
}

void MetarListener::startDocument() {
    reset();
}

void MetarListener::endDocument() {
    // Nothing to do
}

void MetarListener::startArray() {
    arrayDepth++;

    // Check if we're entering the clouds array
    if (currentKey == "clouds") {
        inCloudsArray = true;
    }
}

void MetarListener::endArray() {
    // Check if we're exiting the clouds array
    if (inCloudsArray && arrayDepth == 2) {
        inCloudsArray = false;
    }

    arrayDepth--;
}

void MetarListener::startObject() {
    objectDepth++;

    // When we enter a METAR object (depth 1 in the array, so objectDepth becomes 1)
    // Note: The JSON structure is an array of objects, so:
    // - arrayDepth 1 = main array
    // - objectDepth 1 = individual METAR object
    if (objectDepth == 1 && arrayDepth == 1) {
        currentMetar.reset();
    }
}

void MetarListener::endObject() {
    // When we complete a METAR object at the top level
    if (objectDepth == 1 && arrayDepth == 1 && !inCloudsArray) {
        // We've completed parsing a METAR, call the callback
        if (callback && currentMetar.icaoId.length() > 0) {
            callback(currentMetar);
            metarCount++;
        }
    }

    objectDepth--;
}

void MetarListener::key(String key) {
    currentKey = key;
}

void MetarListener::value(String value) {
    // Only process values at the METAR object level (not nested in clouds, etc.)
    if (objectDepth == 1 && !inCloudsArray) {
        if (currentKey == "icaoId") {
            currentMetar.icaoId = value;
        } else if (currentKey == "fltCat") {
            currentMetar.fltCat = value;
        } else if (currentKey == "wspd") {
            currentMetar.wspd = value.toInt();
        } else if (currentKey == "wgst") {
            currentMetar.wgst = value.toInt();
        } else if (currentKey == "wxString") {
            currentMetar.wxString = value;
        }
    }

    // Clear key after processing value
    currentKey = "";
}
