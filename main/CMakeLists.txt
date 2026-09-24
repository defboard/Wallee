idf_component_register(
    SRCS "Wallee.cpp"
    INCLUDE_DIRS "."
    REQUIRES arduino-esp32 RTClib Adafruit_ILI9341 XPT2046_Touchscreen
)
