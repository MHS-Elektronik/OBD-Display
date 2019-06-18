# OBD-II Display for Rasperry PI/Linux and Tiny-CAN

[![OBD-Display](https://github.com/MHS-Elektronik/OBD-Display/blob/master/doku/obd_display.jpg)](https://github.com/MHS-Elektronik/OBD-Display/blob/master/doku/obd_Display.jpg)

# [This text in German](https://github.com/MHS-Elektronik/OBD-Display/blob/master/README_DE.md "Diesen Text auf Deutsch anzeigen")

# Funcional scope

* Query of measurement data (SID 0x01)
  * Configurable display of the PID values from 0x00 to 0x4E
* Query of vehicle information (SID 0x09)
  * Breakdown of the Vehicle Identification Number (VIN) by manufacturer, country, model year, serial number, ...)
* Read the error codes (DTC Diagnostic Trouble Codes) (SID 0x03)
  * Plain text Display of error codes via databank
* Display CAN raw data (CAN-Trace)
*Automatic recognition of CAN hardware and baud rate

# Screenshots

[![OBD-Display](https://github.com/MHS-Elektronik/OBD-Display/blob/master/doku/obd_start.png)](https://github.com/MHS-Elektronik/OBD-Display/blob/master/doku/obd_start.png)

[![OBD-Display](https://github.com/MHS-Elektronik/OBD-Display/blob/master/doku/obd_pid_select.jpg)](https://github.com/MHS-Elektronik/OBD-Display/blob/master/doku/obd_pid_select.jpg)

[![OBD-Display](https://github.com/MHS-Elektronik/OBD-Display/blob/master/doku/obd_list_view.jpg)](https://github.com/MHS-Elektronik/OBD-Display/blob/master/doku/obd_list_view.jpg)

[![OBD-Display](https://github.com/MHS-Elektronik/OBD-Display/blob/master/doku/obd_error_codes.jpg)](https://github.com/MHS-Elektronik/OBD-Display/blob/master/doku/obd_error_codes.jpg)


# Hardware
* Linux PC i.e. Rasberry PI
* Tiny-CAN i.e. Tiny-CAN I-XL, Source of supply: http://www.mhs-elektronik.de
* OBD-CAN cable

[![OBD-Display](https://github.com/MHS-Elektronik/OBD-Display/blob/master/doku/obd2_cable.jpg)](https://github.com/MHS-Elektronik/OBD-Display/blob/master/doku/obd2_cable.jpg)

# Installation

First the Tiny-CAN software package must be installed. For the operation of the software only the "libmhstcan.so" is required. "OBD-Display" searches for this file in the directory "/opt/tiny_can/can_api". 

Download the file "tiny_can_raspberry_XXX.tar.gz" from "http://www.mhs-elektronik.de" and unpack it in the directory "/opt". Replace XXX with the latest version. The archive can be deleted after it has been unpacked. The access rights for the "/opt" directory must first be set, here are the individual steps:

    $ sudo chgrp pi /opt
    $ sudo chmod -R 775 /opt
    $ cd /opt
    $ mv /home/pi/tiny_can_raspberry_XXX.tar.gz .
    $ tar -xzvf  tiny_can_raspberry_XXX.tar.gz
    $ rm tiny_can_rasberry_XXX.tar.gz

Compile the Tiny-CAN API:
 
    $ cd /opt/tiny_can/can_api/src/mhstcan/linux
    $ make
    $ mv libmhstcan.so ../../..
    
The Lib is already included in the package, usually compiling is not necessary.

Install "git":

    $ sudo apt-get install git

Get "ObdDisplay" from "github":

    $ cd /opt
    $ git clone https://github.com/MHS-Elektronik/OBD-Display.git

Install development packages:

    $ sudo apt-get install gtk2.0-dev

Compile "ObdDisplay":

    $ cd /opt/OBD-Display/linux
    $ make

Start "ObdDisplay":

    $ cd /opt/OBD-Display/linux/bin
    $ ./ObdDisplay


# Sources/Links
Two of the core files, "obd_db.c" and "obd_decode.c" in this project are based on the OBD-II API from Ethan Vaughan, https://github.com/ejvaughan/obdii

With the ISO-TP driver I copied a bit from the ISP-TP Linux Kernel driver from Oliver Hartkopp, https://github.com/hartkopp/can-isotp

Most of the information I have taken from this document from emotive:
http://www.emotive.de/documents/WebcastsProtected/Transport-Diagnoseprotokolle.pdf
Really very worth reading, I can only recommend!

Other sources I have used:  
https://github.com/iotlabsltd/pyvin/tree/master/pyvin  
ftp://ftp.nhtsa.dot.gov/manufacture  
https://de.wikipedia.org/wiki/Fahrzeug-Identifizierungsnummer  
https://en.wikipedia.org/wiki/On-board_diagnostics  
https://de.wikipedia.org/wiki/ISO_15765-2  

 
# Image Sources
## Background graphic, VW Beetle:
Source: https://commons.wikimedia.org/wiki/File:Der_Samtrote_Sonderkäfer.jpg
Author: Marco Strohmeier
Licence: Public Domain

## OBD-connector:
Source: https://commons.wikimedia.org/wiki/File:OBD2-Buchse-Stecker-Belegung.jpg
Author: https://de.wikipedia.org/wiki/Benutzer:Losch
Licence: CC BY-SA 4.0  https://creativecommons.org/licenses/by-sa/4.0

The image files were adapted for my purposes.

# Tips and Tricks
## Start the program automatically
If no mouse or keyboard is connected to the PI, it is useful to start the program automatically. To do this, copy the file „ObdDisplay.desktop“ from the directory „tools“ to the directory „/etc/xdg/autostart“.

    $ sudo cp /opt/ObdDisplay/tools/ObdDisplay.desktop /etc/xdg/autostart

## Turn off the screensaver
Open the file "/etc/lightdm/lightdm.conf" in the editor as user "root".

    $ sudo leafpad /etc/lightdm/lightdm.conf
    
In the section "SeatDefaults" modify the line "xserver-command" as follows or add if not exist.

    [SeatDefaults]
    ....
    xserver-command=X -s 0 -dpms
    ....

## Rotate the screen display
Open the file "/boot/config.txt" in the editor as user "root".

    $ sudo leafpad /boot/config.txt
    
Enter the following line into the file

    lcd_rotate=2
    
After reboot the image should be rotated 180° and you can turn the display upside down so that the microUSB connector is at the top. 

## Make the mouse pointer disappear
For our special application, the mouse pointer is annoying. To remove the cursor very easily, we can install a package that hides it:

    $ sudo apt-get install unclutter
    
After a reboot the cursor is invisible.


# ToDos
* Testing :-)
* Web connection, suggest OpenXC, https://github.com/openxc
* OBD-API: Customisation for SocketCAN, develop a SocketCAN Tiny-CAN API driver
* OBD-API: Expand database („obd_db.c/obd_decode“), PIDs
* OBD-API: Different polling intervals for the individual PIDs
* OBD-API: Only query PIDs that are also shown
* OBD-API: Documentation
* GUI: Porting to GTK3













