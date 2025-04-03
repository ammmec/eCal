// uncomment next line to use class GFX of library GFX_Root instead of Adafruit_GFX
//#include <GFX.h>

#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include "heart.h"

#define PWR  D11  // 7
#define BUSY D7   // 18
#define RST  D6   // 1
#define DC   D3   // 14
#define CS   SS   // 5

// alternately you can copy the constructor from GxEPD2_display_selection.h or GxEPD2_display_selection_added.h to here
GxEPD2_3C < GxEPD2_750c_Z08, GxEPD2_750c_Z08::HEIGHT / 4 > display(GxEPD2_750c_Z08(/*CS=*/ CS, /*DC=*/ DC, /*RST=*/ RST, /*BUSY=*/ BUSY)); // GDEW075Z08 800x480, GD7965

void setup() {
  pinMode(PWR, OUTPUT);
  digitalWrite(PWR, HIGH);
  pinMode(D13, OUTPUT);
  digitalWrite(D13, HIGH);
  //display.init(115200);
  display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  //display.clearScreen();
  Serial.println("Starting up");
  digitalWrite(D13, LOW);
  testing();
  digitalWrite(D13, HIGH);
  display.hibernate();
  delay(1000);
  Serial.println("Partial refresh");
  digitalWrite(D13, LOW);
  partial();
  digitalWrite(D13, HIGH);
  Serial.println("Going to sleep");
  display.hibernate();
  digitalWrite(PWR, LOW);
}

const char test[] = "Test Text";
uint16_t y = 10;

void writeText(uint16_t x, uint16_t w, uint16_t h) {
  uint16_t y_inc = 53;
  uint16_t xr = x + display.width()/2;

  display.setTextColor(GxEPD_BLACK);
  display.fillRect(x-2, y-12, w+4, h+4, GxEPD_WHITE);
  display.setCursor(x, y);
  display.print(test);
  display.fillRect(xr-2, y-12, w+4, h+4, GxEPD_WHITE);
  display.setCursor(xr, y);
  display.print(test);
  y += y_inc;
  display.setTextColor(GxEPD_WHITE);
  display.fillRect(x-2, y-12, w+4, h+4, GxEPD_BLACK);
  display.setCursor(x, y);
  display.print(test);
  display.fillRect(xr-2, y-12, w+4, h+4, GxEPD_BLACK);
  display.setCursor(xr, y);
  display.print(test);
  y += y_inc;
  display.setTextColor(GxEPD_RED);
  display.fillRect(x-2, y-12, w+4, h+4, GxEPD_WHITE);
  display.setCursor(x, y);
  display.print(test);
  display.fillRect(xr-2, y-12, w+4, h+4, GxEPD_WHITE);
  display.setCursor(xr, y);
  display.print(test);
  y += y_inc;
}

void partial() {
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_WHITE);
  display.firstPage();
  
  int16_t tbx, tby; uint16_t w, h;
  display.getTextBounds(test, 0, 0, &tbx, &tby, &w, &h);
  uint16_t x = ((display.width()/2 - w) / 2) - tbx + display.width()/2;
  y = ((display.height()/15 - h) / 2) - tby;

  display.getTextBounds("Hi", 0, 0, &tbx, &tby, &w, &h);
  display.setPartialWindow(x-2, y-8, w+4, h+4);
  display.setTextColor(GxEPD_WHITE);
  display.setCursor(x, y);

  Serial.println("Starting to print!");
  display.fillScreen(GxEPD_BLACK);
  do {
    display.print("Hi");
  }
  while (display.nextPage());
}

void testing()
{
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setFullWindow();
  display.firstPage();
  
  
  int16_t tbx, tby; uint16_t w, h;
  display.getTextBounds(test, 0, 0, &tbx, &tby, &w, &h);
  uint16_t x = ((display.width()/2 - w) / 2) - tbx;

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
    writeText(x, w, h);

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
    writeText(x, w, h);

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
    writeText(x, w, h);
    
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
    writeText(x, w, h);

    // FULL RED/BLACK
    for (int i = 0; i < 240; ++i) for (int j = 640; j < 800; ++j) display.drawPixel(i, j, GxEPD_RED);
    for (int i = 240; i < 480; ++i) for (int j = 640; j < 800; ++j) display.drawPixel(i, j, GxEPD_BLACK);
    writeText(x, w, h);
    //display.drawBitmap(190, 350, gImage_heart, 100, 100, GxEPD_WHITE);
  }
  while (display.nextPage());
}

void loop() {};
