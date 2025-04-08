#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include "schedule.h"

#include <WiFi.h>
#include <time.h>
#include "wifi_secure.h"

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
#define START_HOUR 8

uint16_t first_row; // To know where the height of the end of "current and next class"
uint16_t second_row;

char curr_class_pos;

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
void drawClass(char position, char name[], char duration) {
  uint16_t x = FIRST_COLUMN + MARGIN;
  uint16_t y = position * RECT_HEIGHT + TOP_MARGIN;
  uint16_t h = RECT_HEIGHT * duration;
  uint16_t color = (position == curr_class_pos) ? GxEPD_RED : GxEPD_BLACK;

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
  display.drawBitmap(x, y+MARGIN, class_bg, RECT_WIDTH, RECT_HEIGHT-2*MARGIN, color);
  // Extra bitmap to take the gap into account and the longer class
  for (int i = 1; i < duration; ++i) {
    display.drawFastHLine(x, y+i*RECT_HEIGHT, RECT_WIDTH, GxEPD_WHITE);
    display.drawBitmap(x, y+i*RECT_HEIGHT-MARGIN, class_bg, RECT_WIDTH, RECT_HEIGHT, color);
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
void drawClasses(char classes[][128], char durations[]) {

  char i = 0;
  while (i < NUM_CLASSES) {
    if (durations[i] == 0) {
      ++i;
      continue; // If the classroom is empty, continue
    }

    char duration = durations[i];

    drawClass(i, classes[i], duration);
    i += duration;
  }

  display.drawFastVLine(SECOND_COLUMN, TOP_MARGIN, DISPLAY_HEIGHT - BOTTOM_MARGIN, GxEPD_BLACK);
}

// Draws a QR code of the website where the full schedule can be found
void drawQR() {
  const uint16_t qr_width = 125;
  const uint16_t qr_height = 125;

  uint16_t qr_x = (DISPLAY_WIDTH + SECOND_COLUMN - qr_width) / 2;
  uint16_t qr_y = DISPLAY_HEIGHT - qr_height - BOTTOM_MARGIN;

  const char text[] = "Horari sencer:";
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);

  uint16_t text_x = (DISPLAY_WIDTH + SECOND_COLUMN - tbw) / 2;
  uint16_t text_y = qr_y - tbh;

  second_row = text_y - CHAR_HEIGHT - MARGIN*2;
  display.drawFastHLine(SECOND_COLUMN, text_y - CHAR_HEIGHT - MARGIN*2, DISPLAY_WIDTH - SECOND_COLUMN, GxEPD_BLACK);
  display.setCursor(text_x, text_y);
  display.print(text);

  display.drawBitmap(qr_x, qr_y, qr, qr_width, qr_height, GxEPD_BLACK);
}

void drawCurrentNextClass(uint16_t current_hour, char classes[][128], char durations[]) {
  char currClass[64];
  char nextClass[64];
  uint16_t ind_curr_start = current_hour-START_HOUR;
  uint16_t ind_next_start;
  if (strcmp(classes[ind_curr_start], "") != 0) {
    snprintf(currClass, sizeof(currClass), classes[ind_curr_start]);
    while (durations[ind_curr_start]==0) --ind_curr_start;
    curr_class_pos = ind_curr_start;
  }
  else {
    snprintf(currClass, sizeof(currClass), "-");
    curr_class_pos = -1;
  }

  char i = ind_curr_start + durations[current_hour];
  while (i < NUM_CLASSES) {
    if (strcmp(classes[i], currClass) != 0 && durations[i] != 0) {
      snprintf(nextClass, sizeof(nextClass), classes[i]);
      ind_next_start = i;
      i = NUM_CLASSES;
    }
    ++i;
    Serial.println(i-'0');
  }
  if (i == NUM_CLASSES) snprintf(nextClass, sizeof(nextClass), "-");

  char text[] = "Classe en curs:";
  uint16_t x = SECOND_COLUMN + 2*MARGIN;
  uint16_t y = MARGIN+CHAR_HEIGHT+TOP_MARGIN;
  display.setCursor(x, y);
  display.print("Classe en curs:");

  display.setCursor(x, y+(CHAR_HEIGHT+MARGIN*2)*4);
  display.print("Propera classe:");

  y += CHAR_HEIGHT+MARGIN*2;

  display.setFont(&FreeMono9pt7b);
  display.setCursor(x, y);
  y += CHAR_HEIGHT+MARGIN*2;
  display.print(currClass);
  display.setCursor(x, y);
  y += CHAR_HEIGHT+MARGIN*2;
  snprintf(currClass, sizeof(currClass), "%d:00-%d:00", START_HOUR+ind_curr_start, START_HOUR+ind_curr_start+durations[ind_curr_start]);
  display.print(currClass);

  y += (CHAR_HEIGHT+MARGIN*2)*2;
  display.setCursor(x, y);
  y += CHAR_HEIGHT+MARGIN*2;
  display.print(nextClass);
  display.setCursor(x, y);
  y += CHAR_HEIGHT+MARGIN*2;
  snprintf(nextClass, sizeof(nextClass), "%d:00-%d:00", START_HOUR+ind_next_start, START_HOUR+ind_next_start+durations[ind_next_start]);
  display.print(nextClass);

  display.setFont(&FreeMonoBold9pt7b);
  first_row = y;
  display.drawFastHLine(SECOND_COLUMN, first_row, DISPLAY_WIDTH - SECOND_COLUMN, GxEPD_BLACK);
}

void printWithLineBreaks(const char* text, uint16_t x, uint16_t y, uint8_t maxCharsPerLine) {
  int lineHeight = CHAR_HEIGHT + MARGIN;
  int currentY = y;
  const char* lineStart = text;
  int lineLength = 0;

  const char* wordStart;
  int wordLength;

  while (*text) {
    // Find the next word
    wordStart = text;
    while (*text && *text != ' ' && *text != '\n') {
      text++;
    }
    wordLength = text - wordStart;

    // If adding this word exceeds max length, print current line and start new one
    if (lineLength > 0 && (lineLength + wordLength + 1) > maxCharsPerLine) {
      display.setCursor(x, currentY);
      display.print(String(lineStart, lineLength));  // Print the current line
      currentY += lineHeight;  // Move to the next line

      // Reset for the new line
      lineStart = wordStart;
      lineLength = wordLength;
    } else {
      // Add word to the line
      if (lineLength > 0) {
        lineLength++; // Add space
      }
      lineLength += wordLength;
    }

    // Handle newline
    if (*text == '\n') {
      display.setCursor(x, currentY);
      display.print(String(lineStart, lineLength));  // Print the current line
      currentY += lineHeight;

      // Reset for the next line
      text++;
      lineStart = text;
      lineLength = 0;
    }

    // Skip spaces
    if (*text == ' ') {
      text++;
    }
  }

  // Print any leftover text in the current line
  if (lineLength > 0) {
    display.setCursor(x, currentY);
    display.print(String(lineStart, lineLength));
  }
}

void drawAnnouncements(char announcement[]) {
  uint16_t x = SECOND_COLUMN + MARGIN*2;
  uint16_t y = first_row+MARGIN;
  char icon_w = 15;
  char icon_h = 15;
  display.drawBitmap(x, y,  avisos_bw, icon_w, icon_h, GxEPD_BLACK);
  display.drawBitmap(x, y, avisos_red, icon_w, icon_h, GxEPD_RED);
  display.setCursor(x+icon_w+MARGIN, y+icon_h);
  display.print("Avisos:");

  y+= icon_h+CHAR_HEIGHT+MARGIN*2;
  display.setFont(&FreeMono9pt7b);

  if (strcmp(announcement, "") == 0) {
    char text[] = "No hi ha cap avís";
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);
    tbx = (DISPLAY_WIDTH+SECOND_COLUMN-tbw)/2;
    tby = ((second_row-first_row))/2 + y;

    display.setCursor(tbx, tby);
    display.print(text);
  }
  else {
    printWithLineBreaks(announcement, x, y, (DISPLAY_WIDTH-(SECOND_COLUMN+MARGIN*4))/CHAR_WIDTH);
  }

  display.setFont(&FreeMonoBold9pt7b);
}

