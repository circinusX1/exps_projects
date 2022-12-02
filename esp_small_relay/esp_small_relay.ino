#include "ap_wifi_ota.h"

class MyEsp : public esp32_full
{
public:
  MyEsp(byte a,byte b,byte c,byte d,int p):esp32_full(a,b,c,d,p)
  {
    
  };
  virtual ~MyEsp()
  {
  };

  void  user_loop(unsigned int)
  {
    
      return;
  }
  
  void  page_request(const String getput,
                               const String& path,
                               String& page)
  {
      page += "GOT TO MY HANDLER. Page path" + path;


  }
  
} TheEsp(192,168,1,181,80);



void setup() {
  TheEsp.setup();

}

void loop() {
  TheEsp.loop();
}
