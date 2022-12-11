#include "esp32_full.h"
#include <Wire.h>

int   RELAY = 12;

class MyEsp : public esp32_full
{
public:
    MyEsp(int p):esp32_full(p)
    {
        _regain = millis();
    };
    virtual ~MyEsp()
    {
    };

    void    rebooting()
    {

    }

    void  user_loop(unsigned int loop)
    {
        delay(10);
        return;
    }

    void  page_request(ESP8266WebServer* srv,
                       String& page)
    {
        page+= _start_html();
        if(srv->hasArg("reboot"))
        {
            REBOOT();
        }
        if(srv->hasArg("relay"))
        {
            if(srv->arg("relay")=="1"){
                if(RELAY)digitalWrite(RELAY, HIGH);
                __Ramm.relay_state = LOW;
                _relay_state=HIGH;
            }
            if(srv->arg("relay")=="0"){
                if(RELAY)digitalWrite(RELAY, LOW);
                __Ramm.relay_state = HIGH;
                _relay_state=LOW;
            }
        }
        if(srv->hasArg("gpio"))
        {
            int nLED = srv->arg("gpio").toInt();
            pinMode(nLED, OUTPUT);
            digitalWrite(nLED, LOW);
            delay (500);
            digitalWrite(nLED, HIGH);
            delay (500);
            digitalWrite(nLED, LOW);

        }
        page += "<font size='4em'>";
        page += "<li><a href='?relay=1'>TURN ON</a>";
        page += "<li><a href='?relay=0'>TURN OFF</a>";
        page += "<li>LED:" + String(LED);
        page += "<li>RELAY:" + String(RELAY);
        if(_relay_state==LOW)
            page += "<li> LED OFF";
        else
            page += "<li> LED ON";
        page+="</font>";

        page +=  _end_html();

    }
    String _page;
    int    _lop = 0;
    bool   _out = false;

    uint32_t _regain = millis();
} TheEsp(80);



void setup() {
    Serial.println("entering setup1");
    TheEsp.setup();
    Serial.println("entering setup2");
    if(RELAY)pinMode(RELAY, OUTPUT);
    if(LED)pinMode(LED, OUTPUT);
    if (RELAY)digitalWrite(RELAY, __Ramm.relay_state);
    Serial.println("exiting setup3");
}

void loop() {

    TheEsp.loop();
}
