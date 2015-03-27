# E46 Cluster Driver
An Arduino sketch and schematic for the hardware to drive a BMW instrument cluster over CAN-BUS, KBUS and general IO lines. The firmware has been designed to work with my [DashboardSender](https://github.com/tsharp42/DashboardLibrary) application.

## What Is Supported
Currently the board can interact with these parts of the instrument cluster, though some parts are not implemented in the software yet:
* Speedometer
* Tachometer
* Parking Brake LED
* Cruise Control LED
* ABS Warning LED
* Airbag Warning LED
* Fog Light Status
* Full Beam Status
* Coolant Temperature Gauge
* Indicators (Turn Signals) - Including Double Rate
* Door open status
* Bulb out warnings
* Set Time

Not currently working:
* Fuel Gauge
* MPG Readout

## Hardware
For ease of assembly onto standard strip board the design uses the Atmega328p in a DIP package. Two Atmels are used as the KBUS requires the hardware UART to send the data. The two chips are connected over the I2C bus. If using an variant with 2 or more UARTs it is entirely possible to roll the whole firmware into one chip.

### CAN-BUS
To interface with the CAN-BUS the MCP2551 and MCP2515 ICs are used, these connect to the main processor over SPI.

### KBUS
The KBUS connection is handled by a stripped down version of the driver circuitry designed by Thaniel, see Resources.
Only the TX portion is used as the instrument cluster only sends a periodic status packet which can be ignored for the purposes of driving the cluster. An MCP2025 Linbus tranceiver could also be used.

### General IO
Some of the functions (Such as the parking brake LED) require certain pins to be pulled to GND from 12V, these are handled using a 2N2222A on the outputs of a PCF8574. This IO Expander is used just incase I need the other Atmel pins for other uses later on (Fuel guage?)

## Libraries
The main Arduino sketch requires these libraries:
* [PCF8574](https://github.com/skywodd/pcf8574_arduino_library)
* [SimpleTimer](https://github.com/jfturcot/SimpleTimer)
* [CmdMessenger](https://github.com/dreamcat4/CmdMessenger)
* [MCP_Can_lib](https://github.com/coryjfowler/MCP_CAN_lib)

# Resources
* [Thaniel's E46 Blog](http://e46canbus.blogspot.co.uk/)
* [IBus Information](http://web.comhem.se/bengt-olof.swing/IBus.htm)
* [CanBus Messages](http://www.bimmerforums.com/forum/showthread.php?1887229-E46-Can-bus-project)
* [Cluster Pinout](http://www.bmwgm5.com/E46_IKE_Connections.htm)
