#include "_my_config.h"
#include "_utils.h"
#include "hot_ram.h"

ram_str_t    Ramm;

#define  RTC_MEM_START 64

void hot_store()
{
    Ramm.sig = RAM_SIG;
    ::system_rtc_mem_write(RTC_MEM_START, &Ramm, sizeof(Ramm));
}

bool hot_restore()
{
    ::system_rtc_mem_read(RTC_MEM_START, &Ramm, sizeof(Ramm));
    if(Ramm.sig != RAM_SIG)
    {
        LOG("Init RAM BLANK");
        Ramm.loopfail = 0;
        Ramm.ntpfail = 0;
        Ramm.relay = 0;
        Ramm.seconds = 0;
        Ramm.sig = RAM_SIG;
    }
    else
    {
        LOG("RELOAD RAM");
    }
    return Ramm.sig == RAM_SIG;
}
