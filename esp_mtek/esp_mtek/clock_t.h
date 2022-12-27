#ifndef CLOCK_T_H
#define CLOCK_T_H

#include "_utils.h"
#include "eeprom.h"
#include "_dev_config.h"
#include "hot_ram.h"

class simple_clock_t
{
public:
    simple_clock_t(){};
    ~simple_clock_t(){delete _ntp;};

    void tick(){
        ++_oftensec;
        ++_decisec;
        if(_oftensec==10){
            _oftensec = 0;
            ++_second;
            ++_daysec;
            if(_second==60){
                _second=0;
                ++_minute;
                if(_minute==60){
                    ++_hour;
                    if(_hour==24){
                        _hour=0;
                        _daysec=0;
                        _decisec=0;
                    }
                }
            }
        }
    }

    size_t seconds()const{return _daysec;}
    size_t decisec()const{return _decisec;}
    size_t minutes()const{return _daysec/60;}


    FORCE_INLINE unsigned long diff_time(size_t now, size_t start) {
        if (now >= start) return now - start;
        else return 0xFFFFFFFF - start + now + 1;
    }
    FORCE_INLINE unsigned long msElapsed(size_t start){
        unsigned long now = ::millis();
        return diff_time(now, start);
    }


    void set_seconds(size_t seconds){
        _daysec = seconds;
        _second = seconds % 60;
        _hour     = seconds/3600;
        _minute   = (seconds%3600)/60;
        LOG("H %d  M %d  S %d", _hour, _minute, _second);
    }

    void init_ntp()
    {
        LOG("CREATE NTP");
        Ramm.ntpfail = true;
        _ntp = new NTPClient(_ntpUDP,CFG(ntp_offset)*3600);
        _ntp->begin();
        Ramm.ntpfail = false;
    }

    void update_ntp()
    {
        if(_ntp)
        {
            Ramm.ntpfail = true;
            _ntp->update();
            LOG("FETCH NTP");
            size_t s = _ntp->getHours()*60*60;
            s += _ntp->getMinutes()*60;
            s += _ntp->getSeconds();
            set_seconds(s);
            Ramm.ntpfail = false;
        }
    }


private:
    size_t          _oftensec=0;
    size_t          _second=0;
    size_t          _minute=0;
    size_t          _hour=0;
    size_t          _daysec=0;
    size_t          _decisec=0;
    NTPClient*      _ntp = nullptr;
    WiFiUDP         _ntpUDP;
};

extern simple_clock_t  Sclk;

#endif // CLOCK_T_H
