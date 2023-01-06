#ifndef APPLICATION_C_H
#define APPLICATION_C_H

#include "_esp_mpus.h"
#include "eeprom.h"
#include "hot_ram.h"
#include "espxxsrv.h"
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
class sensor_th_t;
class app_t
{
public:

    app_t();
    virtual ~app_t();

    virtual void begin();
    virtual void loop();
    virtual void on_web_event(const sensdata_t* ev);
    virtual void on_relay_event(const sensdata_t* ev);
    virtual void on_button_event(const sensdata_t* ev);
    virtual void on_sensor_event(const sensor_th_t* ps);
    virtual void on_relay_control(float temp, float hum);
    virtual void http_get(ESP8266WebServer* ps, String& page)=0;
    virtual void on_init();
    bool get_relay()const{return _relay_state; }
    static void pet(){ESP.wdtFeed();}
#if I2C_SDA
    const sensors_t& sensors()const{return _sensors;}
#endif

public:
    static void tick_10();

public:
    static       app_t* TheApp;

protected:
    void _tick10();
    void _relay_ctrl(bool rm);
    void _led_ctrl(bool rm);
    void _draw_canvas(String& page);

protected:
    app_cfg_t       _app_cfg;
    espxxsrv_t*     _wifi = nullptr;

    Ticker          _timer;
    bool            _led_state=false;
    bool            _relay_state=0;
    bool            _inevent=false;
    bool            _init = false;
#if I2C_SDA
    sensors_t       _sensors;
#endif
    bool            _tenth = true;
    size_t          _decisecs;
    size_t          _lastsec;
    int             _temp = 0;
    size_t          _buttonstate[2]={0,0};
#if  WITH_GRAPH
    canvas          _canvas;
#endif
};

#endif // APPLICATION_C_H
