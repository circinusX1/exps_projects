#ifndef WIREWIRE_H
#define WIREWIRE_H

#include "_dev_config.h"
#include <twi.h>
#include <Wire.h>

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

extern bool _taken;
extern int  _i2c_sda,_i2c_scl;

inline void i2c_begin(int sda, int scl)
{
    LOG("i2cbegin(%d %d)", sda, scl);
    _i2c_sda = sda; _i2c_scl = scl;
    if(!_taken)
    {
        LOG(__FUNCTION__);
        Wire.begin(sda,scl);
        _taken = true;
    }
    else
    {
        Wire.flush();
        LOG("I2C taken ");
    }
}

inline void i2c_begin(int sda, int scl, int address)
{
    _i2c_sda = sda; _i2c_scl = scl;
    if(!_taken)
    {
        LOG(__FUNCTION__);
        Wire.begin(sda,scl,address);
        _taken = true;
    }
    else
    {
        Wire.flush();
        LOG("I2C taken ");
    }
}

inline void i2c_end()
{
    if(_taken)
    {
        LOG(__FUNCTION__);
        Wire.flush();
        pinMode(_i2c_sda, INPUT);
        pinMode(_i2c_scl, INPUT);
        _taken=false;
    }
    else
    {
        LOG("I2C not taken");
    }
}

bool i2c_scan(String* page, uint8_t* paddrs);



#endif // WIREWIRE_H
