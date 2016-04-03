#include "RestBNB.h"

RestBNB::RestBNB(SoftwareSerial &espSerial, byte chpdPin) {
    this->espSerial = &espSerial;
    this->chpdPin = chpdPin;
    pinMode(this->chpdPin, OUTPUT);
}

bool RestBNB::hardResetESP() {
#ifdef DEBUG
    Serial.println("Resetting ESP.");
#endif
    digitalWrite(this->chpdPin, LOW);
    delay(500);
    digitalWrite(this->chpdPin, HIGH);
#ifdef DEBUG
    Serial.println("ESP reset.");
#endif
    return this->expectResponse("Ai-Thinker");
}

void RestBNB::clearBuffer(){
#ifdef DEBUG
    Serial.println();
    Serial.println("=== CLEAR BUFFER ===");
#endif
    while(this->espSerial->available() > 0) {
#ifdef DEBUG
        Serial.write(this->espSerial->read());
#else
        this->espSerial->read();
#endif
    }
#ifdef DEBUG
    Serial.println();
    Serial.println("=== BUFFER EMPTY ===");
#endif
}

int RestBNB::readESP() {
    // improve read trick
    delay(1);
#ifdef DEBUG
    int read = this->espSerial->read();
    Serial.write(read);
    return read;
#else
    return this->espSerial->read();
#endif
}

bool RestBNB::expectResponse(const char *str, uint32_t timeout, bool clearBuffer) {
#ifdef DEBUG
    Serial.println();
    Serial.print("=== EXPECTING \"");
    Serial.print(str);
    Serial.println("\" ===");
#endif
    byte i;
    byte size = strlen(str);
    int read;
    unsigned long start = millis();

    while (millis() - start < timeout) {
        while(this->espSerial->available() > 0) {
            for (i=0; i<size; ++i) {
                read = this->readESP();
                if (read != str[i]) { break; }
                if (i == (size-1)) {
#ifdef DEBUG
                    Serial.println();
                    Serial.print("=== EXPECTING OK: ");
                    Serial.print(millis() - start);
                    Serial.println(" ===");
#endif
                    if (clearBuffer) { this->clearBuffer(); }
                    return true;
                }
            }
        }
    }
#ifdef DEBUG
    Serial.println();
    Serial.println("=== EXPECTING NOK ===");
#endif
    return false;
}

bool RestBNB::resetESP() {
#ifdef DEBUG
    Serial.println();
    Serial.println("=== SOFT RESET ===");
#endif
    this->clearBuffer();
    this->espSerial->println("AT+RST");
#ifdef DEBUG
    Serial.println();
    Serial.println("=== RESET ===");
    Serial.println();
#endif
    return this->expectResponse("ready");
}

bool RestBNB::begin(uint32_t baud) {
    this->baud = baud;
    this->espSerial->begin(this->baud);
    return this->hardResetESP() && this->resetESP();
}

bool RestBNB::setMode() {
    this->clearBuffer();
    this->espSerial->print("AT+CWMODE=");
    this->espSerial->println(this->mode);

    return this->expectResponse("OK", 200);
}

bool RestBNB::connectAP() {
    this->clearBuffer();
    this->espSerial->print("AT+CWJAP=\"");
    this->espSerial->print(this->ssid);
    this->espSerial->print("\",\"");
    this->espSerial->print(this->password);
    this->espSerial->println("\"");

    return this->expectResponse("OK", 10000);
}

bool RestBNB::setSingleConnection() {
    this->clearBuffer();
    this->espSerial->println("AT+CIPMUX=0");

    return this->expectResponse("OK", 100);
}

bool RestBNB::connect(const char *ssid, const char *password) {
    this->ssid = ssid;
    this->password = password;
    return this->setMode() && this->connectAP() && this->setSingleConnection();
}

void RestBNB::setHost(const char *hostname, uint32_t port) {
    this->host = hostname;
    this->port = port;
}

void RestBNB::setDeviceId(const char *deviceId) {
    this->deviceId = deviceId;
}

bool RestBNB::setConnection() {
    this->clearBuffer();
    this->espSerial->print("AT+CIPSTART=\"TCP\",\"");
    this->espSerial->print(this->host);
    this->espSerial->print("\",");
    this->espSerial->println(this->port);

    return this->expectResponse("OK", 200);
}

bool RestBNB::setSize(int size) {
    this->clearBuffer();
    this->espSerial->print("AT+CIPSEND=");
    this->espSerial->println(size);

    return this->expectResponse(">");
}

