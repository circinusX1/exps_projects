# esp32-template
template for ESP-01 wifi and access point and OTA from the Arduino IDE
Also is using the STH TH sensor over the I2C on the UART pins solderred on pin 2 and 7 of the ESP-01 mini board.


   * will start as AP  with HVAC name. Join this network
   * open in browser 10.5.5.1/wifi
   * select the ssid, pass and the staic IP
   * save
   * power cycle

Use ino to expand the functionality. 
The RELAY GPIO is set for the newer models of ESP8266-ESP-01S
The following pages are available
    * /wifi   - config wifi
    * /ota    - then start the OTA or power cycle, does not get's out frmo ota state by browsing
    * /?param=value - > in your object handler. Check params and append to the page.
    * Uses USRT pins for I2C while reading the I2c bus
    
    
