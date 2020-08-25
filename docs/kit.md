# LED Sectional Kits
On the [/r/flying subreddit](https://www.reddit.com/r/flying) there was a [great deal of interest](https://www.reddit.com/r/flying/comments/a4yb83/aviation_nerd_jackpot/ebiscca/) in a simple kit for the electronic components that could be used to create a [LED Sectional](index.md). Because of that, I've created a simple kit to help kickstart the process.

## Ordering

There are three different ordering links depending on how you'd like your kit configured.
1. [Buyer programs](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=CFKGLG5F2ETDW)
    * The kit will arrived programmed with a LED test program so you can ensure the strand is working properly. You'll need to program it for your map using the instructions located [here](index.md#software). You will need a high-quality Micro USB cable in order to program the board.
2. [Buyer programs, with an included ambient light sensor](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=67RASVCG8DK5A)
    * This includes an added ambient light sensor so the map can be programmed to automatically turn off when the ambient lights are off. I would recommend ONLY adding the light sensor if you plan to put the map in a room where the light will be disruptive at night as this will not significantly impact power usage.

**An important note:** I'm not trying to make a profit on these so I am selling these very close to cost. As you can imagine, there's a fair bit of effort involved in putting them together. If you like your kit once you receive it, I would appreciate it if you considered giving a tax deductible donation to the [Upwind Foundation](https://www.upwindscholarship.com/). We're having a bit of a contest to see who can raise the most, so please do mention my name (Kyle Harmon) if you donate.

## About the Kit
The kit includes the following:
* A custom PCB that includes the following:
    * An ESP8285 microcontroller and components and interfaces that replicate a [WEMOS D1 Mini Lite](https://docs.wemos.cc/en/latest/d1/d1_mini_lite.html) development board.
    * The following additional components included for connectivity and to comply with [Adafruit's Best Practices](https://learn.adafruit.com/adafruit-neopixel-uberguide/best-practices):
        * [330 Ohm Resistor](https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_UNI-ROYAL-Uniroyal-Elec-0603WAF3300T5E_C23138.html)
        * [1000 µF Capacitor](https://lcsc.com/product-detail/Aluminum-Electrolytic-Capacitors-SMD_Lelon-VEJ102M1ATR-1010_C134756.html)
        * [3-pin JST XH Connector](https://lcsc.com/product-detail/Wire-To-Board-Wire-To-Wire-Connector_BOOMELE-Boom-Precision-Elec-XH-3A_C2316.html) to connect LEDs
        * [5-pin JST XH Connector](https://lcsc.com/product-detail/Wire-To-Board-Wire-To-Wire-Connector_BOOMELE-Boom-Precision-Elec-XH-5A_C2318.html) (if shipped with a light sensor) connected to the i2c bus for a TSL2561 light sensor
        * 2x [TS-1187A-B-A-B tacticle switches](https://lcsc.com/product-detail/Tactile-Switches_XKB-Connectivity-TS-1187A-B-A-B_C318884.html), one connected to the reset switch and one ("PROGRAM") connected to GPIO12 (currently unused in the sketch)
        * [ME6211C33M5G-N LDO](https://lcsc.com/product-detail/Dropout-Regulators-LDO_MICRONE-Nanjing-Micro-One-Elec-ME6211C33M5G-N_C82942.html) to shift 3.3v logic to 5v logic for LEDs
        * [2.1mm x 5.5mm DC Barrel Jack](https://www.digikey.com/product-detail/en/cui-inc/PJ-037A/CP-037A-ND/1644545)
* Custom 3D printed case
* JST-XH to JST-SM cable for connecting LEDs.

Here's are some pictures of what you get:
![Kit board](https://wkharmon.github.io/led-sectional/images/Kit%20board.jpg)
![Kit covered](https://wkharmon.github.io/led-sectional/images/Kit%20covered.jpg)

For the electronics, you'll need to purchase the following items as well:
* [WS2811 LED Strands](https://amzn.to/2Sng8vF) (choose the 5V version)
* [5V Power Supply](https://amzn.to/2RnCvn8)

You'll also need to purchase sectionals, backing, glue, etc. to create and mount the map itself. The details about how I did that are documented [here](index.md).

## Using the Kit
Details on how to upload firmware and use the kit can be found [here](index.md#software).

## Limitations
There are a couple of limitations to note:
* This is still a DIY project. I’m trying to make it pretty polished, but it’ll be a little rough around the edges. The color of the case and the board may change.
* The kit only supports 2.4 GHz WiFi, and you should ensure that you have a solid signal wherever you plan to put the map.
* I’ve tested this to about 100 LEDs. It shouldn’t have much trouble handling any single sectional, but if you plan to use a much larger area let me know so I can test it ahead of time.

## Open Source
Everything about the kit is open source. The code lives [here](https://github.com/WKHarmon/led-sectional/blob/master/led-sectional.ino). If you'd like to make your own PCB, the Eagle files are [here](https://github.com/WKHarmon/led-sectional/tree/master/pcb%20files), and if you have a 3D Printer the STL files for the enclosure are [here](https://github.com/WKHarmon/led-sectional/tree/master/enclosure%20stl%20files).