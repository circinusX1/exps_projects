
#include "wirewire.h"
#include "application.h"

/////////////////////////////////////////////////////////////////////////////////////////
class MyApp : public app_t
{
public:
    MyApp(){}
    ~MyApp(){}

    void begin(){
        app_t::begin();
    }

    void loop(){
        app_t::loop();
    }

    void http_get(ESP8266WebServer* srv, String& page)
    {
        page +=  wifisrv_t::start_htm();

        const String& uri = srv->uri();
        LOG(uri.c_str());

        page += F("URI = ") + uri;


        if(uri=="/i2c")
        {
            LOG("I2C PAGE");
            page += F("<form method='GET' action='/i2c'>"
                      "<li><input type='text' hidden name='i2c'/>"
                      "<li><input type='text' placeholder='SDA' name='sda'/>"
                      "<li><input type='text' placeholder='SCL' name='scl'/>"
                      "<li><input type='submit' value='Apply'/></form>");
            page += F("<hr>");
            if(srv->hasArg("sda") && srv->hasArg("scl"))
            {
                char sda[8] = {0};
                char scl[8] = {0};
                page += F("<pre>");
                srv->arg("sda").toCharArray(sda,6);
                srv->arg("scl").toCharArray(scl,6);
                if(::atoi(sda) && atoi(scl))
                {
                    LOG("I2C %d %d", ::atoi(sda), ::atoi(scl));
                    ::i2c_begin(::atoi(sda), atoi(scl));
                    ::i2c_scan(&page,nullptr);
                    page += F("</pre>");

                    ::i2c_end();
                }
            }
        }
        if(srv->hasArg("reboot"))
        {
            REBOOT();
        }
        if(srv->hasArg("relay"))
        {
            if(srv->arg("relay")=="1"){
                _relayOn(true);
            }
            if(srv->arg("relay")=="0"){
                _relayOn(false);
            }
        }
        if(srv->hasArg("gpio"))
        {
            int nLED = srv->arg("gpio").toInt();
            pinMode(nLED, OUTPUT);
            digitalWrite(nLED, LOW);
            delay (1000);
            digitalWrite(nLED, HIGH);
            delay (1000);
            digitalWrite(nLED, LOW);
        }
        page +=  F("<li><a href='?relay=1'>TURN ON</a>");
        page +=  F("<li><a href='?relay=0'>TURN OFF</a>");
        uint32_t free = system_get_free_heap_size();
        page +=  F("<li>RAM:")+String(free);
DONE:
        _draw_canvas(page);

        page +=  wifisrv_t::end_htm();

    }

    void on_init()
    {
        app_t::on_init();
    }

private:


};

/////////////////////////////////////////////////////////////////////////////////////////
MyApp* pApp;


/////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
    pApp = new MyApp();
    pApp->begin();
}

/////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
    pApp->loop();
}

