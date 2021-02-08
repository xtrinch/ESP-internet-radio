Import("env")

env.Append(CPPDEFINES=[
  ("WIFI_SSID", "ssid"),
	("WIFI_PASSWORD", "pass"),
  ("TFT_MISO", "19"),
  ("TFT_MOSI", "23"),
  ("TFT_SCLK", "18"),
  ("TFT_CS", "15"),
  ("TFT_RST", "-1"),
  ("TFT_DC", "2"),
  ("TOUCH_CS", "27"), # enable touch support, chip select
  ("USER_SETUP_LOADED", "1"), # to make sure the above variables are not marked duplicate
  ("ILI9341_DRIVER", "1"),
  ("SMOOTH_FONT", "1"), # to make sure we can load fonts
  ("SPI_FREQUENCY", "40000000"), # to make sure fonts are not pixelated
  ("SPI_READ_FREQUENCY", "20000000"),
  ("SPI_TOUCH_FREQUENCY", "2500000"),
])