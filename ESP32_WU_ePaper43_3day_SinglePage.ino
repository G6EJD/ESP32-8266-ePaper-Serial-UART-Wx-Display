/*########################   Weather Display  #############################
 * Receives and displays the weather forecast from the Weather Underground and then displays using a 
 * JSON decoder wx data to display on an ePaper Display.
 * Weather data received via WiFi connection to Weather Underground Servers and using their 'Forecast' API and data
 * is decoded using Copyright Benoit Blanchon's (c) 2014-2017 excellent JSON library.
 * This source code is protected under the terms of the MIT License and is copyright (c) 2017 by David Bird and permission is hereby granted, free of charge, to
 * any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, but not to sub-license and/or 
 * to sell copies of the Software or to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 *   
 *   See more at http://dsbird.org.uk 
 */   
#include <WiFi.h>
#include <ArduinoJson.h>     // https://github.com/bblanchon/ArduinoJson
#include <WiFiClient.h>
#include "time.h"
#include "epd.h"

//################# LIBRARIES ##########################
// String version = "1.0";       // Version of this program
//################ VARIABLES ###########################
//------ NETWORK VARIABLES---------
// Use your own API key by signing up for a free developer account at http://www.wunderground.com/weather/api/
String apikey        = "----------------";            // See: http://www.wunderground.com/weather/api/d/docs (change here with your KEY)
String City          = "Melksham";                    // Your home city
String Country       = "UK";                          // Your country   
String Conditions    = "conditions";                  // See: http://www.wunderground.com/weather/api/d/docs?d=data/index&MR=1
unsigned long        lastConnectionTime = 0;          // Last time you connected to the server, in milliseconds
const unsigned long  postingInterval    = 10L*60L*1000L; // Delay between updates, in milliseconds, WU allows 500 requests per-day maximum, set to every 10-mins or 144/day
String Units      =  "X"; // M for Metric, X for Mixed and I for Imperial
String currCondString; // string to hold received weather data
String time_str;
#define large true
#define small false

//################ PROGRAM VARIABLES and OBJECTS ################
// Conditions
String WDay0, Day0, Icon0, High0, Low0, Conditions0, Pop0, Averagehumidity0,
       WDay1, Day1, Icon1, High1, Low1, Conditions1, Pop1, Averagehumidity1,
       WDay2, Day2, Icon2, High2, Low2, Conditions2, Pop2, Averagehumidity2,
       WDay3, Day3, Icon3, High3, Low3, Conditions3, Pop3, Averagehumidity3;
 
// Astronomy
String  DphaseofMoon, Sunrise, Sunset, Moonrise, Moonset, Moonlight;

const char* ssid     = "yourSSID";
const char* password = "yourPASSWORD";
const char* host     = "api.wunderground.com";

WiFiClient client; // wifi client object

void setup() { 
  Serial.begin(115200);
  epd_init();
  epd_wakeup();
  StartWiFi(ssid,password);
  SetupTime();
  Serial.println(time_str);
  lastConnectionTime = millis();
  epd_reset();
  epd_screen_rotation(3);
  epd_set_color(BLACK, WHITE);
  epd_set_en_font(ASCII32);
  epd_set_memory(MEM_TF);
  epd_load_pic(); // Transfer TF Card images to NAND FLASH (80MB for images and 40MB for Fonts=128MB Total)
  epd_set_memory(MEM_NAND);
  epd_clear();
  obtain_forecast("forecast",&currCondString);
  obtain_forecast("astronomy",&currCondString);
  DisplayForecast();
  epd_enter_stopmode();         // Put Waveshare ePaper to sleep, consumes <5mA in that state, ESP32 ~20uA
  ESP.deepSleep(10*60*1000000); // Sleep for 10 minutes
}

