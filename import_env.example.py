Import("env")

env.Append(CPPDEFINES=[
  ("WIFI_SSID", "ssid"), # define only if preconfiguring
	("WIFI_PASSWORD", "pass"), # define only if preconfiguring
])