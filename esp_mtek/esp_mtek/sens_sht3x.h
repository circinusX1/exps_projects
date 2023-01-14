#ifndef SENS_SHT31_H
#define SENS_SHT31_H

#include "SHT3x.h"
#include "sensors.h"
#if I2C_SDA

class sens_sht3x :public sensor_th_t
{
public:
    sens_sht3x(int se_id, uint8_t addr);
    virtual ~sens_sht3x();

    void begin(int sda, int scl, uint8_t addr);
    void end();
    void loop();
    const th_t*  range()const{return _range;};
    const th_t&   get_th()const{return _data;}
    const char* name()const{return "SHT21";}

protected:
    SHT3x*   _sht = nullptr; // = new AHT10(addr);
    th_t     _range[2] = {{-40.0f,120.0f},{0.0f,100.0f}};
    th_t     _data;
};
#endif //#if I2C_SDA
#endif // SENS_SHT21_H