void loop() { // Retain for non-sleep mode; comment out line 76 ifu keep the ESP running continuosly then these (loop) lines will run
  if (millis() - lastConnectionTime > postingInterval) {
    obtain_forecast("forecast",&currCondString);
    obtain_forecast("astronomy",&currCondString);
    lastConnectionTime = millis();
    DisplayForecast();
  }
}
void DisplayForecast(){ // Display is 600x800 resolution (in portrait mode)
  int xpos, offset;
  char buff[96]={};
  clear_screen();
  DisplayWXicon(300,50, Icon0, large);
  epd_set_en_font(ASCII64);
  (City+", "+Country).toCharArray(buff,64);       epd_disp_string(buff,0,10);
  (High0 + " / " + Low0).toCharArray(buff,64);    epd_disp_string(buff,95,130);
  (Conditions0).toCharArray(buff,64);             epd_disp_string(buff,10,275);
  epd_set_en_font(ASCII32);
  ("RH: "+Averagehumidity0 + "%").toCharArray(buff,64);  epd_disp_string(buff,115,200);
  DisplayWXicon(115,230, "probrain", small);
  (Pop0 + "%").toCharArray(buff,64);              epd_disp_string(buff,170,230);
  epd_draw_line(0, 355, 600, 355);
    
  DisplayWXicon(50,400, Icon1, small); DisplayWXicon(60,550, "probrain", small);
  offset = (WDay1.length())*27/2; xpos = 150 - offset;
  (WDay1).toCharArray(buff,64);                   epd_disp_string(buff,xpos,370);
  (High1 + "/" + Low1).toCharArray(buff,64);      epd_disp_string(buff,70,520);
  (Pop1 + "%").toCharArray(buff,64);              epd_disp_string(buff,110,550);
                             
  DisplayWXicon(250,400, Icon2, small); DisplayWXicon(260,550, "probrain", small);
  offset = (WDay2.length())*27/2; xpos = 350 - offset;
  (WDay2).toCharArray(buff,64);                   epd_disp_string(buff,xpos,370);
  (High2 + "/" + Low2).toCharArray(buff,64);      epd_disp_string(buff,270,520);
  (Pop2 + "%").toCharArray(buff,64);              epd_disp_string(buff,310,550);
                                
  DisplayWXicon(450,400, Icon3, small); DisplayWXicon(460,550, "probrain", small);
  offset = (WDay3.length())*27/2; xpos = 555 - offset;
  (WDay3).toCharArray(buff,64);                   epd_disp_string(buff,xpos,370);
  (High3 + "/" + Low3).toCharArray(buff,64);      epd_disp_string(buff,470,520);
  (Pop3 + "%").toCharArray(buff,64);              epd_disp_string(buff,510,550);
  epd_draw_line(0, 600, 600, 600);
                                                  epd_disp_string("Sunrise/Set",0,620);
  (Sunrise+"/"+Sunset).toCharArray(buff,64);      epd_disp_string(buff,0,650);
                                                  epd_disp_string("Moonrise/Set",150,620);
  (String(Moonrise)+"/"+String(Moonset)).toCharArray(buff,64);    epd_disp_string(buff,155,650);
                                                  epd_disp_string("Moonphase",325,620);
  (DphaseofMoon+" "+Moonlight+"%").toCharArray(buff,64); epd_disp_string(buff,325,650);
  epd_draw_line(0, 700, 600, 700); epd_draw_line(145, 700, 145, 600); epd_draw_line(318, 700, 318, 600);
  UpdateLocalTime();
  ("Last updated: "+ time_str).toCharArray(buff,64);
  epd_disp_string(buff,50,770);
  epd_update();
}

