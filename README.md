# highStriker2
Scientifically accurate load cell based high striker

<img src="https://user-images.githubusercontent.com/12158282/65821261-e71e9600-e23b-11e9-989a-9c3317056b02.jpg" width="400">

## What is this?
This project originally was created for a client as a marketing tool to be used in shopping centers.  
3 units were made to be used in Lithuania, Latvia, and Estonia.  
Later, one was built for our local maker fair.

## Details
### Hardware
STM32F103C8 with load cell frontend and peak detector.
 
Generic load cell ADCs are too slow to capture the peak of a hit, so a custom implementation is needed.  
The difficult part was to find a fast instrumentation amplifier, eventually settled on Microchip MCP6N16T-100E.  
The 2000kg YZC-516C load cell costs 20€ and has 2.0mV/V full-scale sensitivity. A gain of 500 was used.

Three WS2812 LED strips connected in parallel display the current score and when idle the high score.  
There are three RS485 lines with RJ45 connectors, but they ended up unused. We didn't need additional boards/LEDs.

Schematic and board design was drawn over a weekend.  
Parts were sourced from China, eBay, and Arrow.com.

<img src="https://user-images.githubusercontent.com/12158282/65821262-e7b72c80-e23b-11e9-938f-dcf5b51926b0.jpg" width="400">

### Firmware
Built on PlatformIO with Arduino framework.

ADC collection runs with DMA, but it is not really necessary as the peak detector holds the value for a couple of hundred milliseconds.  
Highscore is stored in a small external EEPROM, as well as sensitivity and zero offset.  
The four buttons are for some in field settings adjustment.

| Buttons     | Action                   |
| ----------- | ------------------------ |
| BTN1        | Sensitivity range adjust |
| BTN2 + BTN3 | Highscore reset          |
| BTN4        | Set zero offset          |

USB is a simple serial 115200 dump of raw values for infield debugging.

Due to the STM32F103 errata, we need two changes:

| File                              | Change                                     |
| --------------------------------- | ------------------------------------------ |
| STM32F1\libraries\Wire\Wire.cpp   | TwoWire Wire(1, I2C_FAST_MODE\|I2C_REMAP); |
| STM32F1\libraries\SPI\src\SPI.cpp | SPIClass SPI(2);                           |

## Performance
The 2000kg load cell is perfect for this, we are yet to see anyone reach the top of the scale.  
Beware that the frame must be made from very thick steel - don't underestimate anyone with a sledgehammer.

## Calibration
Our main test was to sit down on the hit plate and measure our weight, it is pretty accurate.

## Build me one
There are some spare PCBs so contact https://saynomore.lt/

## License
This work is licensed under a [GPL-3.0](LICENSE) license.