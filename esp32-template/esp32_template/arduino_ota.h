#ifndef ARDUINO_OTA_H
#define ARDUINO_OTA_H


class arduino_ota
{
public:
    arduino_ota();

    void setup(ESP8266WebServer* pws);
    void loop(ESP8266WebServer* pws);
};

#endif // ARDUINO_OTA_H
