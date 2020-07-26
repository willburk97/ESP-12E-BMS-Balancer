# ESP-12E-BMS-Balancer
li-ion BMS / Balancer made of multiple NodeMCU ESP-12E (one per cell) using Adafruit io to store/track/trend each cell's voltage.

I used an LM3671 from adafruit to power the 3.3v in of the ESP-12E directly because my NodeMCU version uses an LDO regulator which drops half 
a volt or so. I wanted my ESP-12E to be able to continue monitoring the cell all the way down to 2.4 volts or so. The LDO wouldn't allow that.

I used an IRFZ44 mosfet (driven by D1 through a resistor) to connect a 16 ohm / 3W resistor to the batery input.  At a ~4VDC input this'd be 250mA and 1W worth of drain.  The code 
leads to a battery drain (mosfet) duty cycle of about 80%, so technically about 800mW of drain.  While draining, the resistor can be touched momentarily but is hot enough
to burn you.  It was not hot enough to melt my temporary breadboard or the wires/components nearby.

As mentioned in the code, A0 on the NodeMCU has a built in voltage divider of 100k:220k ohms.  This enables you to simply jumper from A0 to the battery in with a 220k 
resistor (for li-ion) which creates a 100k:440k voltage divider, which is a ratio of 1:5.4 
