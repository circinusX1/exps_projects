#include "eeprom.h"


config_t eeprom_t::Conf = {
    0x11,
    1,
    false,
    false,
    false,
    "-3",
    "https://meeiot.mine.nu",
    "put/5bdd8abe7b78305b7f4be6d52b850053362f945e5a0f915347a425/",              // remote
    "get/5bdd8abe7b78305b7f4be6d52b850053362f945e5a0f915347a425/",              // remote
    "6D F9 FD AA A1 60 42 96 0E C5 29 D5 79 60 07 1F 90 21 4D 99",                                     // finger
    443,     // host port
    "",
    "",
    "",
    "",
    0,
    10,     // seconds
    0xFF,
    "",     // ip
    "192,168,1,1",
    "255,255,255,0",
    {"8.8.8.8","4.4.4.4"},
    "",
    "",
};

/*
const char* host = "https://api.github.com";
const char* fingerpr = "CF 05 98 89 CA FF 8E D8 5E 5C E0 C2 E4 F7 E6 C3 C7 50 DD 5C";

WiFiClientSecure client;
client.connect(host, httpsPort);

if (client.verify(fingerpr, host))
{
    http.begin(client, host);

    String payload;
    if (http.GET() == HTTP_CODE_OK)
        payload = http.getString();
}
else
{
  Serial.println("certificate doesn't match");
}
*/
