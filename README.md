# ESP32-8266-ePaper-Serial-UART-Wx-Display

To use the Weather Display display, you must copy the icons to an SD-Card and insert it into the 4.3" e-Paper display card reader. 

Connect your controller (e.g. ESP32 ) to the display in accordance with the wiring diagram. Don't use RX0/TX0 unless you like making difficult for yourself!

Modify the code to add your WiFi SSID and PASSWORD, change your City and Country, compile and then upload code to ESP32.

Be prepared for things to take a relatively long time, when the programme starts it resets the ESP32 (display) control ports, then issues a wake-up command to the display, then sends the command (epd_load_pic) to load all images from the SD card to FLash memery the icons, then it waits for the display data t obe received before updating the screen. The baud rate is 115200, so that's data at a rate of 11520bytes/sec or about 11.5KB/sec so if transferring large images these can take a while. For example a 250KB image woudl take 22-secs to transfer. You could increase the default baud rate, but Waveshare says don't do this very often.

You can print a degree symbol with this sequence (0xA1, 0xE3) so String("Temp:"+ String(temp) + char(0xA1) + char(0xE3)+"C") will print a degree symbol like this 'Temp: 26.1° C' it (annoyingly) adds a space after the degree symbol! Ask Waveshare why it does that! E.g.

(High0 + char(0xA1) + char(0xE3)).toCharArray(buff,64);

epd_disp_string(buff,x,y);

Enjoy
