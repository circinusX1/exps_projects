#ifndef CLOCK_T_H
#define CLOCK_T_H

#include "_utils.h"
#include "eeprom.h"
#include "_esp_mpus.h"
#include "hot_ram.h"
#define USE_US_TIMER // microsecond timer
#include <osapi.h>

class simple_clock_t
{
    uint64_t _microS;
    uint64_t _micros;
public:
    simple_clock_t(){
        _microS = micros64();
    };
    ~simple_clock_t(){
#if WITH_NTP
        delete _ntp;
#endif
    };

    void tick(){
        _micros = micros64()-_microS;
        ++_oftensec;
        ++_decisec;
        if(_oftensec==10){
            _oftensec = 0;
            ++_second;
            ++_daysec;
            if(_second==60){
                _second=0;
                ++_minute;
                ++_minday;
                if(_minute==60){
                    ++_hour;
                    if(_hour==24){
                        _hour=0;
                        _daysec=0;
                        _decisec=0;
                        _minday =0;
                    }
                }
            }
        }
    }

    size_t seconds()const{return _daysec;}
    size_t decisec()const{return _decisec;}
    size_t minutes()const{return _minday;}
    size_t hours()const{return _hour;}

    FORCE_INLINE unsigned long diff_time(size_t now, size_t start) {
        if(0==start) return 0xFFFFFFFF;         // edge condition, low prpbbility
        if (now >= start) return now - start;
        else return 0xFFFFFFFF - start + now + 1;
    }
    FORCE_INLINE unsigned long msElapsed(size_t start){
        unsigned long now = ::millis();
        return diff_time(now, start);
    }


    void set_seconds(size_t seconds){
        _daysec = seconds;
        _minday = seconds/60;
        _second = seconds % 60;
        _hour     = seconds/3600;
        _minute   = (seconds%3600)/60;
        LOG("SET TIME: %d:%d:%d  %d %d", _hour, _minute, _second, _minday, _daysec);
    }

    void init_ntp()
    {
#if WITH_NTP
        LOG("CREATE NTP TO %d", CFG(ntp_offset)*3600);
        if(Ramm.ntpfail==false)
        {
            Ramm.ntpfail = true;
            _ntp = new NTPClient(_ntpUDP,CFG(ntp_srv),CFG(ntp_offset)*3600, 600000);
            _ntp->begin();
            Ramm.ntpfail = false;
        }
#endif
    }

    void update_ntp()
    {
#if WITH_NTP
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
#endif
    }
    void    flash(int on)
    {
        system_print_meminfo();
        LOG("ull=%d, max days=%llu chipid=%d rtc=%d",
                            sizeof(uint64_t),
                            ((int64_t)(-1))/86400000000);
//        if(on==0)
//            system_timer_reinit();   // screws up clocks
        _microS = micros64();
        size_t xm = millis();
        LOG("   RTC = %lld", system_get_rtc_time());
        LOG("   MICROS = %lld %lld", _microS, xm);
        delay(1);
        LOG("   1MS = %lld",  micros64()-_microS);
        delay(100);
        LOG("   100MS = %lld",  micros64()-_microS);
    }

private:
    int             _oftensec=0;
    int             _second=0;
    int             _minute=0;
    int             _hour=0;
    int             _daysec=0;
    size_t          _decisec=0;
    int             _minday=0;
#if WITH_NTP
    NTPClient*      _ntp = nullptr;
    WiFiUDP         _ntpUDP;
#endif
};

extern simple_clock_t  Sclk;

#endif // CLOCK_T_H
