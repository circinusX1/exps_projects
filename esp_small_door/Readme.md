
# HACKING SMART PLUG GLOBE ELECTRIC
# Set up for TYVES2S Globe smart plug

    * LED Fast blink (3 Hz) -> Access Point mode Name 'door'
    * LED 2 Hz blink, relay ON
    * LED 0.2 Hz small blink relay OFF
        * ON PC: Connect to AP door access point,  password: doordoor
        * ON PC: Open browser and type http://10.5.5.1
        * ON PC in WebBrowser: Configure the PLUG it for your wifi and Apply
    * On PC: Connect back your wifi router
    * On PC: browse http://IP.YOU.HAVE.SET (Usually 192.168.1.###), mine is http://192.168.1.162  
        * On PLUG: if the relay or LED does not work check the GPIO's
        * On PC: Test each GPIO, http://19.168.1.162?gpio=2 then 3 and s on on up to 15
             * On PC in Arduino IDE: Open the esp_small_door.ino project
             * On PC in Arduino IDE: File->Search for LED and RELAY and change them with indenfied GPIO's
             * On PC in Arduino IDE:Compile
             * On PC in Web browser: Goto the UPDATE link in the menu bar of the PLUG page or type: http://192.168.1.162/ota
             * On OC, in Arduino IDE in the bottom build pane see where the elf file is created.
             * On PC in Web browser: Click Browse... button and select esp_+small_door.ino.bin file
             * Press Update.  
             * Wait for Okay or until the LED or RELAY clicks.      
	* On PC in web browse back the http://192.168.1.162	

#### Enjoy.

