#include "esp32_full.h"
#include <Wire.h>

int   RELAY = 12;
int   BUTTON = 14; // = 0; // no button

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
        if(_button == -1)     // power on pressed
        {
            for(int i=0;i<10;i++)
            {
                digitalWrite(LED, LOW);
                delay (100);
                digitalWrite(LED, HIGH);
                delay (100);
            }
            this->force_ap();
            _button = 0;
        }
        delay(10);
        if(get_button_state()==LOW)
        {
           if(_down++>50)
           {
               __Ramm.relay_state=!__Ramm.relay_state;
               set_relay(__Ramm.relay_state==1);
               _down=0;
           }
        }
        else
        {
           _down=0;  
        }
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
               set_relay(true);
            }
            if(srv->arg("relay")=="0"){
               set_relay(false);
            }
        }
        if(srv->hasArg("button"))
        {
            int nBUT = srv->arg("button").toInt();
            pinMode(nBUT, INPUT);
            BUTTON = nBUT;
        }
        if(srv->hasArg("gpio"))
        {
            int nLED = srv->arg("gpio").toInt();
            pinMode(nLED, OUTPUT);
            for(int i=0; i <5; i++)
            {
              digitalWrite(nLED, LOW);
              delay (200);
              digitalWrite(nLED, HIGH);
              delay (200);
            }
        }
        page += "<font size='4em'>";
        page += "<li><a href='?relay=1'>TURN ON</a>";
        page += "<li><a href='?relay=0'>TURN OFF</a>";
        page += "<li>LED:" + String(LED);
        page += "<li>RELAY:" + String(RELAY);
        page += "<li>BUTTON:" + String(BUTTON) + " = " + (get_button_state());
        if(__Ramm.relay_state==LOW)
            page += "<li> LED OFF";
        else
            page += "<li> LED ON";
        page+="</font>";
        page +=  _end_html();
    }

    void set_relay(bool on) // globe plug
    {
        if(on)
        {
          if(RELAY)digitalWrite(RELAY, HIGH);
          __Ramm.relay_state = 1;
        }
        else
        {
          if(RELAY)digitalWrite(RELAY, LOW);
          __Ramm.relay_state = 0;
        }
    }

    int get_button_state() // globe plug
    {
        return digitalRead(BUTTON);
    }
    
    String _page;
    int    _lop = 0;
    bool   _out = false;
    int    _button = 0;
    int    _down = 0;
    uint32_t _regain = millis();
} TheEsp(80);



void setup() {
    Serial.println("entering setup1");
    TheEsp.setup();
    Serial.println("entering setup2");
    if(RELAY) pinMode(RELAY, OUTPUT);
    if(LED)   pinMode(LED, OUTPUT);
    if(BUTTON)pinMode(BUTTON, INPUT);
    digitalWrite(RELAY, __Ramm.relay_state);
    TheEsp._button = TheEsp.get_button_state()==LOW ? -1 : 0;
    Serial.println("exiting setup3");
}

void loop() {

    TheEsp.loop();
}
