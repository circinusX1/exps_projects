
#include "sensors.h"
#include "wirewire.h"
#include "sens_sht21.h"
#include "sens_aht20.h"
#if I2C_SDA

void sensors_t::begin(int a, int c)
{
    uint8_t   ads[MAX_SENS] = {0,0};
    LOG("SENSORS (%d %d)", a, c);
    ::i2c_begin(a, c);
    delay(100);
    ::i2c_scan(nullptr, ads);
    for(int i=0; ads[i];i++)
    {
        if(ads[i]==AHT10_ADDRESS_0X38)
            _sensors[i] = new sens_aht20(i, ads[i]);
        else if(ads[i]==0x40)
            _sensors[i] = new sens_sht21(i, ads[i]);
    }
    for(int i=0; _sensors[i];i++)
    {
        _sensors[i]->begin(_i2c_sda,_i2c_scl, ads[i]);
    }

}

void sensors_t::end()
{
    for(int i=0; _sensors[i];i++)
    {
        delete _sensors[i];
    }
}

void sensors_t::loop()
{
    for(int i=0; _sensors[i];i++)
    {
        _sensors[i]->loop();
    }
}

#endif //#if I2C_SDA
