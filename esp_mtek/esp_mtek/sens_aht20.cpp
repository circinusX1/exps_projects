
#include "sens_aht20.h"
#include "application.h"
#if I2C_SDA

sens_aht20::sens_aht20(int se_id, uint8_t addr):sensor_th_t(se_id, addr)
{
    _type=MTEMP|MHUM;
}

sens_aht20::~sens_aht20()
{
    end();
}

void sens_aht20::begin(int sda, int scl, uint8_t addr)
{
    _aht = new AHT10(addr);
    int max = 30;
    while(max-- && _aht->begin(sda,scl)==false)
    {
        delay(256);
        app_t::pet();
    }
    if(max==0){
        _enabled=false;
    }else{
        _aht->softReset();
    }
}

void sens_aht20::end()
{
    delete _aht;
    _aht=nullptr;
}

void sens_aht20::loop()
{
    if(_aht && _enabled)
    {
        _data.t = _aht->readTemperature();
        _data.h = _aht->readHumidity();
        app_t::TheApp->on_sensor_event(this);
    }
}

#endif // #if I2C_SDA
