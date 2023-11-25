
# A GPS based Sun Tracker - Heliostat


Basically there are two ways to follow the sun, one is searching the sky and the other is calculating
where it supossed to be.

This system calculate the sun position thanks to NREL SPA
National Renewable Energy Laboratory - Solar Position Algorithm.

The algorithm need to be feed with many parameter taken from a GPS.
The GPS give UTC time, longitude, latitude and elevation.
GPS is the most accurate source of time available.
Only the timezone need to be set by us.

The mechanical configuration of it, is the basic orthogonal axis azimut-elevation.

The system can be used like a sun flower or like a heliostat where a target is configured
and then the sun ray always will be aimed to the target.


## Electronic

**Be care that the SPA algorithm only can run in 32 bits processors.**

The system is divided in two parts one is ESP-32 and the other Arduino MEGA 2560 with Ramps 1.4 Shield

Both are intercommunicated by I2C, the ESP-32 run as master and the Ramps as slave.

I have planned to use NEMA 23 motors for that reason I use two MKS TB 6600 drivers to control two stepper motors and 
also have two optical endstop for zero position.

But for smaller motor like Nema 17, the DRV8825 can be used with any problem. 
Take in count that the motors moves a few steps every 30 seconds.

The GPS can be connected to any of them, just need to configure where.

The ESP-32 provide the Tracker, SPA calculation, WIFI and Bluetooth, and also a web page can be implemented .

The human machine interface can be done by Serial port, Telnet or Bluetooth. 
For Bluetooth the recommended phone app is *Serial Bluetooth Terminal by Kai Morich*

https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal&hl=es_EC

Using a menu of options one choose the action to perform.

Is pretty sure that the  partition table must be changed.

In the ESP-32 the  partition table must be change on order to earn space for all: 
https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/tutorials/partition_table.html

I use: *4MB no OTA*


# Sun alignment:

In order to align the system with the sun, build a pin hole box,

1. Set in mode Tracker
2. set the Azimuth to zero --> go end Azimuth
3. set the Elevation to zero --> go end Elevation
4. set START

Then the motors will move, and then with your hand move-align the base of the whole system until the sun pass through the pin hole.

# Setting the Shift from zero:

The azimuth alignment is zero for East,(I live in South hemisphere) and the Elevation alignment is zero for Horizon. 
If the mechanical position of the endstop is diferent you can apply positive or negative shift.
It is a try an error proccess.























