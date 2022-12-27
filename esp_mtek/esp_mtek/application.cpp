
#include "application.h"
#include "hot_ram.h"

app_t* app_t::TheApp;
#define CHECK_SENSORS   10
#define CHECK_NTP       7000

app_t::app_t()
{
    Serial.begin(SERIAL_BPS);
    LOG(__FUNCTION__);
    eeprom_t eprom(-1);
    hot_restore();
}

app_t::~app_t(){
    _wifi->end();
    delete _wifi;
}

void app_t::begin()
{
    LOG(__FUNCTION__);
    app_t::TheApp = this;
    _timer.attach_ms(100, app_t::tick_10);
    _wifi = new wifisrv_t();
    _wifi->begin();
    _sensors.begin(I2C_SDA, I2C_SCL);
    if(CFG(ntp_offset)!=0xFF)
    {
        Sclk.init_ntp();
    }
}

void app_t::loop()
{
    _wifi->loop();
    if(!_init)
    {
        _init=true;
        on_init();
    }
    // led
    if(Sclk.decisec() % _wifi->mode() == 0)
    {
        _ledOn(_led_state=!_led_state);
    }
    delay(100);
    // every second
    size_t now = Sclk.seconds();
    if(now != _lastsec)
    {
        _lastsec = now;
        if(now % CHECK_SENSORS==0)
        {
            LOG("CHECKING SENSORS");
            _sensors.loop();
        }
        if(now % CHECK_NTP==0)
        {
            LOG("CHECKING NTP");
            Sclk.update_ntp();
        }
    }
}

void app_t::on_init()
{
    LED ? pinMode(LED, OUTPUT) : __nill();
    RELAY ? pinMode(RELAY, OUTPUT) : __nill();
    _relayOn(Ramm.relay);
    Sclk.update_ntp();
    _sensors.loop();
}

/////////////////////////////////////////////////////////////////////////////////////////
void app_t::on_web_event(const unidata_t* val)
{
    LOG(__FUNCTION__);
    _inevent = true;

    const unidata_t* p = val;
    while(p)
    {
        switch(p->type)
        {
        case eTIME:   Sclk.set_seconds(p->data); break;
        case eRELAY:  _relayOn(p->data); break;
        }
        p=p->next;
    }
    unidata_t::release(val);
    _inevent = false;
}

//////////////////////////////////////////////////////////////////////////////////////////
void app_t::on_relay_event(const unidata_t* val)
{
    LOG(__FUNCTION__);
    if(!_inevent)
    {
       _wifi->on_relay_event(val);
    }
    unidata_t::release(val);
}

//////////////////////////////////////////////////////////////////////////////////////////
void app_t::on_button_event(const unidata_t* val)
{
    if(val->data==BUTTON_DOWN){
        _buttonstate[0]=BUTTON_DOWN;
        _buttonstate[1]=millis();
    }
    if(_buttonstate[0]=BUTTON_DOWN && val->data==BUTTON_UP)
    {
        _buttonstate[0]=BUTTON_UP;
        if(millis() - _buttonstate[1]>64)
        {
            _led_state=~_led_state;
            _relayOn(_led_state);
        }
    }
    unidata_t::release(val);
}

//////////////////////////////////////////////////////////////////////////////////////////
void app_t::_relayOn(bool on)
{
    LOG("RELAY:%d = %d",RELAY,on);
    eeprom_t e;
    if(on)
        digitalWrite(RELAY, e->relayinverse ? LOW : HIGH);
    else
        digitalWrite(RELAY, e->relayinverse ? HIGH : LOW);
    unidata_t* p = unidata_t::get(eRELAY, (float)on);
    on_relay_event(p);
}

void app_t::tick_10()
{
    Sclk.tick();
    TheApp->_tick10();
}

void app_t::_tick10()
{
    _tenth = true;
    /*
    if(Sclk.get_decisec() % _wifi->mode() == 0)
    {
        _ledOn(_led_state=!_led_state);
    }
    */
    /*
    if(_led_state == HIGH){
        _ledOn(HIGH);
        _led_state=HIGH;
    }
    */
}

void app_t::_ledOn(bool rm)
{
    digitalWrite(LED, rm ? HIGH : LOW);
}


void app_t::_draw_canvas(String& page)
{
#if WITH_GRAPH
    _canvas.draw(page);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////
void app_t::on_sensor_event(const senssor_th* ps)
{
    LOG("sensor event");
    if(CFG(autoenabled))
    {

    }

#if WITH_GRAPH
    _canvas.add_th(Sclk.minutes(), ps, _relay_state);
#endif
}




