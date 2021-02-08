# ESP32-internet-radio

Internet radio station using an ESP32, VS1053 module, a TFT ILI9341 screen and an IR sensor.

Based on https://github.com/Edzelf/Esp-radio but heavily modified.

All configuration, including station presets are hardcoded.

Interaction with the radio happens via the buttons on the ILI9341's touch screen and the IR remote (to be done!). 
Supports starting, stopping, going to next or previous station.

Make sure you upload the `/data` folder where the fonts reside to the ESP before flashing. You can do that with `pio run --target uploadfs` or just use the platformio addon for vscode and click on `Upload Filesystem Image`.

## Usage

To get started, copy `import_env.example.py` to `import_env.py` and fill in your pin configuration.

Example wiring:

| ESP32    | Signal | Wired to LCD       | Wired to VS1053     | Wired to the rest          |
| -------- | ------ | --------------     | ------------------- | ---------------            |
| GPIO32   |        | -                  | pin 1 XDCS          |  -                         |
| GPIO22   |        | -                  | pin 2 XCS           |  -                         |
| GPIO4    |        | -                  | pin 4 DREQ          |  -                         |
| GPIO2    |        | pin 3 D/C or A0    | -                   |  -                         |
| GPIO18   | SCK    | pin 5 CLK or SCK   | pin 5 SCK           |  -                         |
| GPIO19   | MISO   | -                  | pin 7 MISO          |  -                         |
| GPIO23   | MOSI   | pin 4 DIN or SDA   | pin 6 MOSI          |  -                         |
| GPIO15   |        | pin 2 TFT CS       | -                   |  -                         |
| GPIO27   |        | pin TOUCH CS       | -                   |  -                         |
| GPIO25   | -      | -                  | -                   |  Infrared receiver VS1838B |
| -------  | ------ | ---------------    | ------------------- |  ----------------          |
| GND      | -      | pin 8 GND          | pin 8 GND           |  Power supply GND          |
| VCC 5 V  | -      | pin 7 BL           | -                   |  Power supply              |
| VCC 5 V  | -      | pin 6 VCC          | pin 9 5V            |  Power supply              |
| EN       | -      | pin 1 RST          | pin 3 XRST          |  -                         |

