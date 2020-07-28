# ESP-12E-BMS-Balancer
li-ion BMS / Balancer made of multiple NodeMCU ESP-12E (one per cell) using Adafruit io to store/track/trend each cell's voltage.

I used an LM3671 from adafruit to power the 3.3v in of the ESP-12E directly because my NodeMCU version uses an LDO regulator which drops half 
a volt or so. I wanted my ESP-12E to be able to continue monitoring the cell all the way down to 2.4 volts or so. The LDO wouldn't allow that.

I used an IRFZ44 mosfet (driven by D1 through a resistor) to connect a 16 ohm / 3W resistor to the batery input.  At a ~4VDC input this'd be 250mA and 1W worth of drain.  
The code leads to a battery drain (mosfet) duty cycle of about 80%, so technically about 800mW of drain.  While draining, the resistor can be touched momentarily but is 
hot enough to burn you.  It was not hot enough to melt my temporary breadboard or the wires/components nearby.

As mentioned in the code, A0 on the NodeMCU has a built in voltage divider of 100k:220k ohms.  This enables you to simply jumper from A0 to the battery in with a 220k 
resistor (for li-ion) which creates a 100k:440k voltage divider, which is a ratio of 1:5.4 

In case the search function checks the readme for codewords, here're some:
NodeMCU, ESP12, ESP-12, ESP12E, ESP-12E, ESP8266, Battery Management System, Balance, lithium, pack

I added the code from an ESP32 I was testing as well.  I coded it to monitor battery4 (and the code would have to be adjusted to match the battery it is connected to.)
This code was used before I added the staleData code you'll see in the ESP12 code.  The side effect of not having the staleData code is that if a battery is low and then
disconnected from the internet, this ESP will attempt to drain it's battery down to that last known voltage...ignoring the fact that the last known voltage could be very
very old.  This is handy for testing and/or forcing a single cell to a certain voltage but not ideal for regular use or as a reliable BMS. (Basically, I think the staleData
code is important, I just never implemented it in the ESP32 code.)
