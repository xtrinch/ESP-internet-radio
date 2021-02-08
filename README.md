# ESP32-internet-radio

Internet radio station using an ESP32, VS1053 module, a TFT ILI9341 screen and an IR sensor.

Based on https://github.com/Edzelf/Esp-radio but heavily modified.

All configuration, including station presets are hardcoded.

Interaction with the radio happens via the buttons on the ILI9341's touch screen and the IR remote (to be done!). 
Supports starting, stopping, going to next or previous station.

Make sure you upload the `/data` folder where the fonts reside to the ESP before flashing. You can do that with `pio run --target uploadfs` or just use the platformio addon for vscode and click on `Upload Filesystem Image`.
