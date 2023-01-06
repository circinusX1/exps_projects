
#include "application.h"
#include "hot_ram.h"
#include "sens_aht20.h"  //debug granph only

app_t* app_t::TheApp;
#define CHECK_SENSORS   10
#define CHECK_NTP       7000

app_t::app_t()
{
    Serial.begin(SERIAL_BPS);
    LOG(__FUNCTION__);
    eeprom_t eprom(-1);
    hot_restore();
    LOG("STARTING WITH SSID IP = %s %s", CFG(ip), CFG(ssid));
}

app_t::~app_t(){
    _wifi->end();
    delete _wifi;
}

//////////////////////////////////////////////////////////////////////////////////////////
void app_t::begin()
{
    LOG("APP T BEGIN");
    app_t::TheApp = this;
#if I2C_SDA
    _sensors.begin(I2C_SDA, I2C_SCL);
#endif

    _wifi = new espxxsrv_t();
    _wifi->begin();

    _timer.attach_ms(100, app_t::tick_10);
    if(CFG(ntp_offset) != 0xFF)
    {
        LOG("===========> NTP offset = %d", CFG(ntp_offset));
        Sclk.init_ntp();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
void app_t::loop()
{
    _wifi->loop();

    if(!_init)
    {
        _init=true;
        on_init();
    }
    if(_wifi && _wifi->is_otaing())
    {
        return;
    }
    // every second
    size_t now = Sclk.seconds();
    if(now != _lastsec)
    {
        _lastsec = now;
        if(now % CHECK_SENSORS==0)
        {
            if(_wifi)
            {
                if(_wifi->mode()==1){
                    LOG("WIFI: minitek 10.5.5.1");
                }
            }
#if I2C_SDA
            _sensors.loop();
#endif
        }
        if(now % CHECK_NTP==0 && _wifi->sta_connected())
        {
            LOG("CHECKING NTP");
            Sclk.update_ntp();
        }
    }
    {
#if WITH_GRAPH
#if 0
        sens_aht20 tt(0,0);
        tt._data.h = 30+rand()%10;
        tt._data.t = 60+rand()%10;
        _canvas.add_th(_temp, &tt, _relay_state);
        _temp++;
        if(_temp==MINS_PER_DAY){
            _temp = 0;
        }
#endif // test graph
#endif
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
void app_t::on_init()
{
    LED ? pinMode(LED, OUTPUT) : __nill();
    RELAY ? pinMode(RELAY, OUTPUT) : __nill();
    _relay_ctrl(Ramm.relay);
    Sclk.update_ntp();
#if I2C_SDA
    _sensors.loop();
#endif
#if  WITH_GRAPH
    th_t triger = {CFG(temptrig),CFG(humtrig)};
    _canvas.set_trigger(triger);
#endif

    //xTaskCreate(Demo_Task, "DT", 4096, NULL,10, &myTaskHandle);
}

/////////////////////////////////////////////////////////////////////////////////////////
void app_t::on_web_event(const sensdata_t* val)
{
    LOG(__FUNCTION__);
    _inevent = true;

    const sensdata_t* p = val;
    while(p)
    {
        switch(p->type)
        {
        case eTIME:   Sclk.set_seconds(p->u.z); break;
        case eRELAY:  _relay_ctrl(p->u.uc); break;
        }
        p=p->next;
    }
    sensdata_t::release(val);
    _inevent = false;
}

//////////////////////////////////////////////////////////////////////////////////////////
void app_t::on_relay_event(const sensdata_t* val)
{
    LOG(__FUNCTION__);
    if(!_inevent)
    {
        _wifi->on_relay_event(val);
    }
    sensdata_t::release(val);
}

//////////////////////////////////////////////////////////////////////////////////////////
void app_t::on_button_event(const sensdata_t* val)
{
    LOG(__FUNCTION__);
    if(BUTTON)
    {
        if(val->u.uc==BUTTON_DOWN){
            _buttonstate[0]=BUTTON_DOWN;
            _buttonstate[1]=millis();
        }
        if(_buttonstate[0]=BUTTON_DOWN && val->u.uc==BUTTON_UP)
        {
            _buttonstate[0]=BUTTON_UP;
            if(millis() - _buttonstate[1]>64)
            {
                _led_state=~_led_state;
                _relay_ctrl(_led_state);
            }
        }
        sensdata_t::release(val);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void app_t::_relay_ctrl(bool on)
{
    //LOG("RELAY:%d = %d",RELAY,on);
    eeprom_t e;
    if(on)
        digitalWrite(RELAY, RELAY_ON);
    else
        digitalWrite(RELAY, RELAY_OFF);

    sensdata_t* p = sensdata_t::get(eRELAY, on);
    if(_relay_state != on)
    {
        on_relay_event(p);
        _relay_state=on;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void app_t::tick_10()
{
    Sclk.tick();
    TheApp->_tick10();
}

//////////////////////////////////////////////////////////////////////////////////////////
void app_t::_tick10()
{
    _tenth = true;
    if(Sclk.decisec() % _wifi->mode() == 0)
    {
        _led_ctrl(_led_state=!_led_state);
    }
    if(_led_state == LED_ON){
        _led_ctrl(LED_ON);
        _led_state=LED_ON;
    }
}

////////////////////////////////////////////////////////////////////////////////////////
void app_t::_led_ctrl(bool rm)
{
    digitalWrite(LED, rm ? LED_ON : LED_OFF);
}

////////////////////////////////////////////////////////////////////////////////////////
void app_t::_draw_canvas(String& page)
{
#if WITH_GRAPH
    _canvas.draw(page);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////
void app_t::on_sensor_event(const sensor_th_t* ps)
{
#if I2C_SDA
    if(CFG(autoenabled))
    {
        th_t th = ps->get_th();
        on_relay_control(th.t, th.h);
    }

    if(_wifi){
        _wifi->on_sensor_event(ps);
    }

#if WITH_GRAPH
    _canvas.add_th(Sclk.minutes(), ps, _relay_state);
#endif
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
void app_t::on_relay_control(float temp, float hum)
{
    if(CFG(temptrig) && CFG(humtrig))
    {
        float   tt = CFG(temptrig);
        float   ht = CFG(humtrig);
        bool    relay = _relay_state;


    }
}

