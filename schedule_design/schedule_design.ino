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
  display.init(115200, true, 2, false); // For Waveshare with "clever" reset circuit
  Serial.println("Starting up");
  drawSchedule();
  Serial.println("Going to sleep");
  display.powerOff();
  digitalWrite(PWR, LOW);
}

// Draws a class of the schedule. The height of the rectangle increases with the duration
// position <- [0, 12], which "timeslot" it is in
// name <- name of the class
// duration <- how many hours the class lasts
// color <- if the rectangle should be red or black. It alternates between classes
void drawClass(char position, char name[], char duration, uint16_t color) {
  uint16_t x = FIRST_COLUMN + MARGIN;
  uint16_t y = position * RECT_HEIGHT + TOP_MARGIN;
  uint16_t h = RECT_HEIGHT * duration;

  // Center text
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(name, 0, 0, &tbx, &tby, &tbw, &tbh);

  uint16_t tx = x + (RECT_WIDTH - tbw) / 2;
  uint16_t ty = y + (h - tbh) / 2 + CHAR_HEIGHT;

  // Draw dotted background manually
  /*for (int i = x; i < x + RECT_WIDTH; ++i) {
    for (int j = y + MARGIN; j < y + h - MARGIN; ++j) {
      if (( i % 3 == 0 && j % 3 == 0)
            || ((i - 1) % 3 == 0 && (j - 1) % 3 == 0)
            || ((i + 1) % 3 == 0 && (j + 1) % 3 == 0)) display.drawPixel(i, j, color);
      else if (duration > 1) display.drawPixel(i, j, GxEPD_WHITE);
    }
  }*/

  // Draw dotted background in bitmap
  display.drawBitmap(x, y+MARGIN, class_bg_full, RECT_WIDTH, RECT_HEIGHT-2*MARGIN, color);
  // Extra bitmap to take the gap into account and the longer class
  for (int i = 1; i < duration; ++i) {
    display.drawFastHLine(x, y+i*RECT_HEIGHT, RECT_WIDTH, GxEPD_WHITE);
    display.drawBitmap(x, y+i*RECT_HEIGHT-MARGIN, class_bg_full, RECT_WIDTH, RECT_HEIGHT, color);
  }

  // Border
  display.drawRect(x, y + MARGIN, RECT_WIDTH, h - MARGIN * 2, color);

  // White outline
  display.setTextColor(GxEPD_WHITE);
  for (int i = -3; i <= 3; ++i) {
    for (int j = -2; j <= 2; ++j) {
      if (i == 0 && j == 0) continue;
      display.setCursor(tx + i, ty + j);
      display.print(name);
    }
  }

  // Actual text
  display.setTextColor(GxEPD_BLACK);
  display.setCursor(tx, ty);
  display.print(name);
}

// Draws, on the left of the screen, the hours of the schedule
void drawHours() {
  // Giving some margin on the left
  uint16_t x = MARGIN;
  // y position ->  TOP_MARGIN for the border
  //                (RECT_HEIGHT + CHAR_HEIGHT)/2 to center inside its spot
  uint16_t y = TOP_MARGIN + (RECT_HEIGHT + CHAR_HEIGHT) / 2;
  char pos = 1;

  // Line to separate hours with the schedule
  display.drawFastVLine(FIRST_COLUMN, TOP_MARGIN, DISPLAY_HEIGHT - BOTTOM_MARGIN, GxEPD_BLACK);

  for (char i = 8; i < 21; ++i) {
    char hours[7] = "      ";  // 6 chars + null

    // Format hour string
    snprintf(hours, sizeof(hours), "%d-%dh", i, i + 1);

    display.setCursor(x, y);
    display.print(hours);

    if (i != 20) display.drawFastHLine(0, TOP_MARGIN + RECT_HEIGHT * pos++, FIRST_COLUMN + RECT_WIDTH + MARGIN * 2, GxEPD_BLACK);
    y += RECT_HEIGHT;
  }
}

// Given an array of strings, where the string is the class and the position is the hour it is done in,
// print each single class in the schedule
void drawClasses(char classes[][128]) {
  bool colorRed = true;

  for (int i = 0; i < NUM_CLASSES; ++i) {
    if (strcmp(classes[i], "") == 0) continue; // If the classroom is empty, continue

    char duration = 1;
    int j = i;

    // Check consecutive hours
    while (j + 1 < NUM_CLASSES && strcmp(classes[j], classes[j + 1]) == 0) {
      ++duration;
      ++j;
    }

    drawClass(i, classes[i], duration, colorRed ? GxEPD_RED : GxEPD_BLACK);
    i = j;
    colorRed = !colorRed;
  }

  display.drawFastVLine(SECOND_COLUMN, TOP_MARGIN, DISPLAY_HEIGHT - BOTTOM_MARGIN, GxEPD_BLACK);
}

// Draws a QR code of the website where the full schedule can be found
void drawQR() {
  const uint16_t qr_width = 125;
  const uint16_t qr_height = 125;

  uint16_t qr_x = (DISPLAY_WIDTH + SECOND_COLUMN - qr_width) / 2;
  uint16_t qr_y = DISPLAY_HEIGHT - qr_height - BOTTOM_MARGIN - 5;

  const char text[] = "Horari sencer:";
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);

  uint16_t text_x = (DISPLAY_WIDTH + SECOND_COLUMN - tbw) / 2;
  uint16_t text_y = qr_y - tbh;

  display.drawFastHLine(SECOND_COLUMN, text_y - CHAR_HEIGHT - MARGIN, DISPLAY_WIDTH - SECOND_COLUMN, GxEPD_BLACK);
  display.setCursor(text_x, text_y);
  display.print(text);

  display.drawBitmap(qr_x, qr_y, qr, qr_width, qr_height, GxEPD_BLACK);
}

void drawSchedule() {
  display.setRotation(1);
  display.firstPage();
  display.setFullWindow();
  display.setFont(&FreeMonoBold9pt7b);

  Serial.println("Starting to print!");
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);

    drawHours();

    char classes[NUM_CLASSES][128] = {
      "", "GEI FM 10T", "GEI LP 10T", "GEI LP 10T", "GEI IA 10T", "GEI IA 10T", "GEI IA 10T",
      "GEI FM 20T", "", "GEI PRO1 20T", "", "", ""
    };
    drawClasses(classes);

    drawQR();
  } while (display.nextPage());
}

void loop() {}
