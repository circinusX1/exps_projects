#ifndef SENSORS_H
#define SENSORS_H


#include "_my_config.h"
#include "wirewire.h"
#if I2C_SDA

/////////////////////////////////////////////////////////////////////////////////////////
#define MTEMP      0x1
#define MHUM       0x2
#define MPRES      0x4
#define MLUM       0x8
#define MIR        0x10

/////////////////////////////////////////////////////////////////////////////////////////
struct th_t{
    float t;
    float h;
    bool operator!=(const th_t& r){
        return (::fabs(t-r.t)>0.2 && ::fabs(h-r.h)>0.2);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////
class sensor_th_t
{
public:

    sensor_th_t(int se_id, uint8_t addr):_who(se_id),_addr(addr){
    }
    virtual ~sensor_th_t(){
    }
    virtual void begin(int sda, int scl, uint8_t addr)=0;
    virtual void end()=0;
    virtual void loop()=0;
    virtual const th_t* range()const=0;
    virtual const th_t& get_th()const=0;
    virtual const char* name()const=0;
public:
    uint32_t type()const {return _type;}

protected:
    uint8_t   _addr    = 0;
    uint8_t   _who     = 0;
    bool      _enabled = true;
    uint32_t  _type;
};

//////////////////////////////////////////////////////////////////////////////////////////
class sensors_t
{
public:
    sensors_t(){};
    ~sensors_t(){};
    void begin(int a, int c);
    void end();
    void loop();
    const sensor_th_t* sensor(int i)const{ return i < MAX_SENS ? _sensors[i] : nullptr;}

protected:
    sensor_th_t* _sensors[MAX_SENS] = {0,0};
};

#endif // #if I2C_SDA
#endif // SENSORS_H
