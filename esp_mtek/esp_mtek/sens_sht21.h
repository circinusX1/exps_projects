#ifndef SENS_SHT21_H
#define SENS_SHT21_H

#include "SHT2x.h"
#include "sensors.h"

class sens_sht21 :public senssor_th
{
public:
    sens_sht21(int se_id, uint8_t addr);
    virtual ~sens_sht21();

    void begin(int sda, int scl, uint8_t addr);
    void end();
    void loop();
    const th_t*  range()const{return _range;};
    const th_t&   get_th()const{return _data;}
protected:
    SHT2x*   _sht = nullptr; // = new AHT10(addr);
    th_t _range[2] = {{-40.0f,120.0f},{0.0f,100.0f}};
    th_t _data;
};

#endif // SENS_SHT21_H
