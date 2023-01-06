#ifndef EEPROM_C_H
#define EEPROM_C_H
#include <EEPROM.h>
//#include <SPIFFS.h>
#include "_esp_mpus.h"
#include "_utils.h"

#define MAGIC 0xA5
#define          PACK_ALIGN_1   __attribute__((packed, aligned(1)))
struct config_t
{
    char            schedule[64];
    int8_t          autoenabled;
    bool            keepsta;
    uint8_t         temptrig;
    uint8_t         humtrig;
    char            trigger_rule;
    char            h_host[64];
    char            h_put[80];
    char            h_get[80];
    char            h_finger[62];
    short           h_port;
    char            mq_ttbroker[64];
    char            mq_topic[64];
    char            mq_user[16];
    char            mq_pass[9];
    short           mq_port;
    int             get_interval;
    long            ntp_offset=0;
    char            ntp_srv[24];
    char            ip[17];
    char            gw[17];
    char            subnet[17];
    char            dnsa[17];
    char            dnsb[17];
    char            ssid[17];
    char            passwd[17];
    char            minitek[17];
    uint8_t         magic;
}PACK_ALIGN_1;

/////////////////////////////////////////////////////////////////////////////////////////
class   eeprom_t
{
public:
    eeprom_t(int reread=0):_reread(reread)
    {
        if(reread)
        {
            LOG("EPROM BEGIN CFG=%d MAGIC=%d", sizeof(config_t), MAGIC);
            EEPROM.begin(1024);
            _get();
        }
    }
    ~eeprom_t()
    {
        if(_reread==1)
        {
            LOG("SAVING EPROM");
            _put();
        }
        if(_reread){
            EEPROM.end();
            LOG("EPROM END");
        }
    }
    void save(){
        _put();
        EEPROM.end();
        _reread=0;
    }
    void load(){
        EEPROM.begin(1024);
        _get();
        EEPROM.end();
        _reread=0;
    }
    config_t* operator->(){return &Conf;}

private:
    void _get()
    {
        size_t  magicoff = sizeof(config_t)-1;
        LOG("reading magic at address %d", magicoff);
        uint8_t magic = EEPROM.read(magicoff);
        LOG("magic is %X", magic);

        if( magic==MAGIC)
        {
            LOG("EPROM SIGNED, RESOTRING");
            uint8_t* p = (uint8_t*)&Conf;
            for (size_t i = 0; i < sizeof(Conf); i++)
                *(p+i) = EEPROM.read(i);
        }
        else
        {
            LOG("EPROM NOT SIGNED. %hhu",  magic);
            _put();
        }
    }

    void _put()
    {
        LOG("SAVING EPROM %d bytes", sizeof(Conf));
        Conf.magic = MAGIC;
        const uint8_t* p = (uint8_t*)&Conf;
        for (size_t i = 0; i <  sizeof(Conf); i++)
            EEPROM.write(i, *(p+i));

        size_t  magicoff = sizeof(config_t)-1;
        LOG("savinf/reading magic at %d", magicoff);
        uint8_t magic = EEPROM.read(magicoff);
        LOG("magic is %X", magic);

    }


    int             _reread = 0;
public:
    static config_t Conf;
};

#define CFG(x)    eeprom_t::Conf.x

#endif // EEPROM_C_H
