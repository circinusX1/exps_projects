#include "sens_sht21.h"
#include "application.h"

sens_sht21::sens_sht21(int se_id, uint8_t addr):senssor_th(se_id, addr)
{
    _type=MTEMP|MHUM;
}

sens_sht21::~sens_sht21()
{
    end();
}

void sens_sht21::begin(int sda, int scl, uint8_t addr)
{
    _sht = new SHT2x();
    _sht->begin();
}

void sens_sht21::end()
{
    delete _sht;
}

void sens_sht21::loop()
{
    if(_sht && _enabled)
    {
        _sht->read();
        _data.t = _sht->getTemperature();
        _data.h = _sht->getHumidity();
        app_t::TheApp->on_sensor_event(this);
    }
}

