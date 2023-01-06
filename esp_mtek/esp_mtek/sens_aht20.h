#ifndef SENS_AHT20_H
#define SENS_AHT20_H

#include "AHT10.h"
#include "sensors.h"
#if I2C_SDA

class sens_aht20 : public sensor_th_t
{
public:
    sens_aht20(int se_id, uint8_t);
    virtual ~sens_aht20();

    void begin(int sda, int scl, uint8_t addr);
    void end();
    void loop();
    const th_t*  range()const{return _range;}
    const th_t&   get_th()const{return _data;}
    const char* name()const{return "AHT10";}
public:
    AHT10*  _aht = nullptr;
    th_t _range[2] = {{-40.0f,100.0f},{0.0f,100.0f}};
    th_t _data;
};
#endif // #if I2C_SDA
#endif // SENS_AHT20_H
