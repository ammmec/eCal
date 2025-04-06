// uncomment next line to use class GFX of library GFX_Root instead of Adafruit_GFX
//#include <GFX.h>

#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include "schedule.h"

#define PWR  D11  // 7
#define BUSY D7   // 18
#define RST  D6   // 1
#define DC   D3   // 14
#define CS   SS   // 5

#define RECT_HEIGHT 61
#define RECT_WIDTH 200
#define TEXT_HEIGHT 9
#define TOP_MARGIN 4
#define BOTTOM_MARGIN 3

// alternately you can copy the constructor from GxEPD2_display_selection.h or GxEPD2_display_selection_added.h to here
GxEPD2_3C < GxEPD2_750c_Z08, GxEPD2_750c_Z08::HEIGHT / 4 > display(GxEPD2_750c_Z08(/*CS=*/ CS, /*DC=*/ DC, /*RST=*/ RST, /*BUSY=*/ BUSY)); // GDEW075Z08 800x480, GD7965

void setup() {
  pinMode(PWR, OUTPUT);
  digitalWrite(PWR, HIGH);
  display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  Serial.println("Starting up");
  writeText();
  //drawBp();
  Serial.println("Going to sleep");
  display.powerOff();
  digitalWrite(PWR, LOW);
}

void drawBp() {
  display.setRotation(1);
  display.firstPage();
  display.setFullWindow();

  Serial.println("Starting to print!");
  do {
    display.drawBitmap (0, 0, schedule, 480, 800, GxEPD_BLACK);
  }
  while (display.nextPage());
}

void drawRectPink(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  uint16_t x_end = x + w;
  uint16_t y_end = y + h;
  for (int i = x; i < x_end; i++) {
    for (int j = y; j < y_end; j++) {
      // Super light pink
      if (i%2 == 0 && j%2 == 0) display.drawPixel(i, j, GxEPD_RED);
    }
  }
}
/*
font                 xAdvance  yAdvance

FreeMono9pt7b        11        18
FreeMonoBold9pt7b    11        18
*/
void drawHours() {
  // Giving some margin on the left
  uint16_t x = 5;
  // y position ->  TOP_MARGIN for the border
  //                (RECT_HEIGHT - TEXT_HEIGHT)/2 to center inside its spot
  //                - TEXT_HEIGHT to offset the text's height
  uint16_t y = TOP_MARGIN + (RECT_HEIGHT - TEXT_HEIGHT)/2 - TEXT_HEIGHT;
  for (int i = 8; i < 21; ++i) {
    display.setCursor(x, y);
    display.drawChar('0'+i);
    display.print(i);
    y += RECT_HEIGHT;
  }
}

void writeText() {
  display.setRotation(1);
  display.firstPage();
  display.setFullWindow();
  display.setFont(&FreeMonoBold9pt7b);


  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds("8-9h", 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = 95;
  uint16_t y = 40;

  Serial.println("Starting to print!");
  do {
    drawHours();
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.writeFastVLine (300, 0, 800, GxEPD_BLACK);
    display.drawFastHLine (300, 100, 180, GxEPD_BLACK);
    drawRectPink(55, 5, 220, 70);

    display.setCursor(5, y);
    display.print("8-9h");

    display.setTextColor(GxEPD_WHITE);
    for (int i = -3; i <= 3; ++i) {
      for (int j = -2; j <= 2; ++j) {
        display.setCursor(x+i, y+j);
        display.print("FM 10 T");
      }
    }

    display.setTextColor(GxEPD_BLACK);
    display.setCursor(x, y);
    display.print("FM 10 T");
  }
  while (display.nextPage());
}

void loop() {};
