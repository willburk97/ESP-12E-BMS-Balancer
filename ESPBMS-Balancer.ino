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


#define IO_USERNAME "Your adafruit username here"
#define IO_KEY "your adafruit key here"
#define WIFI_SSID "yourSSID"
#define WIFI_PASS "yourwifipassword"
#include "AdafruitIO_WiFi.h"
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

#define analogIn A0  // The NodeMCU ESP12 has a built in 100k:220k Voltage divider so I just jumpered A0 to my battery input with a 220k resistor so it's a 1:5.4 ratio.
      // Note: Vin on the NodeMCU uses a LDO voltage regulator which wouldn't allow me to drain (or use) my battery as low as I wanted. So I ended up using the adafruit
      // LM3671 3.3v buck converter and feeding the 3.3 input on the NodeMCU directly.
#define LEDPin D0    //This is the builtin LED on the NodeMCU, it monitors communications.  On at first and then intermittent flash for comms.
#define mosfetPin D1  // This goes high to activate the mosfet (IRFZ44N) I use to discharge the battery through a 16ohm/3w resistor (4.0v li-ion, 250mA, 1 W drain)
#define dischargeLEDpin D2  //This LED shows me when the battery is being drained
float sensorValue = 0;
float allowedCellDiff = 0.05;
float lowBatt = 10;
bool dischargeState = false;
int sensorInt;
unsigned long lastTxTime = 0;

int thisBatt = 1;   // 0 referenced number of the cell this ESP12 is monitoring. This code is for cell 2.

// You have to manually change the code for each cell in the 'pack.' (This is cell 2 so we're not monitoring it's time.)
unsigned long battTime;
//unsigned long battTime2;
unsigned long battTime3;
unsigned long battTime4;

int staleData = 0;  // Binary coded stale data monitor. 0000 means current data. 0100 means Cell 3 is stale. (i.e. no updates getting to/from adafruit io)


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
//AdafruitIO_Feed *debugtext = io.feed("debug-text");  // Activate this on one ESP12 and you'll have a quick text reference for each cell voltage.

// **You have to manually change the code for each cell in the 'pack.' (This is cell 2 so we upload it's draining status.) You only have to adjust what's in quotes.
AdafruitIO_Feed *battdischarge = io.feed("battdischarge2");

//AdafruitIO_Feed *battdischargetime = io.feed("battdischargetime2");   // This would enable us to monitor how much a cell has been drained. I turned it off to minimize updates to adafruit io.
// The above must be updated to mactch the cell connected to this ESP12.

void setup() {
  pinMode(LEDPin, OUTPUT);
  pinMode(mosfetPin, OUTPUT);
  pinMode(dischargeLEDpin, OUTPUT);
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

// You have to manually change the code for each cell in the 'pack.' (This is cell 2 so we're not requesting it's voltage from adafruit.)
  espbattery->onMessage(getBatt);
//  espbattery2->onMessage(getBatt2);
  espbattery3->onMessage(getBatt3);
  espbattery4->onMessage(getBatt4);

//These force an initial update from adafruit io.
  espbattery->get();
//  espbattery2->get();
  espbattery3->get();
  espbattery4->get();
}

void loop() {
  io.run();

  updateVoltage();

  lowBatt = 10;
  for (i=0;i<4;i++) {
    if (battVolt[i] < lowBatt) { lowBatt = battVolt[i]; }
  }

  digitalWrite(LEDPin, LOW);
  delay(30);
  digitalWrite(LEDPin, HIGH);
  
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
    Serial.println("Sent Last");

    if (battVolt[thisBatt] <= lowBatt || battVolt[thisBatt] < 3.0) {
      digitalWrite(mosfetPin, false);
      digitalWrite(dischargeLEDpin, false);
      dischargeState = false;
      Serial.println("Battery not Draining");
    }
    
// You have to manually change the code for each cell in the 'pack.' (This is cell 2 so we save it's voltage to adafruit io.)
    espbattery2->save(sensorValue);
//Enable this on one ESP12 for easy text readable battery voltages on adafruit io.    
//    debugtext->save(debugText);
//The two below are actually universal and don't have to be updated based on cell.
    battdischarge->save(dischargeState);
//    battdischargetime->save(dischargeTime);   // If you have the adafruit io bandwidth this is a nice way to keep track of each cells drain time.  The cell with the lowest time is the weakest.
    
    digitalWrite(LEDPin, LOW);
    delay(100); // just to be safe

// I originally used these to force a read of the adafruit io every tx loop. These are unnecessary since the onMessage code gives us the updates as soon as adafruit io gets them. 
// Note: You cannot use these if you want the battTime code to work as these force the battTime to update every time they're run. (And these just load whatever old data exists.)
//    espbattery->get();
//    espbattery2->get();
//    espbattery3->get();
//    espbattery4->get();

    digitalWrite(LEDPin, HIGH);

  }
    
  if (dischargeState) { dischargeTime++; }
  
  if ((millis() > 30000) && (battVolt[thisBatt] > (lowBatt + allowedCellDiff)) && (battVolt[thisBatt] > 3.0) && (staleData == 0)) {
    digitalWrite(mosfetPin, true);
    digitalWrite(dischargeLEDpin, true);
    dischargeState = true;
    Serial.println("Battery Draining");
  }
  
  delay(sampleTime-150);

}

void updateVoltage() {

  digitalWrite(mosfetPin, false);  // Turn off the mosfet/resistor load so we can read an accurate 'unloaded' voltage.
  
  int sensorInt = 0;
  for (i=0;i<10;i=i+1) {
    delay(15);
    sensorInt = sensorInt + analogRead(analogIn);
  }
  sensorInt = sensorInt / 10;
  sensorValue = (sensorInt * 5.44) / 1024;  //was 5.31
  sensorInt = (sensorValue * 100);     // Round to two decimal places...
  sensorValue = sensorInt;
  sensorValue = sensorValue / 100;
  
  battVolt[thisBatt] = sensorValue;
  
// ***You have to manually change the code for each cell in the 'pack.' (This is cell 2 so we're not monitoring it's staleness/time.)
  if ((battTime + 80000) < millis()){ staleData = (staleData | (0x1));  }
//  if ((battTime2 + 80000) < millis()){ staleData = (staleData | (0x2));  }
  if ((battTime3 + 80000) < millis()){ staleData = (staleData | (0x4));  }
  if ((battTime4 + 80000) < millis()){ staleData = (staleData | (0x8));  }

  Serial.println(staleData,BIN);
  if (staleData > 0) {
    dischargeState = false;
    digitalWrite(dischargeLEDpin, false);
    delay(300);
    digitalWrite(dischargeLEDpin, true);
  }
  digitalWrite(mosfetPin, dischargeState);
}

// ***You have to manually change the code for each cell in the 'pack.' (This is cell 2 so we're not monitoring it's time.) Commenting this out may(shoud?) be unnecessary.
void getBatt(AdafruitIO_Data *data) {
  battVolt[0] = data->toFloat();
  battTime = millis();
  staleData = (staleData & (0xF - 0x1));
}
//void getBatt2(AdafruitIO_Data *data) {
//  battVolt[1] = data->toFloat();
//  battTime2 = millis();
//  staleData = (staleData & (0xF - 0x2));
//}
void getBatt3(AdafruitIO_Data *data) {
  battVolt[2] = data->toFloat();
  battTime3 = millis();
  staleData = (staleData & (0xF - 0x4));
}
void getBatt4(AdafruitIO_Data *data) {
  battVolt[3] = data->toFloat();
  battTime4 = millis();
  staleData = (staleData & (0xF - 0x8));
}
