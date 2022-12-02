#ifndef HTML5_C_H
#define HTML5_C_H
#include "_config.h"
#include "i2c_senz.h"

#if WITH_GRAPH
#define DAY_OF5 289
#define FIVE    5
#define FIVE_IN_SECS    (5*60)

struct thp_str_t;
class html5_c
{
public:
    html5_c();

    void begin(int w, int h);
    void page(String& page);
    void loop(const thp_str_t& sample, int seconds);

public:
    thp_str_t   _thp;
    thp_str_t   _samples[DAY_OF5];
    int         _index;
    int         _w,_h;
    int         _minutes;
    int         _has = 0;
};

#endif

#endif // HTML5_C_H
