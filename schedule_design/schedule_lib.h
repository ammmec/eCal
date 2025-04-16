#ifndef SCHEDULE_LIB_H
#define SCHEDULE_LIB_H
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include "schedule_bitmaps.h"
#include <WiFi.h>
#include <time.h>
#include "wifi_secure.h"

// Pin definition
#define PWR  D11  // 7
#define BUSY D7   // 18
#define RST  D6   // 1
#define DC   D3   // 14
#define CS   SS   // 5

// Display size
extern uint16_t displayHeight;
extern uint16_t displayWidth;

#define MARGIN 5 // Margins for text and lines

// Character sizes (including spacing)
#define CHAR_HEIGHT 9
#define CHAR_WIDTH 11

// Schedule related information
#define START_HOUR 8 // First hour of the day
#define LAST_HOUR 20
#define CLOSED  "TANCAT"
#define NUM_CLASSES 13

// Bitmap sizes
#define CLASS_BP_WIDTH  24
#define CLASS_BP_HEIGHT 6
#define QR_SIZE 125

// Different layouts
enum Layout {
  DEFAULT_LAYOUT,
  VERBOSE_LAYOUT,
  HORIZONTAL_LAYOUT,
  SIMPLE_LAYOUT
};

// Layout structure
struct LayoutConfig {
  bool showLines;
  bool showQR;
  bool showAnnouncements;
  bool showCurrNext;
  bool saveEnergy; // Schedule will not be refreshed unless it changes (does not show the current class in red)
  bool staticSchedule; // if false, the top of the schedule will be the current class. Otherwise, it has morning and afternoon schedule

  char numClassesDisplayed; // How many classes are displayed in the schedule

  // Size of the class rectangle (1h)
  uint16_t classHeight;
  uint16_t classWidth;

  // Margins of the display, to center schedule
  uint16_t topMargin;
  uint16_t bottomMargin;

  // Columns created by schedule, to position elements
  uint16_t hourLine = CHAR_WIDTH*6+MARGIN*2; // Max size the hours take (ex: 20-21h)
  uint16_t endClassLine; // x position where the class rectangle ends
  uint16_t startAnnouncementsLine; // x position where announcements can start

  // Rows created to separate different elements
  uint16_t rows[2];
};

// Globals
extern LayoutConfig config;

extern uint16_t currentHour;
extern char curr_class_pos; // Index of current class in class array

// Bitmaps
extern const unsigned char class_bg[];
extern const unsigned char current_class_bg[];
extern const unsigned char qr[];
extern const unsigned char avisos_bw[];
extern const unsigned char avisos_red[];

// Function declarations
void setupLayout(Layout l = DEFAULT_LAYOUT, bool lines = true, bool saveEnergy = false, bool staticSchedule = false);
void setLines(bool lines);
void setNumClassesDisplayed(char nClasses);
bool isScheduleChanged(char classes[][32], int16_t durations[], char announcements[], char startHour);
void drawSchedule(char classes[][32], int16_t durations[], char announcements[]);
void drawPicture(bool portrait, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const unsigned char picture_bw[], const unsigned char picture_red[]);
void updateCurrentHour(char classes[][32], int16_t durations[]);
void drawOutline(uint16_t x, uint16_t y, uint16_t w, uint16_t h, char text[], uint16_t color);
void drawClass(char position, char name[], char duration, bool currentClass);
void drawHours(char start);
void drawClasses(char classes[][32], int16_t durations[], char start);
void drawQR();
void drawCurrentNextClass(char classes[][32], int16_t durations[]);
void drawAnnouncements(char announcement[]);
void printWithLineBreaks(const char* text, uint16_t x, uint16_t y, uint8_t maxCharsPerLine);
#endif // SCHEDULE_LIB_H