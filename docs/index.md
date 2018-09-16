LED Sectional
=============

This project uses an ESP8266-based microcontroller to download METARs from a set of airprots and assign a color to a LED representing the current weather at each airport. The LEDs are placed in an aviation sectional and that looks great on a wall.

This project is adapted from a few other very similar ones:
* http://blog.dylanhrush.com/2017/07/live-sectional-map.html
* https://www.reddit.com/r/flying/comments/7avr8q/flight_conditions_sectional_wall_art_thing/
* https://pplaviationuk.wordpress.com/creating-a-metar-map/

There were small gaps in the how-to details for some of them so I figured it might be helpful to fully document how I did it. I also opted to use a microcontroller rather than a Raspberry Pi, so mine is different in that way too.

# Pictures
Here are a few pictures of the final product:
![Final Product](https://wkharmon.github.io/led-sectional/images/Final 1.jpg)
![Final Product](https://wkharmon.github.io/led-sectional/images/Final 2.jpg)
![Final Product](https://wkharmon.github.io/led-sectional/images/Final 3.jpg)

# Things you'll need
The materials themselves are pretty cheap--you should be able to get everything you need aside from the frame for less than $150. The frame cost will vary depending on how big you want to make the sectional. The tools may add some cost if you don't already have what you need.

## Materials
* [WEMOS D1 Mini Lite](https://www.aliexpress.com/store/product/WEMOS-D1-mini-Lite-V1-0-0-WIFI-Internet-of-Things-development-board-based-ESP8285-1MB/1331105_32795857574.html?spm=2114.12010615.8148356.30.10027c8csIW2n0)
    * If you have time to wait for the shipment, support them and buy from the official store, but [this version on Amazon](https://www.amazon.com/gp/product/B07BK435ZW/ref=as_li_tl?ie=UTF8&tag=wkharmon-20&camp=1789&creative=9325&linkCode=as2&creativeASIN=B07BK435ZW&linkId=895aa52a462e2ac4b0134cc4385e1402) worked fine.
    * The non-Lite version will work fine, and this code should be pretty adaptable to any Arduino-compatible board with WiFi.
* 3.3V to 5V Level Shifter
    * [74HCT245](https://www.aliexpress.com/item/10PCS-SN74HCT245N-DIP20-SN74HCT245-DIP-74HCT245-74HCT245N-DIP-20-new-and-original-IC/32537892034.html?spm=a2g0s.9042311.0.0.58d84c4d3QyyCc) are pretty well-regarded
    * [Amazon Alternative](https://www.amazon.com/gp/product/B07CWKY2CB/ref=as_li_tl?ie=UTF8&tag=wkharmon-20&camp=1789&creative=9325&linkCode=as2&creativeASIN=B07CWKY2CB&linkId=f71fa879c7e9dea55436cb533517a00a) - These also worked fine, but they may be too slow if you run a ton of LEDs.
* [WS2811 LED Strand(s)](https://amzn.to/2QxYzbD)
    * I'm also trying out [these](https://www.aliexpress.com/item/Mokungit-50pcs-12MM-WS2811-Full-Color-Square-Pixel-LED-Module-Lighting-String-5V-12V-RGB-Waterproof/32803645847.html?spm=a2g0s.9042311.0.0.41a84c4d6VpNsN) and [these](https://www.aliexpress.com/item/DC5V-12mm-SM16703-RGB-LED-Pixel-module-IP68-waterproof-full-color-50pcs-a-string-perforator-alphabet/32817144776.html?spm=a2g0s.9042311.0.0.41a84c4d6VpNsN) for a lower profile.
* [5V DC Power Supply](https://www.amazon.com/gp/product/B078RT3ZPS/ref=as_li_tl?ie=UTF8&tag=wkharmon-20&camp=1789&creative=9325&linkCode=as2&creativeASIN=B078RT3ZPS&linkId=5997f784bd7a0995cc69fed33f39129a) - This is more power than you really need--you could get a smaller one if you'd prefer.
* [DC Barrel Connectors](https://www.amazon.com/gp/product/B06XNR1ZVL/ref=as_li_tl?ie=UTF8&tag=wkharmon-20&camp=1789&creative=9325&linkCode=as2&creativeASIN=B06XNR1ZVL&linkId=7abd8ecd6cebd232fbd0de9b00091e0e)
* [Solid Core Hookup Wire](https://www.amazon.com/gp/product/B00B4ZRPEY/ref=as_li_tl?ie=UTF8&tag=wkharmon-20&camp=1789&creative=9325&linkCode=as2&creativeASIN=B00B4ZRPEY&linkId=60886b0eec561174abea5c2e56e857f3) - 22G worked well for me
* [3cm x 7cm PCB](https://www.amazon.com/gp/product/B07CK3RCKS/ref=as_li_tl?ie=UTF8&tag=wkharmon-20&camp=1789&creative=9325&linkCode=as2&creativeASIN=B07CK3RCKS&linkId=6082d47cb27919fdff01fd3675a5d203) - This has a bunch of PCBs, I'm sure there's a cheaper opton. Other sizes will work, but these fit nicely into a small project enclosure.
* Optional: [80mm x 50mm x 20mm Project Enclosure](https://www.amazon.com/gp/product/B071WRYXHT/ref=as_li_tl?ie=UTF8&tag=wkharmon-20&camp=1789&creative=9325&linkCode=as2&creativeASIN=B071WRYXHT&linkId=c31a81d8e941299dbfce5364d464440e)
* Optional: 3-pin [JST-XH Connectors](https://amzn.to/2QANmqO) and [JST-SM Connectors](https://amzn.to/2xiQ3o2) if you want a interface between the microcontroller and the LED strand(s)
* Optional: [Velcro](https://amzn.to/2MAEjTu) to affix the power supply and controller to the board
* Optional: 1000 µF, 6.3V or higher capacitor
* Optional: 220 Ohm resistor

You'll also need either:
* Sectional(s) from the FAA - remember that they're two sided so you may need two to get full coverage
* [Hard Tempered Board](https://www.homedepot.com/p/Hardboard-Tempered-Common-1-8-in-x-2-ft-x-4-ft-Actual-0-125-in-x-23-75-in-x-47-75-in-7005015/202088786) or MDF
    * The tempered board has a really nice surface for glueing, but it doesn't drill cleanly. Some others use MDF which might be cleaner.
* [3M Super 77 Spray Adhesive](https://www.homedepot.com/p/3M-16-7-oz-Super-77-Multi-Purpose-Spray-Adhesive-77-24VOC30/301619194)
* [SP8 Picture Frame](https://www.pictureframes.com/Custom-Frames/Canvas-Ready-Wood-Picture-Frames/SP8-black-wood-canvas-frame)
    * This is one of the only super-deep frames I've seen that are easily orderable online.
* Optional: [PVA Adhesive](https://amzn.to/2OtIGBo) if you have a seam between sectionals

OR

* Have a [digital file](https://www.faa.gov/air_traffic/flight_info/aeronav/digital_products/vfr/) of the sectional printed and mounted on foamcore at [PictureFrames.com](http://www.pictureframes.com). If you do this, be sure to maintain the 300 ppi from the original file in order to keep the sectional to scale (assuming you want that at least).
    * Use the [SP8 Picture Frame](https://www.pictureframes.com/Custom-Frames/Canvas-Ready-Wood-Picture-Frames/SP8-black-wood-canvas-frame) for plenty of depth.

## Tools
Some of the tools I have are linked, but any one will work.
* [Soldering iron](https://amzn.to/2QALNsW) and solder
* [Wire stripper](https://amzn.to/2xlO9TD)
* Drill with a 8mm drill bit
* [Glue gun](https://amzn.to/2Qz1p03) with plenty of [glue](https://amzn.to/2pcJWOB)
* Optional: [Crimper for JST connectors](https://amzn.to/2QFYEds)
