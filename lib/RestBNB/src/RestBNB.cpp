/*
 *  RestBNB.cpp
 *
 *  Written by Vlad-Iliescu (https://github.com/Vlad-Iliescu) for BNBT TEAM
 *  (https://github.com/BNBT).
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "RestBNB.h"

RestBNB::RestBNB(SoftwareSerial &espSerial, byte chpdPin) {
    this->espSerial = &espSerial;
    this->chpdPin = chpdPin;
    pinMode(this->chpdPin, OUTPUT);
}

bool RestBNB::hardResetESP() const {
#ifdef DEBUG
    Serial.println(F("Resetting ESP."));
#endif
    digitalWrite(this->chpdPin, LOW);
    delay(500);
    digitalWrite(this->chpdPin, HIGH);
#ifdef DEBUG
    Serial.println(F("ESP reset."));
#endif
    return this->expectResponse("Ai-Thinker");
}

void RestBNB::clearBuffer() const {
#ifdef DEBUG
    Serial.println();
    Serial.println(F("=== CLEAR BUFFER ==="));
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
    Serial.println(F("=== BUFFER EMPTY ==="));
#endif
}

int RestBNB::readESP() const {
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

bool RestBNB::expectResponse(const char *str, uint32_t timeout, bool clearBuffer) const {
#ifdef DEBUG
    Serial.println();
    Serial.print(F("=== EXPECTING \""));
    Serial.print(str);
    Serial.println(F("\" ==="));
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
                    Serial.print(F("=== EXPECTING OK: "));
                    Serial.print(millis() - start);
                    Serial.println(F(" ==="));
#endif
                    if (clearBuffer) { this->clearBuffer(); }
                    return true;
                }
            }
        }
    }
#ifdef DEBUG
    Serial.println();
    Serial.println(F("=== EXPECTING NOK ==="));
#endif
    return false;
}

bool RestBNB::resetESP() const {
#ifdef DEBUG
    Serial.println();
    Serial.println(F("=== SOFT RESET ==="));
#endif
    this->clearBuffer();
    this->espSerial->println("AT+RST");
#ifdef DEBUG
    Serial.println();
    Serial.println(F("=== RESET ==="));
    Serial.println();
#endif
    return this->expectResponse("ready");
}

bool RestBNB::begin(const uint32_t& baud) {
    this->baud = baud;
    this->espSerial->begin(this->baud);
    return this->hardResetESP() && this->resetESP();
}

bool RestBNB::setMode() const {
    this->clearBuffer();
    this->espSerial->print("AT+CWMODE=");
    this->espSerial->println(this->mode);

    return this->expectResponse("OK", 200);
}

bool RestBNB::connectAP() const {
    this->clearBuffer();
    this->espSerial->print("AT+CWJAP=\"");
    this->espSerial->print(this->ssid);
    this->espSerial->print("\",\"");
    this->espSerial->print(this->password);
    this->espSerial->println("\"");

    return this->expectResponse("OK", 10000);
}

bool RestBNB::setSingleConnection() const {
    this->clearBuffer();
    this->espSerial->println("AT+CIPMUX=0");

    return this->expectResponse("OK", 100);
}

bool RestBNB::connect(const char *ssid, const char *password) {
    this->ssid = ssid;
    this->password = password;
    return this->setMode() && this->connectAP() && this->setSingleConnection();
}

void RestBNB::setHost(const char *hostname, const uint32_t &port) {
    this->host = hostname;
    this->port = port;
}

void RestBNB::setDeviceId(const char *deviceId) {
    this->deviceId = deviceId;
}

bool RestBNB::setConnection() const {
    this->clearBuffer();
    this->espSerial->print("AT+CIPSTART=\"TCP\",\"");
    this->espSerial->print(this->host);
    this->espSerial->print("\",");
    this->espSerial->println(this->port);

    return this->expectResponse("OK");
}

bool RestBNB::setSize(int size) const {
    this->clearBuffer();
    this->espSerial->print("AT+CIPSEND=");
    this->espSerial->println(size);

    return this->expectResponse(">");
}

bool RestBNB::sendGetDeviceConfig() const {
    this->espSerial->println("GET /device/config HTTP/1.1");
    this->espSerial->print("Device: ");
    this->espSerial->println(this->deviceId);
    this->espSerial->println();

    return this->expectResponse("HTTP/1.1 ", 2000, false);
}

void RestBNB::closeConnection() const {
    this->clearBuffer();
    this->espSerial->println("AT+CIPCLOSE");
    this->expectResponse("OK", 100);
}

void RestBNB::createResponse() {
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
    this->lastResponse->length = 0;
    String *data = new String();
#ifdef DEBUG
    Serial.println(F("=== PARSING ==="));
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
                this->lastResponse->length = data->length();
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
        this->createResponse();
#ifdef DEBUG
        Serial.println(F("=== GOT CONFIG RESPONSE ==="));
        Serial.print(F("HTTP status = "));
        Serial.println(this->lastResponse->httpStatus);
        Serial.print(F("Length = "));
        Serial.println(this->lastResponse->length);
        Serial.print(F("body = "));
        Serial.println(this->lastResponse->data);
        Serial.println(F("=== END CONFIG GOT RESPONSE ==="));
#endif
        return ok && this->lastResponse->httpStatus;
    }

    return false;
}

int RestBNB::getLastResponseStatus() const {
    if (this->lastResponse && this->lastResponse->httpStatus) {
        return this->lastResponse->httpStatus;
    }
    return 0;
}

byte RestBNB::readByte(int &position) const{
    if (position >= this->lastResponse->length) {
        return 0;
    }

    byte currentState = 0;
    char iter = this->lastResponse->data[position];
    while (position < this->lastResponse->length && iter != ',') {
        currentState = currentState*10 + iter - 48;
        iter = this->lastResponse->data[++position];
    }
    ++position;

    return currentState;
}

char *RestBNB::readString(int &position) const {
    if (position >= this->lastResponse->length) {
        return '\0';
    }

    char iter = this->lastResponse->data[position];
    char *buffer = new char[25];
    byte len = 0;
    while (position < this->lastResponse->length && iter != ',') {
        buffer[len] = iter;
        ++len;
        iter = this->lastResponse->data[++position];
    }
    buffer[len] = '\0';
    ++position;

    return buffer;
}

bool RestBNB::setConfig(char **&states, byte** &colors, byte &stateCount, byte &currentState) const{
    int i = 0;
    byte j = 0;
    currentState = this->readByte(i);
#ifdef DEBUG
    Serial.println();
    Serial.print(F("=== currentState = "));
    Serial.print(currentState);
    Serial.println(F(" ==="));
#endif

    stateCount = this->readByte(i);
#ifdef DEBUG
    Serial.print(F("=== stateCount = "));
    Serial.print(stateCount);
    Serial.println(F(" ==="));
#endif
    states = new char*[stateCount];
    colors = new byte*[stateCount];

    for(j=0; j<stateCount; ++j) {
        states[j] = this->readString(i);
        colors[j] = new byte[3];
        colors[j][0] = readByte(i);
        colors[j][1] = readByte(i);
        colors[j][2] = readByte(i);
    }
#ifdef DEBUG
    for(j=0; j<stateCount; ++j) {
        Serial.print(F("=== states["));
        Serial.print(j);
        Serial.print(F("] = "));
        Serial.print(states[j]);
        Serial.println(F(" ==="));

        Serial.print(F("=== colors["));
        Serial.print(j);
        Serial.print(F("] = ("));
        for (byte m=0; m<3; ++m) {
            Serial.print(colors[j][m]);
            if (m != 2) { Serial.print(F(", ")); }
        }
        Serial.println(F(") ==="));
    }
#endif

    return true;
}

bool RestBNB::sendPutDeviceState(const byte& stateId, const unsigned long& code) const {
    this->espSerial->print("PUT /device/state?s=");
    this->espSerial->print(stateId);
    this->espSerial->print("&u=");
    this->espSerial->print(code);
    this->espSerial->print(" HTTP/1.1\r\nDevice: ");
    this->espSerial->println(this->deviceId);
    this->espSerial->println();

    return this->expectResponse("HTTP/1.1 ", 2000, false);
}

bool RestBNB::putState(const byte& stateId, const unsigned long& code) {
    this->closeConnection();

    int length = 46 + strlen(this->deviceId) +
            String(stateId).length() + String(code).length();

    bool ok = this->setConnection() &&
            this->setSize(length) &&
            this->sendPutDeviceState(stateId, code);
    if (ok) {
        this->createResponse();
#ifdef DEBUG
        Serial.println(F("=== GOT PUT STATE RESPONSE ==="));
        Serial.print(F("HTTP status = "));
        Serial.println(this->lastResponse->httpStatus);
        Serial.print(F("Length = "));
        Serial.println(this->lastResponse->length);
        Serial.print(F("body = "));
        Serial.println(this->lastResponse->data);
        Serial.println(F("=== END GOT RESPONSE ==="));
#endif
        return ok && this->lastResponse->httpStatus;
    }

    return false;
}

const char *RestBNB::getUserName() const {
    return this->lastResponse->data;
}

bool RestBNB::sendGetDeviceState() const {
    this->espSerial->print("GET /device/state HTTP/1.1\r\nDevice: ");
    this->espSerial->println(this->deviceId);
    this->espSerial->println();

    return this->expectResponse("HTTP/1.1 ", 2000, false);
}

bool RestBNB::getState() {
    this->closeConnection();
    bool ok = this->setConnection() &&
            this->setSize(40 + strlen(this->deviceId)) &&
            this->sendGetDeviceState();

    if (ok) {
        this->createResponse();
#ifdef DEBUG
        Serial.println(F("=== GOT GET STATE RESPONSE ==="));
        Serial.print(F("HTTP status = "));
        Serial.println(this->lastResponse->httpStatus);
        Serial.print(F("Length = "));
        Serial.println(this->lastResponse->length);
        Serial.print(F("body = "));
        Serial.println(this->lastResponse->data);
        Serial.println(F("=== END GOT RESPONSE ==="));
#endif
        return ok && this->lastResponse->httpStatus;
    }

    return false;
}

void RestBNB::setCurrentState(byte &currentState) const {
    int i = 0;
    currentState = this->readByte(i);
}
