# LED Sectional Kits
On the [/r/flying subreddit](https://www.reddit.com/r/flying) there was a [great deal of interest](https://www.reddit.com/r/flying/comments/a4yb83/aviation_nerd_jackpot/ebiscca/) in a simple kit for the electronic components that could be used to create a [LED Sectional](index.md). Because of that, I've created a simple kit to help kickstart the process.

## Ordering
I'll be setting up a PayPal order link here shortly.

## About the Kit
The kit includes the following:
* Wemos D1 Mini micro-controller
* Custom PCB with the following additional components included for connectivity and to comply with [Adafruit's Best Practices](https://learn.adafruit.com/adafruit-neopixel-uberguide/best-practices) :
    * [330 Ohm Resistor](https://www.digikey.com/product-detail/en/yageo/CFR-25JB-52-330R/330QBK-ND/1636)
    * [1000 µF Capacitor](https://www.digikey.com/product-detail/en/rubycon/25PX1000MEFCT810X16/1189-1583-3-ND/3134863)
    * 2x [JST XH Connectors](https://www.digikey.com/product-detail/en/jst-sales-america-inc/B3B-XH-A-LF-SN/455-2248-ND/1651046)
    * [74HCT245N Level Shifter](https://www.aliexpress.com/item/5PCS-SN74HCT245N-DIP20-SN74HCT245-DIP-74HCT245-74HCT245N-new-and-original-IC-free-shipping/32537892034.html?spm=a2g0s.9042311.0.0.65834c4dVZ9zG9)
    * [1N4004 Diode](https://www.digikey.com/product-detail/en/micro-commercial-co/1N4004-TP/1N4004-TPMSTR-ND/773641)
    * [2.1mm x 5.5mm DC Barrel Jack for power](https://www.digikey.com/product-detail/en/cui-inc/PJ-037A/CP-037A-ND/1644545)
* Custom 3D printed case
* JST-XH to JST-SM cable for connecting LEDs.

Here's a picture of what you get:
TODO: Picture goes here

For the electronics, you'll need to purchase the following items as well:
* [WS2811 LED Strands](https://amzn.to/2Sng8vF) (choose the 5V version)
* [5V Power Supply](https://amzn.to/2RnCvn8)

You'll also need to purchase sectionals, backing, glue, etc. to create and mount the map itself. The details about how I did that are documented [here](index.md).

## Using the Kit
Details on how to upload firmware and use the kit can be found [here](index.md#software).

## Open Source
Everything about the kit is open source. The code lives [here](https://github.com/WKHarmon/led-sectional/blob/master/led-sectional.ino). If you'd like to make your own PCB, the Eagle files are [here](https://github.com/WKHarmon/led-sectional/tree/master/pcb%20files), and if you have a 3D Printer the STL files for the enclosure are [here](https://github.com/WKHarmon/led-sectional/tree/master/enclosure%20stl%20files).