void drawSchedule() {
  display.setRotation(1);
  display.firstPage();
  display.setFullWindow();
  display.setFont(&FreeMonoBold9pt7b);


  // Get current time (to tell what class is next)
  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  /*WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  
  // Init and get the time
  configTime(3600, 3600, "pool.ntp.org");
  uint16_t hour;

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    hour = timeinfo.tm_hour;
    uint16_t minute = timeinfo.tm_min;

    if (minute >= 50) {
      hour = (hour + 1) % 24; // Wrap around to 0 if it's 23:50+
    }
  } else {
    Serial.println("Failed to get time");
  }
  
  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  */

  Serial.println("Starting to print!");
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);

    drawHours();

    char classes[NUM_CLASSES][128] = {
      "", "GEI FM 10T", "GEI LP 10T", "GEI LP 10T", "GEI IA 10T", "GEI IA 10T", "GEI IA 10T",
      "GEI FM 20T", "", "GEI PRO1 20T", "", "", ""
    };
    char durations[NUM_CLASSES] = {
      0, 1, 2, 0, 3, 0, 0,
      1, 0, 1, 0, 0, 0
    };
    drawCurrentNextClass(12, classes, durations); // TODO: change number to "hour"
    drawClasses(classes, durations);

    drawAnnouncements("Aquest es un missatge molt llarg que s'haura de tallar en mes d'un!");

    drawQR();
  } while (display.nextPage());
}

void loop() {}
