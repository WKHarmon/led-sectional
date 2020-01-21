# PCB Files
This is a simple PCB design that holds a Wemos D1 Mini and other recommended components for driving RGB LEDs (like WS2811s). It is optimized for SMD assembly.

It requires the following components:
* [Wemos D1 Mini](https://www.aliexpress.com/store/product/D1-mini-Mini-NodeMcu-4M-bytes-Lua-WIFI-Internet-of-Things-development-board-based-ESP8266/1331105_32529101036.html)
* [330 Ohm Resistor](https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_UNI-ROYAL-Uniroyal-Elec-0603WAF3300T5E_C23138.html)
* [1000 µF Capacitor](https://lcsc.com/product-detail/Tantalum-Capacitors_AVX-TAJE108M006RNJ_C54883.html)
* [74HCT245PW Level Shifter](https://lcsc.com/product-detail/74-Series_Nexperia-74HCT245PW_C5980.html)
* [3-pin JST XH Connector](https://www.digikey.com/product-detail/en/jst-sales-america-inc/B3B-XH-A-LF-SN/455-2248-ND/1651046)
* [2.1mm x 5.5mm DC Barrel Jack](https://www.digikey.com/product-detail/en/cui-inc/PJ-037A/CP-037A-ND/1644545)
* (optional) [5-pin JST XH Connector](https://www.digikey.com/product-detail/en/jst-sales-america-inc/B5B-XH-A-LF-SN/455-2270-ND/1530483)
    * Used for connecting a TSL2561 digital light sensor
* (optional) [2-pin JST XH Connector](https://www.digikey.com/product-detail/en/jst-sales-america-inc/B2B-XH-A(LF)(SN)/455-2247-ND/1651045)
    * Used to attach an external reset button
* (optional) [4.5mm Square Button](https://lcsc.com/product-detail/Tactile-Switches_C-K_PTS530GN055SMTRLFS_C-K-PTS530GN055SMTRLFS_C221861.html)
    * Connected to GPIO12 to start WiFiManager configuration