void DisplayWXicon(int x, int y, String IconName, bool imgsize){
  Serial.println(IconName);
  if      (IconName == "rain"            || IconName == "nt_rain"         ||
           IconName == "chancerain"      || IconName == "nt_chancerain")
           if (imgsize == large) {epd_disp_bitmap("LRAIN.JPG",x,y);} else {epd_disp_bitmap("RAIN.JPG",x,y);}
  else if (IconName == "snow"            || IconName == "nt_snow"         ||
           IconName == "flurries"        || IconName == "nt_flurries"     ||
           IconName == "chancesnow"      || IconName == "nt_chancesnow"   ||
           IconName == "chanceflurries"  || IconName == "nt_chanceflurries")
           if (imgsize == large) epd_disp_bitmap("LSNOW.JPG",x,y); else epd_disp_bitmap("SNOW.JPG",x,y);
  else if (IconName == "sleet"           || IconName == "nt_sleet"        ||
           IconName == "chancesleet"     || IconName == "nt_chancesleet")
           if (imgsize == large) epd_disp_bitmap("LSLEET.JPG",x,y); else epd_disp_bitmap("SLEET.JPG",x,y);
  else if (IconName == "sunny"           || IconName == "nt_sunny"        ||
           IconName == "clear"           || IconName == "nt_clear")
           if (imgsize == large) epd_disp_bitmap("LSUNNY.JPG",x,y); else epd_disp_bitmap("SUNNY.JPG",x,y);
  else if (IconName == "partlysunny"     || IconName == "nt_partlysunny"  ||
           IconName == "mostlysunny"     || IconName == "nt_mostlysunny")
           if (imgsize == large) epd_disp_bitmap("LMSUNN.JPG",x,y); else epd_disp_bitmap("MSUNNY.JPG",x,y);
  else if (IconName == "cloudy"          || IconName == "nt_cloudy")  
           if (imgsize == large) epd_disp_bitmap("LCLOUD.JPG",x,y); else epd_disp_bitmap("CLOUDY.JPG",x,y);
  else if (IconName == "mostlycloudy"    || IconName == "nt_mostlycloudy" ||
           IconName == "partlycloudy"    || IconName == "nt_partlycloudy")  
           if (imgsize == large) epd_disp_bitmap("LPCLOU.JPG",x,y); else epd_disp_bitmap("PCLOUD.JPG",x,y);
  else if (IconName == "tstorms"         || IconName == "nt_tstorms"      ||
           IconName == "chancetstorms"   || IconName == "nt_chancetstorms")
           if (imgsize == true) epd_disp_bitmap("LTSTOR.JPG",x,y); else epd_disp_bitmap("TSTORM.JPG",x,y);
  else if (IconName == "fog"             || IconName == "nt_fog"          ||
           IconName == "hazy"            || IconName == "nt_hazy")
           if (imgsize == large) epd_disp_bitmap("LFOG.JPG",x,y); else epd_disp_bitmap("FOG.JPG",x,y); // Fog and Hazy images are identical
  else if (IconName == "thermo")
           epd_disp_bitmap("TEMPH.BMP",x,y);
  else if (IconName == "probrain")
           epd_disp_bitmap("PROBR.BMP",x,y);
  else     epd_disp_bitmap("QUEST.BMP",x,y);
}

bool obtain_forecast (String forecast_type, String* currCondString) {
  client.stop();  // Clear any current connections
  Serial.println("Connecting to "+String(host)); // start a new connection
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
   Serial.println("Connection failed");
   return false;
  }
  // Weather Underground API address http://api.wunderground.com/api/YOUR_API_KEY/conditions/q/YOUR_STATE/YOUR_CITY.json
  String url = "http://api.wunderground.com/api/"+apikey+"/"+forecast_type+"/q/"+Country+"/"+City+".json";
  Serial.println("Requesting URL: "+String(url));
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
  "Host: " + host + "\r\n" +
  "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Connection Timeout...Stopping");
      client.stop();
      return false;
    }
  }
  Serial.print("Receiving API weather data");
  while(client.available()) {
    *(currCondString) = client.readStringUntil('\r');
    Serial.print(".");
  }
  Serial.println("\r\nClosing connection");
  //Serial.println(*currCondString);
  if (forecast_type == "forecast"){
    showWeather_forecast(currCondString); 
  }
  if (forecast_type == "astronomy"){
    showWeather_astronomy(currCondString); 
  }
  return true;
}

