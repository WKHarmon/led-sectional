LED Sectional
=============
    * [Pictures](#pictures)
    * [Things you'll need](#things-youll-need)
        * [Materials](#materials)
        * [Tools](#tools)
    * [Software](#software)
    * [Assembly](#assembly)
        * [Soldering and Wiring](#soldering-and-wiring)
        * [Mounting the sectionals](#mounting-the-sectionals)
        * [Drilling](#drilling)
        * [Glueing LEDs](#glueing-leds)

This project is adapted from a few other very similar ones:
    * [http://blog.dylanhrush.com/2017/07/live-sectional-map.html](http://blog.dylanhrush.com/2017/07/live-sectional-map.html)
    * [https://www.reddit.com/r/flying/comments/7avr8q/flight_conditions_sectional_wall_art_thing/](https://www.reddit.com/r/flying/comments/7avr8q/flight_conditions_sectional_wall_art_thing/)
    * [https://pplaviationuk.wordpress.com/creating-a-metar-map/](https://pplaviationuk.wordpress.com/creating-a-metar-map/)

There were small gaps in the how-to details for some of them so I figured it might be helpful to fully document how I did it. I also opted to use a microcontroller rather than a Raspberry Pi, so mine is different in that way too.

# Pictures
Here are a few pictures of the final product:
![Final Product](https://wkharmon.github.io/led-sectional/images/Final 1.jpg)
![Final Product](https://wkharmon.github.io/led-sectional/images/Final 2.jpg)
![Final Product](https://wkharmon.github.io/led-sectional/images/Final 3.jpg)

# Things you'll need
The materials themselves are pretty cheap--you should be able to get everything you need aside from the frame for less than $150. The frame cost will vary depending on how big you want to make the sectional. The tools may add some cost if you don't already have what you need.

## Materials
I've linked to Amazon for most of the electronic items. You should be able to buy them for cheaper and in smaller quantites from Aliexpress or another similar store if you're willing to wait for the shipment. (In the interest of full disclosure, the Amazon links do have my affiliate tags, but I'm not really trying to make any money here.)
    * [WEMOS D1 Mini Lite](https://www.aliexpress.com/store/product/WEMOS-D1-mini-Lite-V1-0-0-WIFI-Internet-of-Things-development-board-based-ESP8285-1MB/1331105_32795857574.html?spm=2114.12010615.8148356.30.10027c8csIW2n0)
        * If you have time to wait for the shipment, support them and buy from the official store, but [this version on Amazon](https://www.amazon.com/gp/product/B07BK435ZW/ref=as_li_tl?ie=UTF8&tag=wkharmon-20&camp=1789&creative=9325&linkCode=as2&creativeASIN=B07BK435ZW&linkId=895aa52a462e2ac4b0134cc4385e1402) also worked fine.
        * The non-Lite version will work fine, and this code should be pretty adaptable to any Arduino-compatible board with WiFi.
    * 3.3V to 5V Level Shifter
        * [WeiMeet 4 Channel Level Converter](https://www.amazon.com/gp/product/B07CWKY2CB/ref=as_li_tl?ie=UTF8&tag=wkharmon-20&camp=1789&creative=9325&linkCode=as2&creativeASIN=B07CWKY2CB&linkId=f71fa879c7e9dea55436cb533517a00a) - These are what I used (and will document here), but they're considered to be a bit slow. You may run into problems if you have a ton of LEDs.
        * [74HCT245](https://www.aliexpress.com/item/10PCS-SN74HCT245N-DIP20-SN74HCT245-DIP-74HCT245-74HCT245N-DIP-20-new-and-original-IC/32537892034.html?spm=a2g0s.9042311.0.0.58d84c4d3QyyCc) are faster if you would prefer to use them instead.
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

**OR**

    * Have a [digital file](https://www.faa.gov/air_traffic/flight_info/aeronav/digital_products/vfr/) of the sectional printed and mounted on foamcore at [PictureFrames.com](http://www.pictureframes.com). If you do this, be sure to maintain the 300 ppi from the original file in order to keep the sectional to scale (assuming you want that at least).
        * Use the [SP8 Picture Frame](https://www.pictureframes.com/Custom-Frames/Canvas-Ready-Wood-Picture-Frames/SP8-black-wood-canvas-frame) for plenty of depth.

## Tools
Some of the tools I have are linked, but any one will work.
    * [Soldering iron](https://amzn.to/2QALNsW) and solder
    * [Wire stripper](https://amzn.to/2xlO9TD)
    * Drill with a 8mm drill bit
    * [Glue gun](https://amzn.to/2Qz1p03) with plenty of [glue](https://amzn.to/2pcJWOB)
    * Optional: [Crimper for JST connectors](https://amzn.to/2QFYEds)

# Software
The code I used for this project is available at https://github.com/WKHarmon/led-sectional. It is designed to use the Arduino IDE and should be relatively easy to adapt for your own purposes. I should mention that I'm not an engineer--I just hack together code to try to make it work, so I'm sure the code is ugly.

The program uses standard definitions for VFR, MVFR, IFR, and LIFR, but I also set it to show yellow if an airport is VFR but winds are greater than 25 nots, and an airport will strobe white every 5 seconds if there are thunderstorms in the area. These can be disabled by setting `DO_WINDS` and `DO_LIGHTNING` to false respectively.

On power on, all LEDs will show orange initially. Once the controller is connected to the WiFi network, they'll turn purple, and then they'll show the airport colors once the data is downloaded in parsed. LEDs for airports that have not reported weather in the last 3 hours, or that don't have a clear flight category will be turned off.

You'll need to install the following in order to program the board:
    * [ESP8266 Core](https://github.com/esp8266/Arduino) is necessary to use the ESP8266-compatible controller in the Arduino IDE.
    * [CH340G Driver](https://wiki.wemos.cc/downloads) is necessary for your computer to see the board
    * Within the Library Manager in the Arduino IDE, you'll also need to install the [FastLED Library])(http://fastled.io/).

Once you have all of that installed, you'll need to set the Board to `LOLIN(WEMOS) D1 mini Lite` under Tools in the Arduino IDE.

The `airports` vector contains all of the airports used for the project in the order in which they appear in the LED strand. `NULL` is used when we have unused LEDs in the strand. You'll also need to edit in the SSID and Password for your WiFi network.

# Assembly
## Soldering and Wiring
The Wemos D1 Mini Lite can accept 5V as input voltage, but the GPIO pins run at 3.3v. The WS2811 LED strand is expecting 5V on the data line, so it is necessary to convert the GPIO 3.3v to the WS2811 5V using a logic level shifter. (In some cases it may work without the shifter, but using one is the safer bet.) Below is a breadboard diagram for wiring it all up:

![Breadboard Diagram](https://wkharmon.github.io/led-sectional/images/Wemos PCB 1_bb.png)

I've diagramed on a breadboard for simplicity, but I used a 3cm x 7cm PCB. In the diagram, the (optional) JST-XH connector on the right goes to the WS2811 LED strand. If you don't want to bother with the connector, you can just solder the wires from the LED strand to the PCB.

![Controller on PCB](https://wkharmon.github.io/led-sectional/images/Microcontroller on PCB.jpg)

The data flow on the WS2811 strand is directional. In the LED pixel itself, the PCB will be marked with an arrow on one side. That arrow indicates the input side of the pixel. Connect the microcontroller to the side of the strand such that the wires from the controller are connected to the input side of the pixel (with the arrow), matching the 5V, Data, and Ground lines on the strand to those on the controller.

The WS2811 LED strand has pigtails for injecting power on the ends of the strand as well. Solder a DC barrel connector for power to the pigtails nearest the microcontroller. The microcontroller will receive power from its conenction to the LED strand.

Once I had it all connected up, I created a small pigtail with a JST-XH connector on one side and a JST-SM connector on the other side (which matches the connectors that came with the LED strand) to make it easy to attach and detach the LED strand, then I put it in an enclosure box.

![Enclosure](https://wkharmon.github.io/led-sectional/images/Enclosure.jpg)

## Mounting the sectionals
This section is only necessary if you bought actual sectionals and didn't take the shortcut and have a sectional printed and pre-mounted.

When I bought the Hard Tempered Board noted above, I had Home Depot cut it to the desired size. The combined halves of the San Francisco sectional were just over 36" high, and the board itself was 24" wide, so I just decided to use that width.

I cut off the bottom white space of the northern sectional, then I taped the sectionals at the seam using scotch tape, and measured which 24" of the sectional width I actually wanted. I then put the board on top of the sectional and used an Exacto knife to cut off the excess parts of the sectional, but leaving about 1" on each side of the sectional just in case I didn't get it perfectly centered when glueing.

Next, I sprayed the shiny side of the board with the 3M Spray Adhesive, waited about a minute per the directions, and then laid the sectional flat on the board so it would adhere. I used a ruler to flatten out any spots that were not flat.

Once the glue was dried, I cut off the excess parts of the sectional using an exacto knife, and then glued the flap created by the seam between the two sectionals using PVA Adhesive.

## Drilling
**Important: Don't use ForeFlight to determine where METARs are available.** ForeFlight aggregates several sources of weather data together, and contains several airports that are not available via the aviationweather.gov API. I suggest using the METAR map at aviationweather.gov to determine where to drill.

Once I figured out where I wanted to drill, I used an 8mm drill bit to drill through the sectional and the backing board together. This resulted in some holes that weren't super clean.

![Rough Edges](https://wkharmon.github.io/led-sectional/images/Rough edges.jpg)

I specifically wanted my LEDs to stick out, but if you didn't then adjusting the drill bit size and using a countersink bit would allow you to align them with the front of the board.

After I drilled, I then took a file and an exacto knife to clean up the edges to make it look significantly better. I suspect that MDF or foamcore backing would be a lot cleaner and might not need that step.

## Glueing LEDs
Once all of the holes were drilled, I rough fit the LEDs throught he holes in the back to try to find the optimal routing. IT's not important what order they're in yet--I'm just trying to waste the fewest LEDs. Once I figured it out, I started gluing a few at a time using the hot glue gun.

![First Glued LEDs](https://wkharmon.github.io/led-sectional/images/First glued leds.jpg)

In the picture you can see that I cut the strand because the next LED in the strand was too far from the previous one, but eventually I realized that LEDs are cheap, so it is much easier to just leave the LEDs on the strand and not use them than to cut the strand and splice in a jumper. The final product looked like this:

![Final Back](https://wkharmon.github.io/led-sectional/images/Final back.jpg)

#Final Details
Because I don't have good tools or really any skills with woodworking, I purchased a frame from http://www.pictureframes.com. Their SP8 frame has 2 1/4" of depth, which is plenty to contain the electronics. For my 24"x36 1/2" project, it cost me about $120 shipped for the frame, and it came fully assembled with mounting hardware (but no glass), which I thought was a pretty good details.

I mounted the map on a place on my wall previously had a wall sconce light, so I was able to re-use the existing power rather than running new power or running a cable.

Enjoy!