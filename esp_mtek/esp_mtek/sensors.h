#ifndef SENSORS_H
#define SENSORS_H

#include "_dev_config.h"
#include "wirewire.h"


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
};

/////////////////////////////////////////////////////////////////////////////////////////
class senssor_th
{
public:

    senssor_th(int se_id, uint8_t addr):_who(se_id),_addr(addr){
    }
    virtual ~senssor_th(){
    }
    virtual void begin(int sda, int scl, uint8_t addr)=0;
    virtual void end()=0;
    virtual void loop()=0;
    virtual const th_t* range()const=0;
    virtual const th_t& get_th()const=0;

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
protected:
    senssor_th* _sensors[MAX_SENS] = {0,0};
};

#endif // SENSORS_H
