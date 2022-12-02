#include "esp_32_base.h"
#include "i2c_senz.h"
#include "html5_c.h"
#include <Wire.h>

#define USER_SIG    0x13


class my_esp_c : public esp_32_base_c
{
public:
    my_esp_c():esp_32_base_c(){};
    virtual ~my_esp_c(){};
    void  page_request(ESP8266WebServer* srv, String& page);
    void  user_loop(unsigned int loop);
    void  user_begin();

public:
    void relay_on()
    {
        digitalWrite(RELAY, _user.relayinverse ? HIGH :LOW);
        __Ramm.relay_state = 1;

    }

    void relay_off()
    {
        digitalWrite(RELAY, _user.relayinverse ? LOW : HIGH);
        __Ramm.relay_state = 0;
    }

private:
    String      _page;
    int         _lop = 0;
    bool        _out = false;
    i2c_senz_c  _senz;
    thp_str_t   _htp;
    html5_c     _graph;
    int         _fivemin = 0;
    bool        _firstloop = false;
    uint32_t    _last_read = 0;
    struct humi_str_t
    {
        int   tlimit    =  40;
        int   hlimit    =  35;
        int   presslimit = 1000;
        int   relayinverse = 0;
        bool  active = true;
        int   sig = USER_SIG;
    }           _user;
};


my_esp_c esp;

void  my_esp_c::user_begin()
{
    _senz.begin();
    _graph.begin(800,400);

    this->load_data(_user);
    if(_user.sig==USER_SIG)
    {
        _htp.temp_l = _user.tlimit;
        _htp.hum_l  =  _user.hlimit;
        _htp.pres_l = _user.presslimit;
    }
    else
    {
        _htp.temp_l  = _user.tlimit = 40;;
        _htp.hum_l   =  _user.hlimit = 38;
        _htp.pres_l  = _user.presslimit = 0;
        _user.active = 1;
        _user.relayinverse = 0;
    }
    if(__Ramm.relay_state){
        relay_on();
    }else{
        relay_off();
    }
}

void  my_esp_c::user_loop(unsigned int loop)
{
    static int test = 500;
    if(millis() - _last_read > 10000 || _firstloop==false)
    {
        _last_read = millis();
        _senz.loop(_htp);
        if(_htp.hum>0 && _htp.hum_l>0 && _user.active)
        {
            if(_htp.temp!=0 && _htp.hum_l>0)
            {
                if(_htp.temp > _htp.temp_l+1 && _htp.hum < _htp.hum_l-1 )
                {
                    Serial.print("RELAY ON");
                    relay_on();
                }
                else if(_htp.temp < _htp.temp_l-1 || _htp.hum > _htp.hum_l+1)
                {
                    Serial.print("RELAY OFF");
                    relay_off();
                }
            }
        }
    }
    if(_seconds % 60 == 0|| _firstloop==false)
    {
        pinMode(RELAY, OUTPUT);
        pinMode(LED, OUTPUT);
        digitalWrite(RELAY, __Ramm.relay_state);

        _senz.loop(_htp);
        _graph.loop(_htp,_seconds/60);
    }
    _firstloop=true;
}

