#ifndef I2C_BUZ_H
#define I2C_BUZ_H

#include "_config.h"
#include "Wire.h"
#if WITH_SHT21
#   include "SHT2x.h"        // comment out if you dont have it
#endif

#if WITH_BMP085
#   include "Adafruit_BMP085_U.h" // __BMP085_H__
#endif

#define TW24_HOURS  288
struct thp_str_t
{
    float temp;
    float hum;
    float pres;
    float temp_l = 40;
    float hum_l  = 38;
    float pres_l = 1000;
};

#if HAS_I2C
class i2c_senz_c
{
public:
    i2c_senz_c(){
    }

    ~i2c_senz_c(){
        Serial.println("\nI2C ended");
    }

    void begin(int sda=I2C_SDA, int scl=I2C_SCL)
    {
        Serial.println("i2c_senz_c BEGIN");
        if(I2C_SDA == 1)
        {
            Serial.flush();
            Serial.end();
        }
        Wire.begin(sda,scl);
        bool ret = scan();
        if(ret)
        {
            Serial.println("Starting SH2x");
#if WITH_SHT21
            Serial.println("Starting SH2x");
            Serial.flush();
            _sht = new SHT2x();
            _sht->begin(&Wire);
            _i2cs += ("SHT Sensor started\n");
#else

#endif

#if WITH_BMP085
            Serial.println("Starting WITH_BMP085");
            _bmp =  new Adafruit_BMP085_Unified();
            if (!_bmp->begin(I2C_SDA, I2C_SCL))
            {
                _i2cs += ("FAILED _bmp sensor\n");
                delete _bmp;  _bmp = nullptr;
            }
#endif
        }
        if(I2C_SDA == 1)
        {
            Serial.begin(SERIAL_BOS);
        }
        Serial.println("Scan Ended");
    }
    //////////////////////////////////////////////////////////////////////////////////
    bool scan()
    {
        bool ret = false;
        byte error, address;
        Serial.println("Scanning");
        for(address = 1; address < 127; address++ )
        {
            Wire.beginTransmission(address);
            delay(8);
            error = Wire.endTransmission();
            if (error == 0)
            {
                Serial.print("I2C device: 0X");
                Serial.println(address);
                ret = true;
            }
        }
        Serial.println("scaning Done");
        return ret;
    }


    void loop(thp_str_t& htp)
    {

        if(I2C_SDA == 1) // shared with UART on ESPS-01
        {
            Serial.flush();
            Serial.end();
        }

#if WITH_SHT21
        if(_sht)
        {

            _sht->read();
            htp.temp  = _sht->getTemperature();
            htp.hum   = _sht->getHumidity();
            htp.pres  = 0.0001f;
        }
        else
        {

            htp.temp = rand() % 50  + 20;
            htp.hum  = rand() % 50  + 20;
            htp.pres = rand() % 1000  + 120;

        }
#endif

#if WITH_BMP085
        if(_bmp)
        {
            sensors_event_t event;
            h = 0.0001f;
            _bmp->getEvent(&event);
            if(event.pressure){
                _bmp->getPressure(&_htp.pres);
                _bmp->getTemperature(&_htp.temp);
            }
        }
        else{
            htp.temp = rand()%20  + 20;
            htp.hum  = rand()%40  + 20;
            htp.pres  = rand()%1000  + 120;
        }

#endif

        if(I2C_SDA == 1)
        {

            Serial.begin(SERIAL_BOS);
        }

    }

private:

#if WITH_BMP085
    Adafruit_BMP085_Unified* _bmp = nullptr;
#endif
#if WITH_SHT21
    SHT2x*                  _sht = nullptr;
#endif
    String                 _i2cs;
    bool                   _hasdev=false;
};
#endif

#endif // I2C_BUZ_H