void showWeather_astronomy(String* currCondString) {
  Serial.println("Creating object...");
  DynamicJsonBuffer jsonBuffer(1*1024);
  // Create root object and parse the json file returned from the api. The API returns errors and these need to be checked to ensure successful decoding
  JsonObject& root = jsonBuffer.parseObject(*(currCondString));
  if (!root.success()) {
    Serial.println(F("jsonBuffer.parseObject() failed"));
  }
  // Extract weather info from parsed JSON
  JsonObject& current = root["moon_phase"];
  String percentIlluminated = current["percentIlluminated"];
  String phaseofMoon = current["phaseofMoon"];
  int SRhour         = current["sunrise"]["hour"];
  int SRminute       = current["sunrise"]["minute"];
  int SShour         = current["sunset"]["hour"];
  int SSminute       = current["sunset"]["minute"];
  int MRhour         = current["moonrise"]["hour"];
  int MRminute       = current["moonrise"]["minute"];
  int MShour         = current["moonset"]["hour"];
  int MSminute       = current["moonset"]["minute"];
  Sunrise   = (SRhour<10?"0":"")+String(SRhour)+":"+(SRminute<10?"0":"")+String(SRminute);
  Sunset    = (SShour<10?"0":"")+String(SShour)+":"+(SSminute<10?"0":"")+String(SSminute);
  Moonrise  = (MRhour<10?"0":"")+String(MRhour)+":"+(MRminute<10?"0":"")+String(MRminute);
  Moonset   = (MShour<10?"0":"")+String(MShour)+":"+(MSminute<10?"0":"")+String(MSminute);
  Moonlight = percentIlluminated;
  DphaseofMoon = phaseofMoon;
}

