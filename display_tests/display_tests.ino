// uncomment next line to use class GFX of library GFX_Root instead of Adafruit_GFX
//#include <GFX.h>

#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include "heart.h"

// alternately you can copy the constructor from GxEPD2_display_selection.h or GxEPD2_display_selection_added.h to here
GxEPD2_3C < GxEPD2_750c_Z08, GxEPD2_750c_Z08::HEIGHT / 4 > display(GxEPD2_750c_Z08(/*CS=*/ SS, /*DC=*/ D3, /*RST=*/ D6, /*BUSY=*/ D7)); // GDEW075Z08 800x480, GD7965

void setup()
{
  pinMode(D11, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(SS, OUTPUT);
  digitalWrite(D11, HIGH);
  display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  //display.clearScreen();
  Serial.println("Starting up");
  helloWorld();
  digitalWrite(D11, LOW);
  Serial.println("Going to sleep");
  display.hibernate();
}

const char test[] = "Test Text";

void helloWorld()
{
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setFullWindow();
  display.firstPage();
  
  
  int16_t tbx, tby; uint16_t w, h;
  display.getTextBounds(test, 0, 0, &tbx, &tby, &w, &h);
  uint16_t x = ((display.width()/2 - w) / 2) - tbx;
  uint16_t y = ((display.height()/15 - h) / 2) - tby;

  uint16_t xr = x + display.width()/2;
  uint16_t y_inc = 53;

  Serial.println("Starting to print!");
  do {
    display.fillScreen(GxEPD_WHITE);
    y = ((display.height()/15 - h) / 2) - tby;

    // LIGHTEST PINK:
    for (int i = 0; i < 480; i++) {
      for (int j = 0; j < 160; j++) {
        // Pink shades: 
        if (i%3 == 0 && j%3 == 0 && i < 240) display.drawPixel(i, j, GxEPD_RED);
        if (i%3 == 0 && j%3 == 0 && i >= 240) display.drawPixel(i, j, GxEPD_BLACK);
      }
    }
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(x, y);
    display.print(test);
    display.setCursor(xr, y);
    display.print(test);
    y += y_inc;
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(test);
    display.setCursor(xr, y);
    display.print(test);
    y += y_inc;
    display.setTextColor(GxEPD_RED);
    display.setCursor(x, y);
    display.print(test);
    display.setCursor(xr, y);
    display.print(test);
    y += y_inc;

    // 2nd PINK
    for (int i = 0; i < 480; i++) {
      for (int j = 160; j < 320; j++) {
        // Pink shades: 
        // Super light pink
        if (i%2 == 0 && j%2 == 0 && i < 240) display.drawPixel(i, j, GxEPD_RED);
        if (i%2 == 0 && j%2 == 0 && i >= 240) display.drawPixel(i, j, GxEPD_BLACK);
        // Light pink
        /*if (i%3 == 0 && j%3 == 0) display.drawPixel(i, j, GxEPD_RED);
        else if ((i-1)%3 == 0 && (j-1)%3 == 0) display.drawPixel(i, j, GxEPD_RED);
        else if ((i+1)%3 == 0 && (j+1)%3 == 0) display.drawPixel(i, j, GxEPD_RED);*/
        // Darker pink
        /* if (i%2 == 0 && j%2 == 0) display.drawPixel(i, j, GxEPD_RED);
        else if (i%2 != 0 && j%2 != 0) display.drawPixel(i, j, GxEPD_RED); */
      }
    }
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(x, y);
    display.print(test);
    display.setCursor(xr, y);
    display.print(test);
    y += y_inc;
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(test);
    display.setCursor(xr, y);
    display.print(test);
    y += y_inc;
    display.setTextColor(GxEPD_RED);
    display.setCursor(x, y);
    display.print(test);
    display.setCursor(xr, y);
    display.print(test);
    y += y_inc;

    // 3RD PINK:
    for (int i = 0; i < 480; i++) {
      for (int j = 320; j < 480; j++) {
        // Pink shades: 
        // Super light pink
        //if (i%2 == 0 && j%2 == 0) display.drawPixel(i, j, GxEPD_RED);
        // Light pink
        if (i < 240) {
          if (i%3 == 0 && j%3 == 0) display.drawPixel(i, j, GxEPD_RED);
          else if ((i-1)%3 == 0 && (j-1)%3 == 0) display.drawPixel(i, j, GxEPD_RED);
          else if ((i+1)%3 == 0 && (j+1)%3 == 0) display.drawPixel(i, j, GxEPD_RED);
        }
        else {
          if (i%3 == 0 && j%3 == 0) display.drawPixel(i, j, GxEPD_BLACK);
          else if ((i-1)%3 == 0 && (j-1)%3 == 0) display.drawPixel(i, j, GxEPD_BLACK);
          else if ((i+1)%3 == 0 && (j+1)%3 == 0) display.drawPixel(i, j, GxEPD_BLACK);
        }
        // Darker pink
        /* if (i%2 == 0 && j%2 == 0) display.drawPixel(i, j, GxEPD_RED);
        else if (i%2 != 0 && j%2 != 0) display.drawPixel(i, j, GxEPD_RED); */
      }
    }
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(x, y);
    display.print(test);
    display.setCursor(xr, y);
    display.print(test);
    y += y_inc;
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(test);
    display.setCursor(xr, y);
    display.print(test);
    y += y_inc;
    display.setTextColor(GxEPD_RED);
    display.setCursor(x, y);
    display.print(test);
    display.setCursor(xr, y);
    display.print(test);
    y += y_inc;
    
    // 4TH PINK
    for (int i = 0; i < 480; i++) {
      for (int j = 480; j < 640; j++) {
        // Pink shades: 
        // Super light pink
        //if (i%2 == 0 && j%2 == 0) display.drawPixel(i, j, GxEPD_RED);
        // Light pink
        /*if (i%3 == 0 && j%3 == 0) display.drawPixel(i, j, GxEPD_RED);
        else if ((i-1)%3 == 0 && (j-1)%3 == 0) display.drawPixel(i, j, GxEPD_RED);
        else if ((i+1)%3 == 0 && (j+1)%3 == 0) display.drawPixel(i, j, GxEPD_RED);*/
        // Darker pink
        if (i < 240) {
          if (i%2 == 0 && j%2 == 0) display.drawPixel(i, j, GxEPD_RED);
          else if (i%2 != 0 && j%2 != 0) display.drawPixel(i, j, GxEPD_RED);
        }
        else {
          if (i%2 == 0 && j%2 == 0) display.drawPixel(i, j, GxEPD_BLACK);
          else if (i%2 != 0 && j%2 != 0) display.drawPixel(i, j, GxEPD_BLACK);
        }
      }
    }
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(x, y);
    display.print(test);
    display.setCursor(xr, y);
    display.print(test);
    y += y_inc;
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(test);
    display.setCursor(xr, y);
    display.print(test);
    y += y_inc;
    display.setTextColor(GxEPD_RED);
    display.setCursor(x, y);
    display.print(test);
    display.setCursor(xr, y);
    display.print(test);
    y += y_inc;

    // FULL RED/BLACK
    for (int i = 0; i < 240; ++i) for (int j = 640; j < 800; ++j) display.drawPixel(i, j, GxEPD_RED);
    for (int i = 240; i < 480; ++i) for (int j = 640; j < 800; ++j) display.drawPixel(i, j, GxEPD_BLACK);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(x, y);
    display.print(test);
    display.setCursor(xr, y);
    display.print(test);
    y += y_inc;
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(test);
    display.setCursor(xr, y);
    display.print(test);
    y += y_inc;
    display.setTextColor(GxEPD_RED);
    display.setCursor(x, y);
    display.print(test);
    display.setCursor(xr, y);
    display.print(test);
    //display.drawBitmap(190, 350, gImage_heart, 100, 100, GxEPD_WHITE);
  }
  while (display.nextPage());
}

void loop() {};
