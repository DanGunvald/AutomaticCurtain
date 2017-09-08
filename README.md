# AutomaticCurtain

Controls curtain and blinds using servo for blinds and stepper motor for curtain,

Ball puley can be found at : https://www.thingiverse.com/thing:1322

The wallmount/box (curtainmotoholdercontrollerbox.scad) has to be modified so the ballpulley + chain will fit.

Using esp8266 for wifi/mqtt gateway and arduino mini pro for controller. Stepper driver drv8825 is more silent then a4988.

Very basic drawing(automaticcurtain_bb.png) showing how I put it together. Missing all gnd and vcc. Vcc for stepper is 12v, vcc for arduino is 5v and vcc for esp is 3.3v. To control servo power I use a bd237, but that might not be the best choice. I out a rectifier between my 12v source and the stepper driver.

https://www.thingiverse.com/thing:2426076

Materials:<br />
Arduino mini pro  <br />
esp8266 12e (any of the smaller variants if it has to fit in the box)<br />
ams1117 3.3v<br />
5v switching regulator<br />
nema17<br />
sg90 servo <br />
drv8825/a4988 stepper driver<br />
Capcitator<br />
Rectifier(Dont know if this is realy needed)<br />
12v psu<br />
