# PCB Files
Originally this project used a WEMOS D1 Mini; however, to make assembly a bit easier the PCB now has all the components placed on the PCB directly, including an ESP8285 microcontroller. All of the SMD components are from the parts library that [jlcpcb.com](https://jlcpcb.com) can assemble directly for cheap. The files in this folder are:

* LED Sectional PCB - EasyEDA.json - PCB design file for import into EasyEDA.
* LED Sectional - Gerbers.zip - PCB production files that should be usable by any PCB manufacturer.
* LED Sectional - BOM.csv - All of the components used in the board design.
* LED Sectional - Centroid.csv - Pick and place file for assembly of the SMD components.

If assembling through jlcpcb.com, it's still necessary to solder all of the through hole components like the micro USB, power, and LED connectors. Other assembly fabs can handle the thru-hole components, but most charge significantly more for the via-in-pads that are used in the design.