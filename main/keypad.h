#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

enum {
  OUTSIDE_KEYAREA = -1
};

struct Button {
  String label;
  int id;
  uint16_t bgcolor = ILI9341_BLACK;
};

struct Rect {
  int x, y, w, h;
};


template <int R, int C>
class KeyPad
{
  const Button (&keyboard)[R][C];
  Rect area;

public:

  KeyPad(const Button (&kbd)[R][C], Rect screen_area)
    : keyboard(kbd)
    , area(screen_area)
  {
  }

  int DetectButtons(int X, int Y)
  {
    if (X < area.x or X >= area.x + area.w or
        Y < area.y or Y >= area.y + area.h)
    {
      return OUTSIDE_KEYAREA;
    }

    int i = (Y - area.y) * R / area.h;
    int j = (area.x + area.w - X) * C / area.w;
    return keyboard[i][j].id;
  }

  void draw(Adafruit_ILI9341& tft)
  {
    tft.setFont(0);

    int X0 = area.x;
    int Y0 = area.y;
    int W = area.w;
    int H = area.h;
    int DX = W / C;
    int DY = H / R;

    // Draw button background colors
    tft.fillRect(X0, Y0, W, H, ILI9341_BLACK);
    for (int i = 0; i < R; ++i) {
      for (int j = 0; j < C; ++j) {
        uint16_t bgcolor = keyboard[i][j].bgcolor;
        if (bgcolor != ILI9341_BLACK) {
          tft.fillRect(X0+DX*j, Y0+DY*i, DX, DY, bgcolor);
        }
      }
    }

    // Draw grid
    for (int i = 1; i < R; ++i) {
      tft.drawFastHLine(X0, Y0+DY*i, W, ILI9341_WHITE);
    }
    for (int j = 1; j < C; ++j) {
      tft.drawFastVLine(X0+DX*j, Y0, H, ILI9341_WHITE);
    }

    // Display keypad lables
    for (int i = 0; i < R; ++i) {
      for (int j = 0; j < C; ++j) {
        const String& label = keyboard[i][j].label;

        tft.setCursor(
            X0+DX*j + DX/2 - 8*label.length(),
            Y0+DY*i + DY/2 - 10);

        tft.setTextSize(3);
        tft.setTextColor(ILI9341_WHITE);
        tft.println(label);
      }
    }
  }
};

