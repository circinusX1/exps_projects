  #include "ap_wifi_ota.h"
#include <Wire.h>

int   RELAY = 0;
int   LED   = 2;

const char* STATIC_IP ="192.168.1.223";
#define FEW_HOURS (3600000*12)
RAMM __Ramm __attribute__ ((section (".noinit")));


class MyEsp : public esp32_full
{
public:
  MyEsp(int p):esp32_full(p)
  {
    _regain = millis();
  };
  virtual ~MyEsp()
  {
  };

  void    rebooting()
  {

  }

  void  user_loop(unsigned int loop)
  {
      if(_lop++%_off == 0){
        if(LED)digitalWrite(LED, _toggle=!_toggle);
      }

      if(millis()-_regain>FEW_HOURS)
      {
          Serial.println("rebooting now");
          REBOOT();
      }
      delay(10);
      return;
  }

  void  page_request(ESP8266WebServer* srv,
                     String& page)
  {

      page += "<H1>MY PAGE</H1>";
      page += _page;
      if(srv->hasArg("reboot"))
      {
            REBOOT();
      }
      if(srv->hasArg("relay"))
      {
        if(srv->arg("relay")=="1"){
           if(RELAY)digitalWrite(RELAY, LOW);
           __Ramm.relay_state = LOW;
           _replay_state=HIGH;
           _off=15;
        }
        if(srv->arg("relay")=="0"){
             if(RELAY)digitalWrite(RELAY, HIGH);
             __Ramm.relay_state = HIGH;
            _replay_state=LOW;
            _off=100;
        }
      }
      if(srv->hasArg("gpio"))
      {
          LED = srv->arg("gpio").toInt();
          pinMode(LED, OUTPUT);
          digitalWrite(LED, LOW);
          delay (500);
          digitalWrite(LED, HIGH);
          delay (500);
          digitalWrite(LED, LOW);

      }
      page += "<font size='4em'>";
      page += "<li><a href='?relay=1'>TURN ON</a>";
      page += "<li><a href='?relay=0'>TURN OFF</a>";
      page += "<li>gpio:" + String(LED);
      if(_replay_state==LOW)
        page += "<li> LED OFF";
      else
        page += "<li> LED ON";
      page+="</font>";

  }


  int _replay_state=0;
  String _page;
  int    _toggle=0;
  int    _lop = 0;
  bool   _out = false;
  uint32_t _off = 60;
  uint32_t _regain = millis();
} TheEsp(80);



void setup() {
  Serial.println("entering setup1");
  TheEsp.setup();
  Serial.println("entering setup2");
  if(RELAY)pinMode(RELAY, OUTPUT);
  if(LED)pinMode(LED, OUTPUT);
  Serial.println("exiting setup3");
  if (RELAY)digitalWrite(RELAY, __Ramm.relay_state);
  TheEsp._replay_state=!__Ramm.relay_state;

}

void loop() {

  TheEsp.loop();
}
