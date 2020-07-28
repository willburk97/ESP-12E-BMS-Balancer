// Tutorial Link: https://learn.adafruit.com/adafruit-io-basics-analog-input
//
// Adafruit invests time and resources providing (...a small part of...) this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// (The adafruit io portion of this was...) Written by Todd Treece for Adafruit Industries
// Copyright (c) 2016 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

// All other code below was written by William Burk for himself or copied and pasted from somewhere else. (Sorry Todd for making a mess of your code.)


#define IO_USERNAME "yourusernamehere"
#define IO_KEY "yourkeyhere"
#define WIFI_SSID "yourssidhere"
#define WIFI_PASS "yourpasswordhere"
#include "AdafruitIO_WiFi.h"
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

#define analogIn 34
#define LEDPin 2
#define mosfetPin 32
#define dischargeLEDpin 25
float sensorValue = 0;
float allowedCellDiff = 0.05;
float lowBatt = 10;
int thisBatt = 3;  // This means Battery 3
bool dischargeState = false;
int sensorInt;
unsigned long lastTxTime = 0;
unsigned int minTxTime = 60000;
unsigned int sampleTime = 1000;
String debugText = "0.00 0.00 0.00 0.00 ";
unsigned int dischargeTime = 0;
int i;
float battVolt[4];
AdafruitIO_Feed *espbattery = io.feed("espbattery");
AdafruitIO_Feed *espbattery2 = io.feed("espbattery2");
AdafruitIO_Feed *espbattery3 = io.feed("espbattery3");
AdafruitIO_Feed *espbattery4 = io.feed("espbattery4");
//AdafruitIO_Feed *debugtext = io.feed("debug-text");
AdafruitIO_Feed *battdischarge = io.feed("battdischarge4");
//AdafruitIO_Feed *battdischargetime1 = io.feed("battdischargetime1");

void setup() {
  pinMode(LEDPin, OUTPUT);
  pinMode(mosfetPin, OUTPUT);
  pinMode(dischargeLEDpin, OUTPUT);
//  adc1_config_channel_atten(ADC1_CHANNEL_0,0);
  // start the serial connection
  Serial.begin(115200);

  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());

  espbattery->onMessage(getBatt);
  espbattery2->onMessage(getBatt2);
  espbattery3->onMessage(getBatt3);

  espbattery->get();
  espbattery2->get();
  espbattery3->get();
}

void loop() {
  io.run();

  updateVoltage();

  lowBatt = 10;
  for (i=0;i<4;i++) {
    if (battVolt[i] < lowBatt) { lowBatt = battVolt[i]; }
  }

  digitalWrite(LEDPin, HIGH);
  delay(30);
  digitalWrite(LEDPin, LOW);
  
      debugText = "";
      for (i=0;i<4;i=i+1) {
        Serial.print(battVolt[i]); Serial.print(" ");
        debugText += String(battVolt[i]);
        debugText += String(" ");
      }
      Serial.println();
  if(millis() > lastTxTime + minTxTime)
  {
    lastTxTime = millis();
    Serial.print("Sent Last");

    if (battVolt[thisBatt] <= lowBatt || battVolt[thisBatt] < 3.0) {
      digitalWrite(mosfetPin, false);
      digitalWrite(dischargeLEDpin, false);
      dischargeState = false;
      Serial.println("Battery not Draining");
//      updateVoltage();
    }

    espbattery4->save(sensorValue);
//    debugtext->save(debugText);
    battdischarge->save(dischargeState);
//    battdischargetime1->save(dischargeTime);
    
    digitalWrite(LEDPin, HIGH);
    delay(100); // just to be safe


    espbattery->get();
    espbattery2->get();
    espbattery3->get();

    digitalWrite(LEDPin, LOW);

  }
    
  if (dischargeState) { dischargeTime++; }
  
  if ((millis() > 30000) && (battVolt[thisBatt] > (lowBatt + allowedCellDiff)) && (battVolt[thisBatt] > 3.0)) {
    digitalWrite(mosfetPin, true);
    digitalWrite(dischargeLEDpin, true);
    dischargeState = true;
    Serial.println("Battery Draining");
  }
  
  delay(sampleTime-250);

}

void updateVoltage() {

  digitalWrite(mosfetPin, false);
  
  int sensorInt = 0;
  for (i=0;i<10;i=i+1) {
    delay(25);
    sensorInt = sensorInt + analogRead(analogIn);
  }
  sensorInt = sensorInt / 10;
  Serial.println(sensorInt);
  sensorValue = (sensorInt * 2.13 * 3.3) / 4096;
  sensorInt = (sensorValue * 100);     // Round to two decimal places...
  sensorValue = sensorInt;
  sensorValue = sensorValue / 100;
  
  battVolt[thisBatt] = sensorValue;
  
  digitalWrite(mosfetPin, dischargeState);
}

void getBatt(AdafruitIO_Data *data) {
  battVolt[0] = data->toFloat();
}
void getBatt2(AdafruitIO_Data *data) {
  battVolt[1] = data->toFloat();
}
void getBatt3(AdafruitIO_Data *data) {
  battVolt[2] = data->toFloat();
}
