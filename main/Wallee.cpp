#include "Formatting.hpp"
#include "keypad.h"

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <RTClib.h>
#include <SD.h>
#include <XPT2046_Touchscreen.h>


// pin definitions
const uint8_t PIN_TFT_DC    =  4;
const uint8_t PIN_TFT_CS    =  5;
const uint8_t PIN_TFT_SCK   = 18;   // VSPI (default)
const uint8_t PIN_TFT_MISO  = 19;   // VSPI (default)
const uint8_t PIN_TFT_MOSI  = 23;   // VSPI (default)
const uint8_t PIN_TFT_LED   = 15;
const uint8_t PIN_TFT_RST   = 22;
const uint8_t PIN_TOUCH_CS  = 14;
const uint8_t PIN_TOUCH_IRQ = 27;
const uint8_t PIN_BEEPER    = 21;

const uint8_t PIN_SD_CS     = 32;
const uint8_t PIN_SD_SCK    = 33;   // HSPI
const uint8_t PIN_SD_MOSI   = 25;   // HSPI
const uint8_t PIN_SD_MISO   = 26;   // HSPI

// NOTE: GPIO 12 (MTDI) is a boot strapping pin. It must be LOW during boot,
// but plugging in the RTC pulls it HIGH. The boot then fails due to incorrect
// configuration. In order to work around this, apply the following (irreversible):
//
//    espefuse.py burn_efuse XPD_SDIO_TIEH 1
//    espefuse.py burn_efuse XPD_SDIO_REG 1
//    espefuse.py burn_efuse XPD_SDIO_FORCE 1
const uint8_t PIN_RTC_SCL   = 12;
const uint8_t PIN_RTC_SDA   = 13;

// calibrate touchscreen
const int MINPRESSURE = 10;         // minimum required force for touch event
const int TS_MINX = 370;
const int TS_MINY = 470;
const int TS_MAXX = 3700;
const int TS_MAXY = 3600;


// devices
SPIClass SPI2(HSPI);

Adafruit_ILI9341 tft(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);
XPT2046_Touchscreen touch(PIN_TOUCH_CS, PIN_TOUCH_IRQ);

RTC_DS3231 rtc;


// app state
long inputNumber = 0;
bool wasDown = false;

enum class UiState {
  Standby,
  Keypad,
};
UiState uiState = UiState::Standby;

enum Keycode {
  KEY_0 = 0,
  KEY_1 = 1,
  KEY_2 = 2,
  KEY_3 = 3,
  KEY_4 = 4,
  KEY_5 = 5,
  KEY_6 = 6,
  KEY_7 = 7,
  KEY_8 = 8,
  KEY_9 = 9,
  KEY_OK = 10,
  KEY_ESC = 11,
};

const Button
  BTN_0   = {"0", KEY_0},
  BTN_1   = {"1", KEY_1},
  BTN_2   = {"2", KEY_2},
  BTN_3   = {"3", KEY_3},
  BTN_4   = {"4", KEY_4},
  BTN_5   = {"5", KEY_5},
  BTN_6   = {"6", KEY_6},
  BTN_7   = {"7", KEY_7},
  BTN_8   = {"8", KEY_8},
  BTN_9   = {"9", KEY_9},
  BTN_OK  = {"OK", KEY_OK, ILI9341_GREEN},
  BTN_ESC = {"C", KEY_ESC, ILI9341_RED};

Button keypad_buttons[4][3] = {
  { BTN_7,   BTN_8,  BTN_9 },
  { BTN_4,   BTN_5,  BTN_6 },
  { BTN_1,   BTN_2,  BTN_3 },
  { BTN_ESC, BTN_0,  BTN_OK },
};

KeyPad keypad(keypad_buttons, {0, 80, 240, 320 - 80});


// Forward declarations
void interactKeypad(int X, int Y);
void enableBacklight();
void disableBacklight();
TS_Point getTouchEvent();
void showResultNumber(long result);
void showResultText(int color, String text, int xPos);
void drawKeypad();
void playSoundButtonOk();
void playSoundButtonCancel();
void playSoundResultOk();


