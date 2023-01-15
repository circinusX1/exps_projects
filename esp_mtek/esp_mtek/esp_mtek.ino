
#include "wirewire.h"
#include "application.h"
#include "systeminfo.h"

/////////////////////////////////////////////////////////////////////////////////////////
class MyApp : public app_t
{
public:
    MyApp(){}
    ~MyApp(){}

    void begin(){
        app_t::begin();
        ::print_system_info();
    }

    void loop(){
        app_t::loop();
    }

    void http_get(ESP8266WebServer* srv, String& page)
    {
        page = espxxsrv_t::start_htm();

        const String& uri = srv->uri();
        LOG(uri.c_str());



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
#if I2C_SDA
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
#endif // #if I2C_SDA
            }
        }
        if(srv->hasArg("reboot"))
        {
            ESP.restart();
        }
        if(srv->hasArg("relay"))
        {
            if(srv->arg("relay")=="1"){
                _relay_ctrl(true);
                Sclk.flash(1);
            }
            if(srv->arg("relay")=="0"){
                _relay_ctrl(false);
                Sclk.flash(0);
            }

        }
        if(srv->hasArg("gpio"))
        {
            int nLED = srv->arg("gpio").toInt();
            pinMode(nLED, OUTPUT);
            digitalWrite(nLED, LED_OFF);
            delay (1000);
            digitalWrite(nLED, LED_ON);
            delay (1000);
            digitalWrite(nLED, LED_OFF);
        }
        page += F("NTP:") + _S(CFG(ntp_srv));
        page += F("NTP Time:") + _S(CFG(ntp_offset));
        page +=  F("<table><tr><td width='50%'>");
        page +=  F("URI = ") + uri;
        page +=  F("<br><a href='?relay=1'><button>TURN ON</button></a>");
        page +=  F("<br><a href='?relay=0'><button>TURN OFF</button></a>");
#if I2C_SDA
        const sensor_th_t* pt = _sensors.sensor(0);
        if(pt)
        {
            const th_t& th = pt->get_th();
            page +=  F("<br>Temp: ")+String(th.t)+F(" / ") +CFG(temptrig);
            page +=  F("<br>Hum: ")+String(th.h)+F(" / ") +CFG(humtrig);
        }
#endif // #if I2C_SDA
        page +=  CFG(autoenabled) ? F("Relay TH enabled") : F("Relay TH off");
        page +=  "<br>Schedule: " + _S(CFG(schedule));
        page +=  F("<br>Day Min: ")+String(Sclk.minutes());
        page +=  F("<br>Time: ")+String(Sclk.hours())+F(":")+
                                 String(Sclk.minutes())+F(":")+
                                 String(Sclk.seconds());

        page += F("<br>Sensors: ");
        for(int i=0;i<MAX_SENS;i++)
        {
            const sensor_th_t* ps = _sensors.sensor(i);
            if(ps){
                page += ps->name();
                page += F(" ");
            }
        }
        uint32_t free = system_get_free_heap_size();
        page +=  F("<br>Free RAM:")+String(free);

        page += F("</td><td width='50%'>");
        if(_wifi->sta_connected()){
            page += F("<br>Connected to:") + String(CFG(ssid));
            page += F("<br>Device IP:") + String(CFG(ip));
        }else{
            page += F("<br>SSID:") + String(CFG(ssid));
            page += F("<br>IP:") + String(CFG(ip));
        }
        if(_wifi->ap_active()){
            page+=F("<font color='green'>");
        }else{
            page+=F("<font color='grey'>");
        }
        page += F("<br>Access point:") + String(CFG(minitek));
        page += F("<br>Device IP: 10.5.5.1");

        page += F("</td></tr></table>");

DONE:
        _draw_canvas(page);
        espxxsrv_t::end_htm(page);

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
    LOG("MAIN SETUP");
}

/////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
    pApp->loop();
}


//////////////////////////////////////////////////////////////////////////////////////////
