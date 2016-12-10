# MyLetterBox
A IoT device based on ESP8266 to send an e-mail hen you receveid a physical mail

What is more boring than waiting for a parcel and check your's letter box ?

This program is designed to use an ESP8266 to monitoring a physical letter box !

There is no loop, because it have to be triggered by a switch you have
to connect to the Reset pin and activated by the lid.

ESP8266 will remain in deep sleep mode until the postman open the lid.

!!! IMPORTANT !!!
The power have to be provided by a Li-Ion or lipo single cell trough a diode
to reduce the voltage.
Common silicium diode have a 0.6V drop out.
With fully charged, Li-Ion cell (4.2V), the voltage will be 3.6 volts
The minimal voltage required for ESP8266 is 2.9 volts, it will remain
3.5V in the cell.
During communication with wireless active, the current is beetween 50mA and 100mA
In deep sleep mode, the current is lower than 200µA !


This code is designed to be used with Arduino IDE
Encode user and password for smtp2go.com in base 64 utf8 are done by base64.h library
In alternate, you can use https://www.base64encode.org/ and directly put those in the right places in code



Adapted from SurferTim's Ethernet email program by Ty Tower May 2015
See more at: http://www.esp8266.com/viewtopic.php?f=29&t=3349#sthash.ry0vdMKz.dpuf
Register at smtp2go.com --easy.



