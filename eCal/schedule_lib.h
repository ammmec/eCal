#ifndef SCHEDULE_LIB_H
#define SCHEDULE_LIB_H
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include "schedule_bitmaps.h"
#include <WiFi.h>
#include <time.h>
#include "wifi_secure.h"

#define DEBUG true

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
  MINIMALIST_LAYOUT,
  SIMPLE_LAYOUT,
  VERBOSE_LAYOUT,
  HORIZONTAL_LAYOUT,
  PICTURE_LAYOUT
};

// Types of change
enum change_t {
  NONE,
  GENERAL,
  ADD,
  CANCELLED
};

// Easily keep track of the days of the week
enum WEEKDAYS {
  SUNDAY, // = 0
  MONDAY,
  TUESDAY,
  WEDNESDAY,
  THURSDAY,
  FRIDAY,
  SATURDAY
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

  char name[3];
};

// Globals
RTC_DATA_ATTR extern LayoutConfig config;
RTC_DATA_ATTR extern uint16_t rawConfig;

RTC_DATA_ATTR extern uint16_t currentHour;
extern WEEKDAYS weekday;
extern char curr_class_pos; // Index of current class in class array

#define SIZE_ANNOUNCEMENTS 256U
RTC_DATA_ATTR extern bool needRefresh;
RTC_DATA_ATTR extern char announcements[SIZE_ANNOUNCEMENTS];
RTC_DATA_ATTR extern char classes[NUM_CLASSES][32];
RTC_DATA_ATTR extern int16_t durations[NUM_CLASSES];
RTC_DATA_ATTR extern change_t changed[NUM_CLASSES];

// Variables to check if the schedule needs to be refreshed
RTC_DATA_ATTR extern char prevStartHour;

// Bitmaps
extern const unsigned char class_bg[];
extern const unsigned char current_class_bg[];
extern const unsigned char qr[];
extern const unsigned char avisos_bw[];
extern const unsigned char avisos_red[];

// Function declarations
bool connectWiFi(uint8_t &minutesTilNextHour);
bool connectWiFi();
void disconnectWiFi();
void restartData();
void setupLayout(Layout l = DEFAULT_LAYOUT, bool lines = true, bool saveEnergy = false, bool staticSchedule = false);
void setLines(bool lines);
void setNumClassesDisplayed(char nClasses);
void drawNoSchedule();
void drawSchedule(char classes[][32], int16_t durations[], char announcements[], change_t changed[]);
void drawPicture(bool portrait, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const unsigned char picture_bw[], const unsigned char picture_red[]);
uint8_t updateCurrentHour();
void findCurrentPos();
void drawOutline(uint16_t x, uint16_t y, uint16_t w, uint16_t h, char text[], uint16_t color);
void drawRectangle(uint16_t h, uint16_t x, uint16_t y, uint16_t color, const unsigned char bg[]);
void drawClass(char position, char name[], char duration, bool currentClass, change_t changedStatus);
void drawHours(char start);
void drawClasses(char classes[][32], int16_t durations[], char start, change_t changed[], char announcements[]);
void drawQR();
void drawLogo(uint16_t x, uint16_t y);
void drawNoWiFi();
void drawCurrentNextClass(char classes[][32], int16_t durations[]);
void drawAnnouncements(char announcement[]);
void printWithLineBreaks(const char* text, uint16_t x, uint16_t y, uint8_t maxCharsPerLine);
void clearScreen();
#endif // SCHEDULE_LIB_H