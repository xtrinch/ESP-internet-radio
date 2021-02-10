Import("env")

env.Append(CPPDEFINES=[
  ("WIFI_SSID", "ssid"),
	("WIFI_PASSWORD", "pass"),
  ("TFT_MISO", "19"), # shared with VS1053, touch controller & TFT controller
  ("TFT_MOSI", "23"), # shared with VS1053, touch controller & TFT controller
  ("TFT_SCLK", "18"), # shared with VS1053, touch controller & TFT controller
  ("TFT_CS", "15"),
  ("TFT_RST", "-1"), # tied to enable
  ("TFT_DC", "2"),
  ("TFT_BL", "21"), # backlight pin
  ("TFT_TOUCH_IRQ_PIN", "26"),
  ("VS_CS_PIN", "22"), # VS1053 chip select pin
  ("VS_DCS_PIN", "32"), # VS1053 DCS pin
  ("VS_DREQ_PIN", "4"), # VS1053 DREQ pin
  ("IR_PIN", "25"),
  ("TOUCH_CS", "27"), # enable touch support, chip select
  ("USER_SETUP_LOADED", "1"), # to make sure the above variables are not marked duplicate
  ("ILI9341_DRIVER", "1"),
  ("SMOOTH_FONT", "1"), # to make sure we can load fonts
  ("SPI_FREQUENCY", "40000000"), # to make sure fonts are not pixelated
  ("SPI_READ_FREQUENCY", "20000000"),
  ("SPI_TOUCH_FREQUENCY", "2500000"),
])