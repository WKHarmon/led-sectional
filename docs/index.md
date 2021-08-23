LED Sectional
=============
* [LED Sectional](#led-sectional)
* [Pictures](#pictures)
* [Things you'll need](#things-youll-need)
    * [Electronics](#electronics)
        * [LED Sectional Kit](#led-sectional-kit)
        * [Buying a la carte](#buying-a-la-carte)
    * [Map and Frame](#map-and-frame)
    * [Tools](#tools)
* [Software](#software)
* [Assembly](#assembly)
    * [Soldering and Wiring](#soldering-and-wiring)
    * [Mounting the sectionals](#mounting-the-sectionals)
    * [Drilling](#drilling)
    * [Glueing LEDs](#glueing-leds)
* [Final Details](#final-details)

This project is adapted from a few other very similar ones:
* [Live sectional map](http://blog.dylanhrush.com/2017/07/live-sectional-map.html)
* [Flight conditions sectional wall art thing](https://www.reddit.com/r/flying/comments/7avr8q/flight_conditions_sectional_wall_art_thing/)
* [Creating a METAR map](https://pplaviationuk.wordpress.com/creating-a-metar-map/)

There were small gaps in the how-to details for some of them so I figured it might be helpful to fully document how I did it. I also opted to use a microcontroller rather than a Raspberry Pi, so mine is different in that way too.


# Pictures
Here are a few pictures of the final product:
![Final Product](https://wkharmon.github.io/led-sectional/images/Final 1.jpg)
![Final Product](https://wkharmon.github.io/led-sectional/images/Final 2.jpg)
![Final Product](https://wkharmon.github.io/led-sectional/images/Final 3.jpg)


# Things you'll need
The materials themselves are pretty cheap--you should be able to get everything you need aside from the frame for less than $100. The frame cost will vary depending on how big you want to make the sectional. The tools may add some cost if you don't already have what you need.

## Electronics
### LED Sectional Kit
I originally wrote this guide with instructions on how to buy all of the electronic components on Amazon, but now I have a kit available that contains all of the electronics except for the power adapter and the LED strands themslves. The kit is significantly cheaper than buying each of the components a la carte since most of the components come in large quantities.

[Click here to find out more information about the kit.](kit.md)

The Amazon links from the original how-to are below in case you'd enjoy doing everything yourself.

### Buying a la carte
I've linked to Amazon for most of the electronic items. You should be able to buy them for cheaper and in smaller quantites from Aliexpress or another similar store if you're willing to wait for the shipment. (In the interest of full disclosure, the Amazon links do have my affiliate tags, but I'm not really trying to make any money here.)
* [WEMOS D1 Mini Lite](https://www.aliexpress.com/store/product/WEMOS-D1-mini-Lite-V1-0-0-WIFI-Internet-of-Things-development-board-based-ESP8285-1MB/1331105_32795857574.html?spm=2114.12010615.8148356.30.10027c8csIW2n0)
    * If you have time to wait for the shipment, support them and buy from the official store, but [this version on Amazon](https://amzn.to/2R4Pjio) also worked fine.
    * The non-Lite version will work fine, and this code should be pretty adaptable to any Arduino-compatible board with WiFi.
* 3.3V to 5V Level Shifter
    * [WeiMeet 4 Channel Level Converter](https://amzn.to/2R50OGu) - These are what I originally used (and will document here), but they're considered to be a bit slow. You may run into problems if you have a ton of LEDs.
    * [74HCT245](https://www.aliexpress.com/item/10PCS-SN74HCT245N-DIP20-SN74HCT245-DIP-74HCT245-74HCT245N-DIP-20-new-and-original-IC/32537892034.html?spm=a2g0s.9042311.0.0.58d84c4d3QyyCc) are faster and are what I use in the kit.
* [WS2811 LED Strand(s)](https://amzn.to/2BsGVzj)
* [5V DC Power Supply](https://amzn.to/2EE5Oft) - This is more power than you really need--you could get a smaller one if you'd prefer.
* [DC Barrel Connectors](https://amzn.to/2Gu7aeh)
* [Solid Core Hookup Wire](https://amzn.to/2BwFLD3) - 22G worked well for me
* [3cm x 7cm PCB](https://amzn.to/2GudB15) - This has a bunch of PCBs, I'm sure there's a cheaper opton. Other sizes will work, but these fit nicely into a small project enclosure.
* Optional: [80mm x 50mm x 20mm Project Enclosure](https://amzn.to/2ShrLUw)
* Optional: 3-pin [JST-XH Connectors](https://amzn.to/2SbjpOi) and [JST-SM Connectors](https://amzn.to/2LoXkJy) if you want a interface between the microcontroller and the LED strand(s)
* Optional: [Velcro](https://amzn.to/2BssRpr) to affix the power supply and controller to the board
* Optional components recommended by NeoPixels that I didn't add:
    * 1000 µF, 6.3V or higher capacitor - put this between the 5v and Ground on the power adapter to mitigate spikes on power-on
    * 220 Ohm resistor - put this on the data wire between the level shifter HV1 pin and the data wire on the connector to the LED strand to also mitigate power spikes

## Map and Frame
**You'll also need either:**
* Sectional(s) from the FAA - remember that they're two sided so you may need two to get full coverage
* 3/8" Foamcore or [Hard Tempered Board](https://www.homedepot.com/p/Hardboard-Tempered-Common-1-8-in-x-2-ft-x-4-ft-Actual-0-125-in-x-23-75-in-x-47-75-in-7005015/202088786) or MDF
    * I used the board in my first map, but found foamcore to be much easier to work with on one I made for a friend.
* [3M Super 77 Spray Adhesive](https://www.homedepot.com/p/3M-16-7-oz-Super-77-Multi-Purpose-Spray-Adhesive-77-24VOC30/301619194)
* [SP8 Picture Frame](https://www.pictureframes.com/Custom-Frames/Canvas-Ready-Wood-Picture-Frames/SP8-black-wood-canvas-frame)
    * This is one of the only super-deep frames I've seen that are easily orderable online.
* Optional: [PVA Adhesive](https://amzn.to/2EC5qhn) if you have a seam between sectionals

**OR**

* Have a [digital file](https://www.faa.gov/air_traffic/flight_info/aeronav/digital_products/vfr/) of the sectional printed and mounted on foamcore at [PictureFrames.com](http://www.pictureframes.com). If you do this, be sure to maintain the 300 ppi from the original file in order to keep the sectional to scale (assuming you want that at least).
    * Use the [SP8 Picture Frame](https://www.pictureframes.com/Custom-Frames/Canvas-Ready-Wood-Picture-Frames/SP8-black-wood-canvas-frame) for plenty of depth. The [SP2 Picture Frame](https://www.pictureframes.com/Custom-Frames/Canvas-Ready-Wood-Picture-Frames/SP2-black-wood-canvas-frame) also works pretty well, but the wires may rub against the wall.

## Tools
Here are the tools you'll need. I've provided links to the ones I use, but any set of good tools will work.
* [Glue gun](https://amzn.to/2V492Om) with plenty of [glue](https://amzn.to/2Sby6RA)

If you used foamcore backing:
* [Foamboard Hole Drill](https://amzn.to/2VIc7Du) (You'll use the smallest one in the set.)

If you used wood backing:
* Drill with a 7mm drill bit

If you went the a la carte route for the electronics, you'll also need:
* [Soldering iron](https://amzn.to/2BwFUGB) and solder
* [Wire stripper](https://amzn.to/2BzBWg6)
* Optional: [Crimper for JST connectors](https://amzn.to/2S5a1fi)

# Software
The code I used for this project is available at [https://github.com/WKHarmon/led-sectional](https://github.com/WKHarmon/led-sectional). It is designed to use the Arduino IDE and should be relatively easy to adapt for your own purposes. I should mention that I'm not an engineer--I just hack together code to try to make it work, so I'm sure the code is ugly.

The program uses standard definitions for VFR, MVFR, IFR, and LIFR, but I also set it to show yellow if an airport is VFR but winds are greater than 25 knots, and an airport will strobe white every 5 seconds if there are thunderstorms in the area. These can be disabled by setting `DO_WINDS` and `DO_LIGHTNING` to false respectively.

On power on, all LEDs will show orange initially. Once the controller is connected to the WiFi network, they'll turn purple, and then they'll show the airport colors once the data is downloaded in parsed. LEDs for airports that have not reported weather in the last 3 hours, or that don't have a clear flight category will be turned off. Weather is updated every 15 minutes, and the update rate can be adjusted in `REQUEST_INTERVAL`.

You'll need to install the following in order to program the board:
* [ESP8266 Core](https://github.com/esp8266/Arduino) is necessary to use the ESP8266-compatible controller in the Arduino IDE.
* [CH340G Driver](https://www.wemos.cc/en/latest/tutorials/d1/get_started_with_arduino_d1.html) is necessary for your computer to see the board
* Within the Library Manager in the Arduino IDE, you'll also need to install the [FastLED Library](http://fastled.io/).
    * If you're using a TSL2561 Ambient Light Sensor from the kit, you'll also need to add the Adafruit Unified Sensor and hte Adafruit TSL2561 libraries as well.

Once you have all of that installed, you'll need to set the Board to `LOLIN(WEMOS) D1 mini Lite` under Tools in the Arduino IDE.

The `airports` vector contains all of the airports used for the project in the order in which they appear in the LED strand. `NULL` is used when we have unused LEDs in the strand. If you'd like to add a key, you can mark those LEDs as `VFR`, `WVFR`, `MVFR`, `IFR`, and `LIFR`. You'll also need to edit in the SSID and Password for your WiFi network.


# Assembly
## Soldering and Wiring
The following section is only necessary if you went the a la carte route.

The Wemos D1 Mini Lite can accept 5V as input voltage, but the GPIO pins run at 3.3v. The WS2811 LED strand is expecting 5V on the data line, so it is necessary to convert the GPIO 3.3v to the WS2811 5V using a logic level shifter. (In some cases it may work without the shifter, but using one is the safer bet.) Below is a breadboard diagram for wiring it all up:

![Breadboard Diagram](https://wkharmon.github.io/led-sectional/images/Wemos PCB 1_bb.png)

I've diagramed on a breadboard for simplicity, but I used a 3cm x 7cm PCB. In the diagram, the (optional) JST-XH connector on the right goes to the WS2811 LED strand. If you don't want to bother with the connector, you can just solder the wires from the LED strand to the PCB.

![Controller on PCB](https://wkharmon.github.io/led-sectional/images/Microcontroller on PCB.jpg)

 If you have a smaller number of LEDs on your map, it may be possible to power the LED strand from the Micro USB port on the Wemos D1 Mini rather than connecting an external power supply. No wiring changes are necessary, but be sure to not connect an external power supply when the Micro USB port is connected. Also be aware that the Wemos D1 Mini has a 0.5A fuse on the 5V line so it would need to be a really small number of LEDs.	

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

The Foamboard Holl Drill makes super clean cuts and is the perfect size for the LEDs, so I highly recommend grabbing that set if you used foamcore backing and drilling using them.

On my first board I used wood backing, which was quite a bit more challenging. I used a 7mm drill bit to drill through the sectional and the backing board together. This resulted in some holes that weren't super clean.

![Rough Edges](https://wkharmon.github.io/led-sectional/images/Rough edges.jpg)

I specifically wanted my LEDs to stick out, but if you don't then adjusting the drill bit size and using a countersink bit would allow you to align them with the front of the board.

After I drilled, I then took a file and an exacto knife to clean up the edges to make it look significantly better.

## Glueing LEDs
Once all of the holes were drilled, I rough fit the LEDs throught he holes in the back to try to find the optimal routing. IT's not important what order they're in yet--I'm just trying to waste the fewest LEDs. Once I figured it out, I started gluing a few at a time using the hot glue gun.

![First Glued LEDs](https://wkharmon.github.io/led-sectional/images/First glued leds.jpg)

In the picture you can see that I cut the strand because the next LED in the strand was too far from the previous one, but eventually I realized that LEDs are cheap, so it is much easier to just leave the LEDs on the strand and not use them than to cut the strand and splice in a jumper. The final product looked like this:

![Final Back](https://wkharmon.github.io/led-sectional/images/Final back.jpg)


# Final Details
Because I don't have good tools or really any skills with woodworking, I purchased a frame from [http://www.pictureframes.com](https://www.pictureframes.com/Custom-Frames/Canvas-Ready-Wood-Picture-Frames/SP8-black-wood-canvas-frame). Their SP8 frame has 2 1/4" of depth, which is plenty to contain the electronics. For my 24"x36 1/2" project, it cost me about $120 shipped for the frame, and it came fully assembled with mounting hardware (but no glass), which I thought was a pretty good details.

I mounted the map on a place on my wall previously had a wall sconce light, so I was able to re-use the existing power rather than running new power or running a cable.

Enjoy!