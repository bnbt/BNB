#ifndef __RestBNB_H__
#define __RestBNB_H__

#include "Arduino.h"
#include "SoftwareSerial.h"

//#define DEBUG

struct Response {
public:
    int httpStatus;
    int length;
    const char *data;
};

class RestBNB {
public:
    RestBNB(SoftwareSerial &espSerial, byte chpdPin);
    bool begin(const uint32_t& baud = 9600);
    bool connect(const char *ssid, const char *password);
    void setHost(const char *hostname, const uint32_t& port);
    void setDeviceId(const char *deviceId);
    bool getConfigData();
    int getLastResponseStatus() const;
    bool setConfig(char **&states, byte **&colors, byte &stateCount, byte &currentState) const;

    bool putState(const byte& stateId, const unsigned long& code);
    const char* getUserName() const;

    bool getState();
    void setCurrentState(byte &currentState) const;

private:
    void clearBuffer() const;
    bool hardResetESP() const;
    int readESP() const;
    bool expectResponse(const char *str, uint32_t timeout = 1000, bool clearBuffer = true) const;
    bool resetESP() const;
    bool setMode() const;
    bool connectAP() const;
    bool setSingleConnection() const;
    void createResponse();
    bool setConnection() const;
    bool setSize(int size) const;
    bool sendGetDeviceConfig() const;
    void closeConnection() const;
    bool sendPutDeviceState(const byte& stateId, const unsigned long& code) const;
    bool sendGetDeviceState() const;

    byte readByte(int &position) const;
    char *readString(int &position) const;

    SoftwareSerial *espSerial;
    byte chpdPin;
    uint32_t baud;
    byte mode = 1;
    Response *lastResponse;

    const char *ssid;
    const char *password;
    const char *host;
    uint32_t port;
    const char *deviceId;
};

#endif /* #ifndef __RestBNB_H__ */
