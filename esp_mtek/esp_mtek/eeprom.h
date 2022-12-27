#ifndef EEPROM_C_H
#define EEPROM_C_H
#include <EEPROM.h>
#include "_dev_config.h"

#define MAGIC 0x14

struct config_t
{
    uint8_t         magic;
    int8_t          autoenabled;
    bool            client;
    bool            keepsta;
    bool            relayinverse;
    char            timeoff[4];
    char            h_host[64];
    char            h_put[80];
    char            h_get[80];
    char            h_finger[62];
    short           h_port;
    char            mq_ttbroker[64];
    char            mq_topic[64];
    char            mq_user[20];
    char            mq_pass[20];
    short           mq_port;
    int             get_interval;
    int             ntp_offset=0;

    char            ip[17];
    char            gw[17];
    char            subnet[17];
    char            dns[2][17];
    char            ssid[17];
    char            passwd[17];
};

/////////////////////////////////////////////////////////////////////////////////////////
class   eeprom_t
{
public:
    eeprom_t(int reread=0):_reread(reread)
    {
        if(reread)
        {
            LOG("EPROM BEGIN CFG=%d", sizeof(config_t));
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

    config_t* operator->(){return &Conf;}

private:
    void _get()
    {
        if(EEPROM.read(0)==MAGIC)
        {
            LOG("EPROM SIGNED, RESOTRING");
            uint8_t* p = (uint8_t*)&Conf;
            for (size_t i = 0; i < sizeof(Conf); i++)
                *(p+i) = EEPROM.read(i);
        }
        else
        {
            LOG("EPROM NOT SIGNED.");
            _put();
        }
    }

    void _put()
    {
        LOG("SAVING EPROM");
        Conf.magic = MAGIC;
        const uint8_t* p = (uint8_t*)&Conf;
        for (size_t i = 0; i <  sizeof(Conf); i++)
            EEPROM.write(i, *(p+i));
    }
    int             _reread = 0;
public:
    static config_t Conf;
};

#define CFG(x)    eeprom_t::Conf.x

#endif // EEPROM_C_H
