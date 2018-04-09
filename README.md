**NOTE**: *I am not the original author of this software;* this software was
originally created by Jeremy Burton, http://jedi98.uk/antenna_analyzer.php, modified to work with the SARK-110. 

SARK-110 Antenna Analyzer GUI
=======================================

![](http://www.sark110.com/_/rsrc/1522782273174/files/qt-antennaanalyzer/QT-AA-Screenshot.png)

Build Instructions
===================

Prerequisites:
--------------

* QT Creator 

Build
------
Use QT creator to build the software from Linux or Windows.


Installation Instructions
=========================
### Linux
Run the installer program located in installer/linux folder.

**NOTE**: the installer creates the file 99-sark110.rules at /etc/udev/rules.d to change the permissions of hidraw* device driver and allow connecting to the SARK-110. If this does not work, run the program with administrative rights (e.g. from root or using sudo command).

### Windows
Run the installer program located in installer/win32 folder.
