#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include "LedControl.h"
#include <TimeLib.h>
#include <Ticker.h>
#include <ESP8266HTTPClient.h>

const char *ssid     = "ssid";
const char *password = "password";

int counter;
int clockminusstatus;
int setupledstatus;
int p1 = D8;
int p2 = D7;
int p3 = D6;
int httpCode,tempfinal;
String response;
int timezone_value = 2;
int brightness = 0;
#define timezone true //true = GMT+ false = GMT-

WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP, "tempus1.gum.gov.pl");
LedControl lc=LedControl(p1,p2,p3,1);
Ticker setupled;
Ticker updatetemp;
Ticker brightbutton;
HTTPClient http;

void updatehttp(){
   http.begin("http://api.thingspeak.com/apps/thinghttp/send_request?api_key=CRAE0CW7DF3YSLQ5");
   httpCode = http.GET();
   if(httpCode > 0 && httpCode == HTTP_CODE_OK){
  response = http.getString();
  int replaceindex = response.indexOf("<");
  response.remove(replaceindex);
  Serial.println(response);
  tempfinal = response.toInt();
  Serial.println(tempfinal);
}
}
void writeTemp() {
  int seg1,seg2;
  seg1 = tempfinal%10;
  seg2 = tempfinal/10;
  if(response.startsWith("-")){
    lc.setChar(0,6,'-',false);
  }
  if(seg2 == 0){
  } else {
  lc.setChar(0,5,seg2,false);
  }
  lc.setChar(0,4,seg1,false);
  //Degree
  lc.setLed(0,3,1,true);
  lc.setLed(0,3,2,true);
  lc.setLed(0,3,6,true);
  lc.setLed(0,3,7,true);
  //Capital "C" letter
  lc.setLed(0,2,1,true);
  lc.setLed(0,2,6,true);
  lc.setLed(0,2,4,true);
  lc.setLed(0,2,5,true);
}
void blinkDot() {
  setupledstatus = ~setupledstatus;
  if(setupledstatus == -1){
   for(int i=0;i<=7;i++){
   lc.setChar(0,i,' ', true);
   }
  } else {
   for(int i=0;i<=7;i++){
   lc.setChar(0,i,' ', false);
   }
  }
}
void writeClock(int h,int m, int s) {
  int seg1,seg2,seg3,seg4,seg5,seg6;
  if(timezone == true){
   h = h + timezone_value;
  } else if(timezone == false) {
    h = h - timezone_value;
  }
  seg1 = h/10;
  seg2 = h-seg1*10;
  seg3 = m/10;
  seg4 = m-seg3*10;
  seg5 = s/10;
  seg6 = s-seg5*10;
  lc.setChar(0,7,seg1,false);
  lc.setChar(0,6,seg2,false);
  lc.setChar(0,4,seg3,false);
  lc.setChar(0,3,seg4,false);
  lc.setChar(0,1,seg5,false);
  lc.setChar(0,0,seg6,false);
  clockminusstatus = ~clockminusstatus;
  if(clockminusstatus == -1){
    lc.setChar(0,2,'-',false);
    lc.setChar(0,5,'-',false);
  } else {
    lc.setChar(0,2,' ',false);
    lc.setChar(0,5,' ',false);
  }
}

void writeDate(int d, int m1, int y){
  lc.clearDisplay(0);
  int seg11,seg22,seg33,seg44,seg55,seg66;
  y = y%1000;
  seg11 = d/10;
  seg22 = d-seg11*10;
  seg33 = m1/10;
  seg44 = m1-seg33*10;
  seg55 = y/10;
  seg66 = y-seg55*10;
  lc.setChar(0,7,seg11,false);
  lc.setChar(0,6,seg22,false);
  lc.setChar(0,5,'-',false);
  lc.setChar(0,4,seg33,false);
  lc.setChar(0,3,seg44,false);
  lc.setChar(0,2,'-',false);
  lc.setChar(0,1,seg55,false);
  lc.setChar(0,0,seg66,false);
}
void testbutton(){
  if(digitalRead(D3) == LOW){
    brightness++;
    if(brightness == 4){
      brightness = 0;
    }
  }
  switch(brightness) {
    case 0:
    lc.setIntensity(0,8);
    break;
    case 1:
    lc.setIntensity(0,4);
    break;
    case 2:
    lc.setIntensity(0,2);
    break;
    case 3:
    lc.setIntensity(0,0);
    break;
  }
}
void setup() {
  pinMode(D3,INPUT_PULLUP);
  lc.shutdown(0,false);
  lc.setIntensity(0,8);
  lc.clearDisplay(0);
  setupled.attach(0.2, blinkDot);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  ntpClient.begin();
  setupled.detach();
  updatetemp.attach(30, updatehttp);
  updatehttp();
  brightbutton.attach(0.2, testbutton);
}
void loop() {
 ntpClient.update();
 if(counter == 30){
  lc.clearDisplay(0);
  time_t dateCalc = ntpClient.getEpochTime(); 
  writeDate(day(dateCalc),month(dateCalc),year(dateCalc)%100);
  delay(5000);
  lc.clearDisplay(0);
  writeTemp();
  delay(5000);
  lc.clearDisplay(0);
  counter = 0;
 } else {
  writeClock(ntpClient.getHours(), ntpClient.getMinutes(),ntpClient.getSeconds());
  counter++;
  delay(1000);
 }
}