void  my_esp_c::page_request(ESP8266WebServer* srv, String& page)
{
    if(srv->hasArg("fetch"))
    {
        page=_start_html(false);
        int has=0;
        for(int i=0;i<DAY_OF5-1;i++)
        {
            if(_graph._samples[i].temp!=0.0f)   has|=0x1;
            if(_graph._samples[i].hum>0.1f)     has|=0x2;
            if(_graph._samples[i].pres>0.1f)    has|=0x4;
        }
        if(has&0x1)
        {
            for(int i=0;i<DAY_OF5-1;i++)
            {
                page += String(_graph._samples[i].temp);
                page += ",";
            }
        }
        page += String(":");
        if(has&=0x2)
        {
            for(int i=0;i<DAY_OF5-1;i++)
            {
                page += String(_graph._samples[i].hum);
                page += ",";
            }
        }
        page += String(":");
        if(has&=0x4)
        {
            for(int i=0;i<DAY_OF5-1;i++)
            {
                page += String(_graph._samples[i].pres);
                page += ",";
            }
        }
        page += String(":");
        page += String(_seconds/(60*5)); // minute*5 in day
        return;
    }
    if(srv->hasArg("reboot"))
    {
        REBOOT();
    }
    if(srv->hasArg("usersave"))
    {
        char tt[16] = {0};
        char ht[16] = {0};
        char pt[16] = {0};
        char ri[16] = {0};
        char ac[16] = {0};

        _esp_srv->arg("tt").toCharArray(tt,14);
        _esp_srv->arg("th").toCharArray(ht,14);
        _esp_srv->arg("tp").toCharArray(pt,14);
        _esp_srv->arg("ri").toCharArray(ri,14);
        _esp_srv->arg("ac").toCharArray(ri,14);

        if(tt[0]) _user.tlimit =     _htp.temp_l  = atoi(tt);
        if(ht[0]) _user.hlimit =     _htp.hum_l   = atoi(ht);
        if(pt[0]) _user.presslimit = _htp.pres_l  = atoi(pt);
        if(ri[0]) _user.relayinverse              = atoi(ri);
        if(ac[0]) _user.active                    = atoi(ac);
        _user.sig = USER_SIG;
        this->save_data(_user);
        page+= _start_html();
        this->load_data(_user);
        page += "<a href='http://"+_toStringIp(WiFi.localIP())+"'>SAVED. GO HOME</a>";
        page += "<li>Temp Trigger   " + String(_user.tlimit);
        page += "<li>Hum Trigger    " + String(_user.hlimit);
        page += "<li>Press Trigger  " + String(_user.presslimit);

        page+= _end_html();
        return;
    }

    //////////////////////////////////////////
    page+= _start_html();
    if(srv->hasArg("relay"))
    {
        if(srv->arg("relay")=="1"){
            relay_on();
        }
        if(srv->arg("relay")=="0"){
           relay_off();
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
    page += F("<font size='3em'>");
    page +=  F("<table><tr><td>");
    page +=  F(__DATE__);
    page +=  F("/");
    page +=  F(__TIME__);
    page +=  F("<li>Temp: ");   page += String(_htp.temp); page += F(" / "); page += String(_htp.temp_l);
    page +=  F("<li>Hum: ");    page += String(_htp.hum);  page += F(" / "); page += String(_htp.hum_l);
    page +=  F("<li>Press: ");  page += String(_htp.pres); page += F(" / "); page += String(_htp.pres_l);
    page +=  F("<li>Secs: ");   page += String(_seconds);
    page += F("  [")+String(_seconds/3600);
    page += F(":")+String((_seconds/60)%60);
    page += F(":")+String(_seconds%60);
    page +=  F("]<li><a href='?relay=1'>TURN ON</a>");
    page +=  F("<li><a href='?relay=0'>TURN OFF</a>");
    page += "<li>gpio:" + String(LED);
    if(__Ramm.relay_state==0)
        page += "<li> RELAY  OFF";
    else
        page += "<li> RELAY  ON";

    page +=  F("</td><td>"
    "<form method='POST' action='?usersave'>"
    "<input type='text' placeholder='Trigger Temp (1-99)' name='tt'/>"
    "<br /><input type='text' placeholder='Trigger Hum (1-99)' name='th'/>"
    "<br /><input type='text' placeholder='Trigger Pressure (1-1200)' name='tp'/>"
    "<br /><input type='text' placeholder='Relay Inverse (0/1)' name='ri'/>"
    "<br /><input type='text' placeholder='Active On HT (0/1)' name='ac'/>"
    "<br /><input type='submit' value='Apply'/></form>");

    page +=  F("</tr></table>");
    page+="</font>";
    _graph.page(page);
    page +=  _end_html();
}

void setup() {
    esp.setup();
}
void loop() {
    esp.loop();
}
