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

// Rectangle (1h class) height and width
#define RECT_HEIGHT 61
#define RECT_WIDTH 200

// Character sizes (including spacing)
#define CHAR_HEIGHT 9
#define CHAR_WIDTH 11

// Margins to center the schedule
#define TOP_MARGIN 4
#define BOTTOM_MARGIN 3
#define MARGIN 5 // General margins for text and stuff

// Display size
#define DISPLAY_HEIGHT 800
#define DISPLAY_WIDTH 480

// x where the 1st column (after the hours) is. Taking into account left and right margin
#define FIRST_COLUMN 64+MARGIN*2
#define SECOND_COLUMN FIRST_COLUMN+RECT_WIDTH+MARGIN*2

// Number of classes displayed
#define NUM_CLASSES 13

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
  uint16_t x_end = x + RECT_WIDTH;
  uint16_t y_end = y + RECT_HEIGHT;
  for (int i = x; i < x_end; i++) {
    for (int j = y; j < y_end; j++) {
      // Super light pink
      if (i%2 == 0 && j%2 == 0) display.drawPixel(i, j, GxEPD_RED);
    }
  }
}

void drawClass(char position, char name[], char duration, uint16_t color) {
  uint16_t x = FIRST_COLUMN+MARGIN;
  uint16_t y = position*RECT_HEIGHT + TOP_MARGIN;
  uint16_t h = RECT_HEIGHT*duration;
  // Draw dotted pattern (background)
  for (int i = x; i < x+RECT_WIDTH; ++i) {
    for (int j = y+MARGIN; j < y+h-MARGIN; ++j) {
      if (i%3 == 0 && j%3 == 0) display.drawPixel(i, j, color);
      else if ((i-1)%3 == 0 && (j-1)%3 == 0) display.drawPixel(i, j, color);
      else if ((i+1)%3 == 0 && (j+1)%3 == 0) display.drawPixel(i, j, color);
      else if (duration > 1) display.drawPixel(i, j, GxEPD_WHITE); // To "break" horizontal lines if duration > 2
    }
  }

  // Draw a border for the rectangle
  display.drawRect(x, y+MARGIN, RECT_WIDTH, h-MARGIN*2, color);

  // Center the text
  int16_t tbx, tby; uint16_t tbw, tbh; // boundary box window
  display.getTextBounds(name, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  x = x + (RECT_WIDTH - tbw) / 2;
  y = y + (RECT_HEIGHT*duration - tbh) / 2 + CHAR_HEIGHT;

  // Add a white outline to the text to make it clear
  display.setTextColor(GxEPD_WHITE);
    for (int i = -3; i <= 3; ++i) {
      for (int j = -2; j <= 2; ++j) {
        display.setCursor(x+i, y+j);
        display.print(name);
      }
    }

    // Write the actual text
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(x, y);
    display.print(name);
}

void drawHours() {
  // Giving some margin on the left
  uint16_t x;
  // y position ->  TOP_MARGIN for the border
  //                (RECT_HEIGHT + CHAR_HEIGHT)/2 to center inside its spot
  uint16_t y = TOP_MARGIN + (RECT_HEIGHT + CHAR_HEIGHT)/2;
  char pos = 1;

  // Line to separate hours with the schedule
  display.drawFastVLine(FIRST_COLUMN, TOP_MARGIN, DISPLAY_HEIGHT-BOTTOM_MARGIN, GxEPD_BLACK);
  for (char i = 8; i < 21; ++i) {
    x = MARGIN;
    char hours[6] = {' ', ' ', ' ', ' ', ' ', 'h'};
    display.setCursor(x, y);
    if (i > 9) { // start and end > 9
      // Start time
      hours[0] = '0'+(i)/10;    hours[1] = '0'+i%10;
      hours[2] = '-';
      // End time
      hours[3] = '0'+(i+1)/10;  hours[4] = '0'+(i+1)%10;
      // Draw a line to limit the hour
      display.drawFastHLine(0, TOP_MARGIN+RECT_HEIGHT*pos++, FIRST_COLUMN+RECT_WIDTH+MARGIN*2, GxEPD_BLACK);
    }
    else if ((i+1) > 9) { // end > 9
      // Start time
      hours[0] = '0'+i;
      hours[1] = '-';
      // End time
      hours[2] = '0'+(i+1)/10;  hours[3] = '0'+(i+1)%10;
      hours[4] = 'h'; hours[5] = ' ';
      // Draw a line to limit the hour
      display.drawFastHLine(0, TOP_MARGIN+RECT_HEIGHT*pos++, FIRST_COLUMN+RECT_WIDTH+MARGIN, GxEPD_BLACK);
    }
    else { // start and end <= 9
      // Start time
      hours[0] = '0'+i;
      hours[1] = '-';
      // End time
      hours[2] = '0'+(i+1);
      hours[3] = 'h'; hours[4] = ' '; hours[5] = ' ';
      }
    display.print(hours);
    y += RECT_HEIGHT;
  }
}

void drawClasses(char classes[][128]) {
  bool colorRed = true;
  for (int i = 0; i < NUM_CLASSES; ++i) {
    if (strcmp(classes[i], "") != 0) {
      char duration = 1;
      // If the next hour is reserved for the same class
      if (i+1 < NUM_CLASSES && strcmp(classes[i], classes[i+1]) == 0) {
        char j = i;
        // Check how many hours of class there are
        while (j < NUM_CLASSES && strcmp(classes[j], classes[j+1]) == 0) {
          ++duration;
          ++j;
        }
        drawClass(i, classes[i], duration, colorRed ? GxEPD_RED : GxEPD_BLACK);
        i = j;
      }
      else {
        drawClass(i, classes[i], duration, colorRed ? GxEPD_RED : GxEPD_BLACK);
      }
      colorRed = !colorRed;
    }
  }
  display.drawFastVLine(SECOND_COLUMN, TOP_MARGIN, DISPLAY_HEIGHT-BOTTOM_MARGIN, GxEPD_BLACK);
  /*drawClass(0, "FM 10 T", 1, GxEPD_RED);
  drawClass(2, "SO2 10 T", 2, GxEPD_BLACK);
  drawClass(5, "PROP 15 L", 1, GxEPD_RED);*/
}

void drawQR() {
  uint16_t qr_width = 125;
  uint16_t qr_height = 125;
  uint16_t qr_x = (DISPLAY_WIDTH + SECOND_COLUMN - qr_width)/2;
  uint16_t qr_y = DISPLAY_HEIGHT - qr_height - BOTTOM_MARGIN - 5;

  char text[] = "Horari sencer:";
  int16_t tbx, tby; uint16_t tbw, tbh; // boundary box window
  display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t text_x = (DISPLAY_WIDTH + SECOND_COLUMN - tbw)/ 2;
  uint16_t text_y = qr_y - tbh;
  
  display.drawFastHLine(SECOND_COLUMN, text_y-CHAR_HEIGHT-MARGIN, DISPLAY_WIDTH-SECOND_COLUMN, GxEPD_BLACK);
  display.setCursor(text_x, text_y);
  display.print(text);
  display.drawBitmap(qr_x, qr_y, qr, qr_width, qr_height, GxEPD_BLACK);
}

void writeText() {
  display.setRotation(1);
  display.firstPage();
  display.setFullWindow();
  display.setFont(&FreeMonoBold9pt7b);


  Serial.println("Starting to print!");
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    //display.writeFastVLine (300, 0, 800, GxEPD_BLACK);
    //display.drawFastHLine (300, 100, 180, GxEPD_BLACK);
    //drawRectPink(55, 5, 220, 70);

    drawHours();


    char classes[NUM_CLASSES][128] = {
    "", "GEI FM 10T", "GEI LP 10T", "GEI LP 10T", "GEI IA 10T", "GEI IA 10T", "GEI FM 20T",
    "", "", "GEI PRO1 20T", "", "", ""};
    drawClasses(classes);

    drawQR();
  }
  while (display.nextPage());
}

void loop() {};
