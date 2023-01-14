
#include "sens_sht3x.h"
#include "application.h"
#if I2C_SDA
sens_sht3x::sens_sht3x(int se_id, uint8_t addr):sensor_th_t(se_id, addr)
{
    _type=MTEMP|MHUM;
}

sens_sht3x::~sens_sht3x()
{
    end();
}

void sens_sht3x::begin(int sda, int scl, uint8_t addr)
{
    Wire.begin(sda,scl);
    Wire.setClock(100000);          //experimental! AVR I2C bus speed: 31kHz..400kHz/31000..400000, default 100000

    _sht = new DFRobot_SHT3x(&Wire, 0x44);

    _sht->begin();
    if(!_sht->softReset()){
        Serial.println("Failed to reset the chip");
    }
    if(!_sht->startPeriodicMode(_sht->eMeasureFreq_Hz5)){
        Serial.println("Failed to enter the periodic mode");
    }
}

void sens_sht3x::end()
{
    delete _sht;
    _sht=nullptr;
}

void sens_sht3x::loop()
{
    if(_sht && _enabled)
    {
        _data.t = _sht->getTemperatureC();
        _data.h = _sht->getHumidityRH();
        app_t::TheApp->on_sensor_event(this);
    }
}

#endif //#if I2C_SDA
