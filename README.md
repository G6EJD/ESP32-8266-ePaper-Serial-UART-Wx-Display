# ESP32-8266-ePaper-Serial-UART-Wx-Display

To use the Weather Display display, you must copy the icons to an SD_Card and insert it int othe 4.3" e-Paper display card reader. 


Connect your controller (e.g. ESP32 ) to the display in accordance with the wiring diagram.

MoOdifiy the code to add your WiFi SSID and PASSWORD, change your City and Country, compile and then upload code to ESP32.

Be prepared for things to take a relatively long time, when the programme starts it resets the ESP32 (display) control ports, then issues a wake-up command to the display, then sends a command (epd_load_pic) to load from SD card to FLash memery the icons, then it waits for the display data. The baud rate is 115200, so that's data at a rate of 11520bytes/sec or about 11.5KB/sec so if transferring large images these can take a while. For example a 250KB image woudl take 22-secs to transfer. You could increase the default baud rate, but Waveshare says don't do this very often.

Enjoy
