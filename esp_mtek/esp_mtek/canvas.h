#ifndef CANVAS_H
#define CANVAS_H
#include "_my_config.h"
#if WITH_GRAPH
#include "_esp_mpus.h"
#include "sensors.h"

#define MINS_PER_DAY        1440
#define SAMPLES_PER_DAY     288  //steps of 5 min
#define TIME_STEP           5


class sensor_th_t;
class canvas
{
public:
    canvas(int w=700, int h=400);
    ~canvas(){};
    void  set_trigger(const th_t& trigger)
    {
        _trigger = trigger;
    }
    void  add_th(int now, const sensor_th_t* ps, uint8_t relay_state)
    {

        int dpg = 0;
        _minutes = now/TIME_STEP;
        const th_t& th = ps->get_th();
        //LOG("TH addin [%d %d] = %dC %d%%", _minutes, now, int(th.t*100), int(th.h*100));
        _graph[_minutes].t = th.t;
        _graph[_minutes].h = th.h;
        relay_state ? set_bit(_minutes) : clear_bit(_minutes);
    }
    void    draw(String& out);
private:

    inline int bindex(int b) { return b / 8; }
    inline int boffset(int b) { return b % 8; }

    void set_bit(int b) {
        _relay[bindex(b)] |= 1 << (boffset(b));
    }
    void clear_bit(int b) {
        _relay[bindex(b)] &= ~(1 << (boffset(b)));
    }
    int get_bit(int b) {
        return _relay[bindex(b)] & (1 << (boffset(b)));
    }
private:
    int     _w,_h;
    int     _minutes;
    int     _has;
    th_t    _trigger = {0,0};
    th_t    _graph[SAMPLES_PER_DAY];
    uint8_t _relay[SAMPLES_PER_DAY/8+1];
};
#endif
#endif // CANVAS_H
