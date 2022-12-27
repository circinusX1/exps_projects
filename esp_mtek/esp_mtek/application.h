#ifndef APPLICATION_C_H
#define APPLICATION_C_H

#include "_dev_config.h"
#include "eeprom.h"
#include "hot_ram.h"
#include "wifisrv.h"
#include "sensors.h"
#include "clock_t.h"
#if  WITH_GRAPH
#include "canvas.h"
#endif


/////////////////////////////////////////////////////////////////////////////////////////
struct app_cfg_t
{
    int k;
};

/////////////////////////////////////////////////////////////////////////////////////////
class senssor_th;
class app_t
{
public:

    app_t();
    virtual ~app_t();

    virtual void begin();
    virtual void loop();
    virtual void on_web_event(const unidata_t* ev);
    virtual void on_relay_event(const unidata_t* ev);
    virtual void on_button_event(const unidata_t* ev);
    virtual void on_sensor_event(const senssor_th* ps);

    virtual void http_get(ESP8266WebServer* ps, String& page)=0;
    virtual void on_init();
    bool get_relay()const{return _relay_state; }
    static void pet(){ESP.wdtFeed();}

public:
    static void tick_10();

public:
    static       app_t* TheApp;

protected:
    void _tick10();
    void _relayOn(bool rm);
    void _ledOn(bool rm);
    void _draw_canvas(String& page);

protected:
    app_cfg_t       _app_cfg;
    wifisrv_t*      _wifi = nullptr;

    Ticker          _timer;
    bool            _led_state=false;
    bool            _relay_state=0;
    bool            _inevent=false;
    bool            _init = false;
    sensors_t       _sensors;
    bool            _tenth = true;
    size_t          _decisecs;
    size_t          _lastsec;
    size_t          _buttonstate[2]={0,0};
#if  WITH_GRAPH
    canvas          _canvas;
#endif
};

#endif // APPLICATION_C_H
