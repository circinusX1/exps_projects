#ifndef HOT_RAM_C_H
#define HOT_RAM_C_H

//////////////////////////////////////////////////////////////////////////////////////////
#define RAM_SIG 0x01

//////////////////////////////////////////////////////////////////////////////////////////
struct ram_str_t{
    uint8_t     sig;
    bool        loopfail;
    bool        ntpfail;
    uint8_t     relay;
    size_t      seconds;
};


extern ram_str_t    Ramm;
bool hot_restore();
void hot_store();


#endif // HOT_RAM_C_H
