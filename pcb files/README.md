# PCB Files
This is a simple PCB design that holds a Wemos D1 Mini and other recommended components for driving RGB LEDs (like WS2811s).

It requires the following components:
* [Wemos D1 Mini](https://www.aliexpress.com/store/product/D1-mini-Mini-NodeMcu-4M-bytes-Lua-WIFI-Internet-of-Things-development-board-based-ESP8266/1331105_32529101036.html)
* [330 Ohm Resistor](https://www.digikey.com/product-detail/en/yageo/CFR-25JB-52-330R/330QBK-ND/1636)
* [1000 µF Capacitor](https://www.digikey.com/product-detail/en/nichicon/RL80J102MDN1KX/493-4024-1-ND/2347912)
* [3-pin JST XH Connector](https://www.digikey.com/product-detail/en/jst-sales-america-inc/B3B-XH-A-LF-SN/455-2248-ND/1651046)
* [74HCT245N Level Shifter](https://www.digikey.com/product-detail/en/texas-instruments/SN74HCT245N/296-1612-5-ND/277258)
* [2.1mm x 5.5mm DC Barrel Jack](https://www.digikey.com/product-detail/en/cui-inc/PJ-037A/CP-037A-ND/1644545)
* (optional) [4-pin JST XH Connector](https://www.digikey.com/product-detail/en/jst-sales-america-inc/B4B-XH-A-LF-SN/455-2249-ND/1651047)
    * Used for connecting a TSL2561 digital light sensor
* (optional) [2-pin JST XH Connector](https://www.digikey.com/product-detail/en/jst-sales-america-inc/B2B-XH-A(LF)(SN)/455-2247-ND/1651045)
    * Used to attach an external reset button
* (optional) [6mm Square Button](https://www.digikey.com/product-detail/en/te-connectivity-alcoswitch-switches/1825910-6/450-1650-ND/1632536)
    * Connected to GPIO12 to start WiFiManager configuration