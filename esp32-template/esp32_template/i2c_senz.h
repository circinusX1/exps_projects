#ifndef I2C_BUZ_H
#define I2C_BUZ_H

#include "_config.h"
#include "Wire.h"
#if WITH_SHT21
#   include "SHT2x.h"        // comment out if you dont have it
#endif
#if WITH_ATH10
#include "AHT10.h"
#endif

#include "esp_32_base.h"

extern RAMM __Ramm;

#if WITH_BMP085
#   include "Adafruit_BMP085_U.h" // __BMP085_H__
#endif


#define TW24_HOURS  288
struct thp_str_t
{
    float temp;
    float hum;
    float pres;
    uint8_t relay;
    float temp_l = 40;
    float hum_l  = 38;
    float pres_l = 1000;
};


class i2c_senz_c
{
public:
    i2c_senz_c(){
    }

    ~i2c_senz_c(){
        Serial.println("\nI2C ended");
        end();
    }

    void end(){
    }

    void begin(int sda=I2C_SDA, int scl=I2C_SCL)
    {
        Serial.println("Wire BEGIN");

        Wire.begin(sda,scl);

        Serial.println("Scanning Wire began");
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
#if WITH_ATH10
            Serial.println("Starting WITH_ATH");
            _ath =  new AHT10(AHT10_ADDRESS_0X38);
            int max = 30;

            while (max-- && _ath->begin(sda,scl) != true)
            {
                delay(1000);
                Serial.print(".");
                Serial.flush();
                ESP.wdtFeed();
            }
            if(max<=0){
                __Ramm.fail=1;
                Serial.print("ATH failed ");
                REBOOT();
            }
            __Ramm.fail=1;
            _ath->softReset();

            Serial.print("WITH_ATH Okay: TH=");
            Serial.println(max);
            Serial.print("T:"); Serial.println(_ath->readTemperature());
            delay(100);
            Serial.print("H:"); Serial.println(_ath->readHumidity());
            delay(100);
#endif
        }
        Serial.println("Scan Ended");
        __Ramm.fail=0;
    }
    //////////////////////////////////////////////////////////////////////////////////
    static bool scan(String* page=nullptr)
    {
        bool ret = false;
        byte error, address;
        Serial.println("Scanning");
        if(page) page->concat("Scanning<br>");
        for(address = 1; address < 127; address++ )
        {
            Serial.print(int(address));
            Serial.print(" ");
            Serial.flush();
            if(address%32==0)
                Serial.println("");

            Wire.beginTransmission(address);
            delay(8);
            error = Wire.endTransmission();
            if (error == 0)
            {
                Serial.print("\r\nI2C device: 0x");
                Serial.print(address, HEX);
                Serial.println("");
                if(page) page->concat("Found : 0x");
                if(page) page->concat(String(address,HEX));
                if(page) page->concat("<br />");
                ret = true;
            }
        }
        Serial.println("Scaning Done");
        if(page) page->concat("Scanning Done<br>");
        return ret;
    }


    void loop(thp_str_t& htp)
    {


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

#if WITH_ATH10
        htp.temp = _ath->readTemperature();
        htp.hum = _ath->readHumidity();
#endif

    }

private:

#if WITH_BMP085
    Adafruit_BMP085_Unified* _bmp = nullptr;
#endif
#if WITH_SHT21
    SHT2x*                  _sht = nullptr;
#endif

#if WITH_ATH10
    AHT10*                  _ath = nullptr;
#endif
    String                 _i2cs;
    bool                   _hasdev=false;
};


#endif // I2C_BUZ_H
