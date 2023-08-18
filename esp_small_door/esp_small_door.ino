

#include "esp32_full.h"
#include <Wire.h>
#include <DateTimeTZ.h>
#include <TimeElapsed.h>
#include <ESPDateTime.h>
#include <DateTime.h>

#define VERSION "1.0.1"

/////////////////////////////////////////////////////
/////////////////   ESP8266 relay ON OFF ////////////

// ca.pool.ntp.org / pool.ntp.org
// board generic ESP8266 module
static uint32_t CurMs = 0;

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
                if(LED)digitalWrite(LED, LOW);
                delay (100);
                if(LED)digitalWrite(LED, HIGH);
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
            _btgpio = srv->arg("button").toInt();
            pinMode(_btgpio, INPUT);
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
        page += VERSION;
        page += "<li><a href='?relay=1'>TURN ON</a>";
        page += "<li><a href='?relay=0'>TURN OFF</a>";
        page += "<li>LED:" + String(LED);
        page += "<li>RELAY:" + String(RELAY);
        page += "<li>BUTTON:" + String(_btgpio) + " = " + (get_button_state());
        
        if(__Ramm.relay_state==LOW)
            page += "<li> LED OFF";
        else
            page += "<li> LED ON";
        if(DateTime.isTimeValid())
        {
           page += "<li>"; 
           page += DateTime.toString();
        }
        else
        {
           page += "<li>Cannot get NTP"; 
           DateTime.begin();
        }
        if(getOnTime())
        {
            page += "<li>Time zone:";
            page += DateTime.getTimeZone();
            page += "<li> On time ";
            page += getOnTime()/60;
            page += ":";
            page += getOnTime()%60;
            
            page += "<li> Off time ";
            page += getOffTime()/60;
            page += ":";
            page += getOffTime()%60;
            
            DateTimeParts p = DateTime.getParts();
            page += "<li> H:M ";
            page += p.getHours();
            page += ":";
            page += p.getMinutes();
        }
        
        page+="</font>";
        page +=  _end_html();
    }

    void set_relay(bool on) // globe plug
    {
        if(on)
        {
          digitalWrite(RELAY, HIGH);
          __Ramm.relay_state = 1;
        }
        else
        {
          digitalWrite(RELAY, LOW);
          __Ramm.relay_state = 0;
        }
    }

    int get_button_state() // globe plug
    {
        return _btgpio  ? digitalRead(_btgpio) : HIGH;
    }
    
    String _page;
    int    _lop = 0;
    bool   _out = false;
    int    _button = 0;
    int    _down = 0;
    int    _btgpio = BUTTON;
    uint32_t _regain = millis();
} TheEsp(80);



void setup() {
    Serial.println("entering setup1");
    TheEsp.setup();
    Serial.println("entering setup2");
    if(RELAY) pinMode(RELAY, OUTPUT);
    if(LED)   pinMode(LED, OUTPUT);
    if(BUTTON)pinMode(BUTTON, INPUT);
    if(RELAY)digitalWrite(RELAY, __Ramm.relay_state);
    TheEsp._button = TheEsp.get_button_state()==LOW ? -1 : 0;
    
    DateTime.setServer(TheEsp.getNTPSrv());
    char cst[16];
    if(TheEsp.getTz()<0)
      ::sprintf(cst,"CST%d",TheEsp.getTz());
    else  
      ::sprintf(cst,"CST+%d",TheEsp.getTz());
    DateTime.setTimeZone(cst);
    DateTime.begin();
    
    if (!DateTime.isTimeValid()) {
      Serial.println("Failed to get time from server.");
      delay(100);
      DateTime.begin();
    } else {
      Serial.printf("Date Now is %s\n", DateTime.toISOString().c_str());
      Serial.printf("Timestamp is %ld\n", DateTime.now());
      
    }
    Serial.println("exiting setup3");
    CurMs = millis();
}


void loop() {

    if(TheEsp.getOnTime())
    {
        uint32_t diff = 0;
        uint32_t curt = millis();
        if(curt > CurMs )
        {
            diff = curt-CurMs;
        }
        else
        {
            diff - 0xFFFFFFFF-CurMs + curt;
        }
        if(diff > (5 * 60 * 1000))           // every 5 min 
        {
            CurMs = curt;
            if(DateTime.isTimeValid())
            {
                 DateTimeParts p = DateTime.getParts();
                 int curminday = p.getHours()*60 + p.getMinutes();
                 
                 if(curminday > TheEsp.getOnTime() && 
                    curminday < TheEsp.getOffTime())
                    {
                        TheEsp.set_relay(true);
                    }
                    else
                    {
                        TheEsp.set_relay(false);
                    }
            }
            else
            {
                DateTime.begin();
                delay(1000);
                if(DateTime.isTimeValid())
                {
                     DateTimeParts p = DateTime.getParts();
                     int curminday = p.getHours()*60 + p.getMinutes();
                     if(curminday > TheEsp.getOnTime() && 
                        curminday < TheEsp.getOffTime())
                        {
                            TheEsp.set_relay(true);
                        }
                        else
                        {
                            TheEsp.set_relay(false);
                        }
                }                
                
            }
        }
    }
    TheEsp.loop();
}