void showWeather_forecast(String* currCondString) {
  Serial.println("Creating object...");
  DynamicJsonBuffer jsonBuffer(8*1024);
  // Create root object and parse the json file returned from the api. The API returns errors and these need to be checked to ensure successful decoding
  JsonObject& root = jsonBuffer.parseObject(*(currCondString));
  if (!root.success()) {
    Serial.println(F("jsonBuffer.parseObject() failed"));
  }
  // Parse JSON
  if (!root.success()) {
    Serial.println(F("jsonBuffer.parseObject() failed"));
  }
  JsonObject& forecast = root["forecast"]["simpleforecast"];
  String wday0  = forecast["forecastday"][0]["date"]["weekday"];        WDay0 = wday0;
  int    day0   = forecast["forecastday"][0]["date"]["day"];            day0<10?(Day0="0"+String(day0)):(Day0=String(day0));
  String mon0   = forecast["forecastday"][0]["date"]["monthname_short"];
  String year0  = forecast["forecastday"][0]["date"]["year"];           Day0 += "-" + mon0 + "-" + year0.substring(2); 
  String icon0  = forecast["forecastday"][0]["icon"];                   Icon0 = icon0;
  String high0  = forecast["forecastday"][0]["high"]["celsius"];        High0 = high0;
  String low0   = forecast["forecastday"][0]["low"]["celsius"];         Low0  = low0;
  String conditions0 = forecast["forecastday"][0]["conditions"];        Conditions0  = conditions0;
  String pop0        = forecast["forecastday"][0]["pop"];               Pop0  = pop0;
  String averagehumidity0 = forecast["forecastday"][0]["avehumidity"];  Averagehumidity0 = averagehumidity0;

  String wday1  = forecast["forecastday"][1]["date"]["weekday"];        WDay1 = wday1;
  int    day1   = forecast["forecastday"][1]["date"]["day"];            day1<10?(Day1="0"+String(day1)):(Day1=String(day1));
  String mon1   = forecast["forecastday"][1]["date"]["monthname_short"];
  String year1  = forecast["forecastday"][1]["date"]["year"];           Day1 += "-" + mon1 + "-" + year1.substring(2); 
  String icon1  = forecast["forecastday"][1]["icon"];                   Icon1 = icon1;
  String high1  = forecast["forecastday"][1]["high"]["celsius"];        High1 = high1;
  String low1   = forecast["forecastday"][1]["low"]["celsius"];         Low1  = low1;
  String conditions1 = forecast["forecastday"][1]["conditions"];        Conditions1  = conditions1;
  String pop1   = forecast["forecastday"][1]["pop"];                    Pop1  = pop1;
  String averagehumidity1 = forecast["forecastday"][1]["avehumidity"];  Averagehumidity1 = averagehumidity1;
  
  String wday2  = forecast["forecastday"][2]["date"]["weekday"];        WDay2 = wday2;
  int    day2   = forecast["forecastday"][2]["date"]["day"];            day2<10?(Day2="0"+String(day2)):(Day2=String(day2));
  String mon2   = forecast["forecastday"][2]["date"]["monthname_short"];
  String year2  = forecast["forecastday"][2]["date"]["year"];           Day2 += "-" + mon2 + "-" + year2.substring(2); 
  String icon2  = forecast["forecastday"][2]["icon"];                   Icon2 = icon2;
  String high2  = forecast["forecastday"][2]["high"]["celsius"];        High2 = high2;
  String low2   = forecast["forecastday"][2]["low"]["celsius"];         Low2  = low2;
  String conditions2 = forecast["forecastday"][2]["conditions"];        Conditions2  = conditions2;
  String pop2   = forecast["forecastday"][2]["pop"];                    Pop2  = pop2;
  String averagehumidity2 = forecast["forecastday"][2]["avehumidity"];  Averagehumidity2 = averagehumidity2;

  String wday3  = forecast["forecastday"][3]["date"]["weekday"];        WDay3 = wday3;
  int    day3   = forecast["forecastday"][3]["date"]["day"];            day3<10?(Day3="0"+String(day3)):(Day3=String(day3));
  String mon3   = forecast["forecastday"][3]["date"]["monthname_short"];
  String year3  = forecast["forecastday"][3]["date"]["year"];           Day3 += "-" + mon3 + "-" + year3.substring(2); 
  String icon3  = forecast["forecastday"][3]["icon"];                   Icon3 = icon3;
  String high3  = forecast["forecastday"][3]["high"]["celsius"];        High3 = high3;
  String low3   = forecast["forecastday"][3]["low"]["celsius"];         Low3  = low3;
  String conditions3 = forecast["forecastday"][3]["conditions"];        Conditions3  = conditions3;
  String pop3   = forecast["forecastday"][3]["pop"];                    Pop3  = pop3;
  String averagehumidity3 = forecast["forecastday"][3]["avehumidity"];  Averagehumidity3 = averagehumidity3;
}

int StartWiFi(const char* ssid, const char* password){
 int connAttempts = 0;
 Serial.print(F("\r\nConnecting to: ")); Serial.println(String(ssid));
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED ) {
   delay(500); Serial.print(".");
   if(connAttempts > 20) return -5;
   connAttempts++;
 }
 Serial.print(F("WiFi connected at: "));
 Serial.println(WiFi.localIP());
 return 1;
}

void clear_screen() {
   epd_clear();
}  

void SetupTime(){
  configTime(0 * 3600, 3600, "0.uk.pool.ntp.org", "time.nist.gov");
  UpdateLocalTime();
}

void UpdateLocalTime(){
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  }
  //See http://www.cplusplus.com/reference/ctime/strftime/
  Serial.println(&timeinfo, "%a %b %d %Y   %H:%M:%S"); // Displays: Saturday, June 24 2017 14:05:49
  time_str = asctime(&timeinfo); // Displays: Sat Jun 24 14:05:49 2017
  time_str = time_str.substring(0,24); // Displays: Sat Jun 24 14:05
}



