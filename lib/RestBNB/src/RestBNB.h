#ifndef __RestBNB_H__
#define __RestBNB_H__

#include "Arduino.h"
#include "SoftwareSerial.h"

#define DEBUG
#define MAX_RETRY 3

struct Response {
public:
    int httpStatus;
    const char *data;
};

class RestBNB {
public:
    RestBNB(SoftwareSerial &espSerial, byte chpdPin);
    void setHost(const char *hostname, uint32_t port);
    bool begin(uint32_t baud = 9600);
    bool connect(const char *ssid, const char *password);
    void setDeviceId(const char *deviceId);
    void closeConnection();
    bool getConfigData();
    int getLastResponseStatus();
    bool setConfig(char *states[], byte colors[][3], byte &stateCount);

private:
    void clearBuffer();
    bool hardResetESP();
    int readESP();
    bool expectResponse(const char *str, uint32_t timeout = 1000, bool clearBuffer = true);
    bool resetESP();
    bool setMode();
    bool connectAP();
    bool setSingleConnection();
    void readConfig();
    bool setConnection();
    bool setSize(int size);
    bool sendGetDeviceConfig();

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