bool RestBNB::sendGetDeviceConfig() {
    this->espSerial->println("GET /device/config HTTP/1.1");
    this->espSerial->print("Device: ");
    this->espSerial->println(this->deviceId);
    this->espSerial->println();

    return this->expectResponse("HTTP/1.1 ", 1000, false);
}

void RestBNB::closeConnection() {
    this->clearBuffer();
    this->espSerial->println("AT+CIPCLOSE");
    this->expectResponse("OK", 100);
}

void RestBNB::readConfig() {
    int read;
    /**
     * @brief step
     * 1 -> read status
     * 2 -> read untill data (~)
     * 3 -> read data untill end (~)
     */
    byte step = 1;
    this->lastResponse = new Response();
    this->lastResponse->httpStatus = 0;
    this->lastResponse->data = "";
    String *data = new String();
#ifdef DEBUG
    Serial.println("=== PARSING ===");
#endif
    while(this->espSerial->available() > 0) {
        if (step == 1) {
            this->lastResponse->httpStatus = (this->readESP()-48)*100 +
                    (this->readESP()-48)*10 + this->readESP()-48;
            ++step;
        } else if (step == 2) {
            // read untill '~'
            read = this->readESP();
            if (read == 126) { ++step; }
        } else if (step == 3) {
            read = this->readESP();

            if (read != 126) {
                data->concat(char(read));
            } else {
                this->clearBuffer();
                this->lastResponse->data = data->c_str();
                return;
            }
        }
    }
}

bool RestBNB::getConfigData() {
    bool ok = this->setConnection() &&
            this->setSize(41 + strlen(this->deviceId)) &&
            this->sendGetDeviceConfig();
    if (ok) {
        this->readConfig();
        Serial.print(this->lastResponse->data);
        return ok && this->lastResponse->httpStatus;
    }

    return false;
}

int RestBNB::getLastResponseStatus() {
    if (this->lastResponse && this->lastResponse->httpStatus) {
        return this->lastResponse->httpStatus;
    }
    return 0;
}

bool RestBNB::setConfig(char *states[], byte colors[][3], byte &stateCount) {
    int length = strlen(this->lastResponse->data);
    int i = 0;
    byte currentState = 0;
    char current;
    byte statesCount = 0;
    byte j;

    /**
     * @brief step
     * 0 -> read current state
     * 1 -> read state name
     * 2 -> read red color
     * 3 -> read green color
     * 4 -> read blue color
     */
    byte step = 0;
    while(i < length) {
        if (step == 0) {
            Serial.println();
            Serial.println("Step 0");
            current = this->lastResponse->data[i];
            while(i<length && current != ',') {
                currentState = currentState*10 + current-48;
                current = this->lastResponse->data[++i];
            }
            Serial.println(currentState);
            step = 1;
        } else if (step == 1) {
            Serial.println();
            Serial.println("Step 1");
            j = 0;
            current = this->lastResponse->data[i];
            while(i<length && current != ',') {
                states[statesCount][j] = current;
                ++j;
                current = this->lastResponse->data[++i];
            }
            states[statesCount][j] = '\0';
            step = 2;
            Serial.println(states[statesCount]);
        } else if (step == 2) {
            Serial.println();
            Serial.println("Step 2");
            colors[statesCount][0] = 0;
            current = this->lastResponse->data[i];
            while(i<length && current != ',') {
                colors[statesCount][0] = colors[statesCount][0]*10 + current-48;
                current = this->lastResponse->data[++i];
            }
            Serial.println(colors[statesCount][0]);
            step = 3;
        } else if (step == 3) {
            Serial.println();
            Serial.println("Step 3");
            colors[statesCount][1] = 0;
            current = this->lastResponse->data[i];
            while(i<length && current != ',') {
                colors[statesCount][1] = colors[statesCount][1]*10 + current-48;
                current = this->lastResponse->data[++i];
            }
            Serial.println(colors[statesCount][1]);
            step = 4;
        } else if (step == 4) {
            Serial.println();
            Serial.println("Step 4");
            colors[statesCount][2] = 0;
            current = this->lastResponse->data[i];
            while(i<length && current != ',') {
                colors[statesCount][2] = colors[statesCount][2]*10 + current-48;
                current = this->lastResponse->data[++i];
            }
            step = 1;
            ++statesCount;
            Serial.println(colors[statesCount][2]);
            Serial.println(statesCount);
        }
        ++i;
    }
    stateCount = statesCount;
    Serial.println();
    Serial.println(statesCount);
    Serial.println(currentState);
    Serial.println(states[0]);
    Serial.println(colors[0][0]);
}
