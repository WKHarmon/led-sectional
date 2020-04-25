# LED Sectional Kits
On the [/r/flying subreddit](https://www.reddit.com/r/flying) there was a [great deal of interest](https://www.reddit.com/r/flying/comments/a4yb83/aviation_nerd_jackpot/ebiscca/) in a simple kit for the electronic components that could be used to create a [LED Sectional](index.md). Because of that, I've created a simple kit to help kickstart the process.

## Ordering
**IMPORTANT: I'm currently out of stock** on the modules that power the kits. I have a shipment on the way from China, but it may take a while to get here. If you place an order now, I'll ship the kit when it becomes available, but it may be a few weeks.
There are three different ordering links depending on how you'd like your kit configured.
1. [Buyer programs](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=CFKGLG5F2ETDW)
    * The kit will arrived programmed with a LED test program so you can ensure the strand is working properly. You'll need to program it for your map using the instructions located [here](index.md#software). You will need a high-quality Micro USB cable in order to program the board.
2. [Pre-programmed](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=783A474TT9K4G)
    * You'll send me your WiFi username and password, as well as the order in which the airports appear on your map. I'll pre-program the module so all you need to do when you receive it is plug it in. NOTE: If your WiFi username or password changes, you'll need to re-program the module yourself. I'll follow up with you after you order with more information about how all of that will work.
3. [Buyer programs, with an included ambient light sensor](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=67RASVCG8DK5A)
    * This includes an added ambient light sensor so the map can be programmed to automatically turn off when the ambient lights are off. I would recommend ONLY adding the light sensor if you plan to put the map in a room where the light will be disruptive at night as this will not significantly impact power usage.

**An important note:** I'm not trying to make a profit on these so I am selling these very close to cost. As you can imagine, there's a fair bit of effort involved in putting them together. If you like your kit once you receive it, I would appreciate it if you considered giving a tax deductible donation to the [Upwind Foundation](https://www.upwindscholarship.com/). We're having a bit of a contest to see who can raise the most, so please do mention my name (Kyle Harmon) if you donate.

## About the Kit
The kit includes the following:
* Wemos D1 Mini micro-controller
* Custom PCB with the following additional components included for connectivity and to comply with [Adafruit's Best Practices](https://learn.adafruit.com/adafruit-neopixel-uberguide/best-practices):
    * [330 Ohm Resistor](https://www.digikey.com/product-detail/en/yageo/CFR-25JB-52-330R/330QBK-ND/1636)
    * [1000 µF Capacitor](https://www.digikey.com/product-detail/en/nichicon/RL80J102MDN1KX/493-4024-1-ND/2347912)
    * [3-pin JST XH Connector](https://www.digikey.com/product-detail/en/jst-sales-america-inc/B3B-XH-A-LF-SN/455-2248-ND/1651046)
    * [4-pin JST XH Connector](https://www.digikey.com/product-detail/en/jst-sales-america-inc/B4B-XH-A-LF-SN/455-2249-ND/1651047)
    * [74HCT245N Level Shifter](https://www.digikey.com/product-detail/en/texas-instruments/SN74HCT245N/296-1612-5-ND/277258)
    * [2.1mm x 5.5mm DC Barrel Jack](https://www.digikey.com/product-detail/en/cui-inc/PJ-037A/CP-037A-ND/1644545)
* Custom 3D printed case
* JST-XH to JST-SM cable for connecting LEDs.

Here's are some pictures of what you get:
![Kit board](https://wkharmon.github.io/led-sectional/images/Kit board.jpg)
![Kit covered](https://wkharmon.github.io/led-sectional/images/Kit covered.jpg)

For the electronics, you'll need to purchase the following items as well:
* [WS2811 LED Strands](https://amzn.to/2Sng8vF) (choose the 5V version)
* [5V Power Supply](https://amzn.to/2RnCvn8)

You'll also need to purchase sectionals, backing, glue, etc. to create and mount the map itself. The details about how I did that are documented [here](index.md).

## Using the Kit
Details on how to upload firmware and use the kit can be found [here](index.md#software).

## Limitations
There are a couple of limitations to note:
* This is still a DIY project. I’m trying to make it pretty polished, but it’ll be a little rough around the edges. The color of the case and the board may change.
* The Wemos modules only support 2.4 GHz WiFi, and you should ensure that you have a solid signal wherever you plan to put the map.
* I’ve tested this to about 100 LEDs. It shouldn’t have much trouble handling any single sectional, but if you plan to use a much larger area let me know so I can test it ahead of time.

## Open Source
Everything about the kit is open source. The code lives [here](https://github.com/WKHarmon/led-sectional/blob/master/led-sectional.ino). If you'd like to make your own PCB, the Eagle files are [here](https://github.com/WKHarmon/led-sectional/tree/master/pcb%20files), and if you have a 3D Printer the STL files for the enclosure are [here](https://github.com/WKHarmon/led-sectional/tree/master/enclosure%20stl%20files).