void setup()
{
  Serial.begin(115200);

  // RTC
  Wire.begin(PIN_RTC_SDA, PIN_RTC_SCL);
  rtc.begin(&Wire);
  rtc.disable32K();
  rtc.writeSqwPinMode(Ds3231SqwPinMode::DS3231_OFF);


  // SD card
  bool SPI_BEGIN = SPI2.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
  bool SD_BEGIN = SPI_BEGIN and SD.begin(PIN_SD_CS, SPI2);

  // Display
  pinMode(PIN_TFT_LED, OUTPUT);
  disableBacklight();

  tft.begin();
  touch.begin();

  tft.setRotation(2);
  tft.fillScreen(ILI9341_BLACK);
  drawKeypad();
  showResultNumber(inputNumber);

  // Sound
  ledcAttachChannel(PIN_BEEPER, /*freq*/1e5, /*resolution*/8, /*channel*/0);
}


void loop()
{
  TS_Point p = getTouchEvent();
  bool isDown = p.z >= MINPRESSURE;

  if (isDown and not wasDown) {
    switch (uiState) {
      case UiState::Standby:
        enableBacklight();
        break;
      case UiState::Keypad:
        interactKeypad(p.y, p.x);
        break;
    };
  }

  delay(100);
  ledcWriteTone(PIN_BEEPER, 0);
  wasDown = isDown;
}


void interactKeypad(int X, int Y)
{
  int keycode = keypad.DetectButtons(X, Y);

  if (keycode >= KEY_0 and keycode <= KEY_9) {
    inputNumber = (inputNumber*10) + (keycode - KEY_0);
    showResultNumber(inputNumber);
    playSoundButtonOk();
  }

  else if (keycode == KEY_OK) {
    File logfile = SD.open("scale.log", FILE_APPEND, true);
    logfile << rtc.now() << " " << inputNumber << endl;
    Serial << rtc.now() << " " << inputNumber << endl;;

    showResultText(ILI9341_GREEN, "CODE OK", 60);
    playSoundResultOk();

    delay(1000);
    inputNumber = 0;
    showResultNumber(inputNumber);
  }

  else if (keycode == KEY_ESC) {
    if (inputNumber == 0) {
      disableBacklight();
    }
    inputNumber = 0;
    showResultNumber(inputNumber);
    playSoundButtonCancel();
  }
}


void enableBacklight()
{
  digitalWrite(PIN_TFT_LED, LOW);
  uiState = UiState::Keypad;
}

void disableBacklight()
{
  digitalWrite(PIN_TFT_LED, HIGH);
  uiState = UiState::Standby;
}

TS_Point getTouchEvent()
{
  TS_Point p = touch.getPoint();
  p.x = 320 - map(p.x, TS_MINX, TS_MAXX, 320, 0);
  p.y = 240 - map(p.y, TS_MINY, TS_MAXY, 240, 0);
  return p;
}

void showResultNumber(long result)
{
  tft.fillRect(0, 0, 240, 80, ILI9341_CYAN);
  tft.setCursor(10, 20);
  tft.setTextSize(4);
  tft.setTextColor(ILI9341_BLACK);
  tft.println(String(result));
}

void showResultText(int color, String text, int xPos)
{
  tft.fillRect(0, 0, 240, 80, color);
  tft.setCursor(xPos, 26);
  tft.setTextSize(3);
  tft.setTextColor(ILI9341_WHITE);
  tft.println(text);
}

void drawKeypad()
{
  tft.fillRect(0, 0, 240, 80, ILI9341_CYAN);
  tft.drawFastHLine(0, 80, 240, ILI9341_WHITE);
  keypad.draw(tft);
}

void playSoundButtonOk()
{
  ledcWriteTone(PIN_BEEPER, 4000);
}

void playSoundButtonCancel()
{
  for (int i = 0; i < 2; i++) {
    ledcWriteTone(PIN_BEEPER, 4500);
    delay(100);
    ledcWriteTone(PIN_BEEPER, 0);
    delay(50);
  }
}

void playSoundResultOk()
{
  for (int i = 0; i < 3; i++) {
    ledcWriteTone(PIN_BEEPER, 4000);
    delay(100);
    ledcWriteTone(PIN_BEEPER, 0);
    delay(50);
  }
}
