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
extern uint16_t display_height;
extern uint16_t display_width;

#define MARGIN 5 // Margins for text and lines
#define HOUR_HEIGHT 61 // Maximum height for an hour slot to fit all classes

// Character sizes (including spacing)
#define CHAR_HEIGHT 9
#define CHAR_WIDTH 11

// Class related information
#define START_HOUR 8 // First hour of the day
#define LAST_HOUR 20
#define CLOSED  "TANCAT"
#define NUM_CLASSES 13

// Bitmap sizes
#define CLASS_BP_WIDTH  24
#define CLASS_BP_HEIGHT 6
#define QR_SIZE 125

enum Layout {
  DEFAULT_LAYOUT,
  VERBOSE_LAYOUT,
  HORIZONTAL_LAYOUT,
  SIMPLE_LAYOUT
};

// Layout structure (default)
struct LayoutConfig {
  bool showLines;
  bool showQR;
  bool showAnnouncements;
  bool showCurrNext;
  bool saveEnergy;
  bool staticSchedule;

  char numClassesDisplayed;
  // Rectangle (1h class) height and width
  uint16_t classHeight; // Has to be divisible by 60
  uint16_t classWidth;  // Has to be divisible by 25

  // margins of the display
  uint16_t topMargin;
  uint16_t bottomMargin;

  uint16_t hourLine;
  uint16_t endClassLine;
  uint16_t startAnnouncementsLine;

  uint16_t rows[2];
};

// Globals
extern LayoutConfig config;

extern char prevAnnouncements[256];
extern char prevAnnouncements[256];
extern char prevClasses[NUM_CLASSES][128];
extern int16_t prevDurations[NUM_CLASSES];
extern char prevStartHour;

extern uint16_t currentHour;
extern char curr_class_pos;

// Bitmaps
extern const unsigned char class_bg[];
extern const unsigned char qr[];
extern const unsigned char avisos_bw[];
extern const unsigned char avisos_red[];

// Function declarations
void setupLayout(Layout l = DEFAULT_LAYOUT, bool lines = true, bool saveEnergy = false, bool staticSchedule = false);
void setLines(bool lines);
void setNumClassesDisplayed(char nClasses);
bool isScheduleChanged(char classes[][128], int16_t durations[], char announcements[], char startHour);
void drawSchedule(char classes[][128], int16_t durations[], char announcements[]);
void drawPicture(bool portrait, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const unsigned char picture_bw[], const unsigned char picture_red[]);
void updateCurrentHour(char classes[][128], int16_t durations[]);
void drawOutline(uint16_t x, uint16_t y, uint16_t w, uint16_t h, char text[], uint16_t color);
void drawClass(char position, char name[], char duration, uint16_t color);
void drawHours(char start);
void drawClasses(char classes[][128], int16_t durations[], char start);
void drawQR();
void drawCurrentNextClass(char classes[][128], int16_t durations[]);
void drawAnnouncements(char announcement[]);
void printWithLineBreaks(const char* text, uint16_t x, uint16_t y, uint8_t maxCharsPerLine);
#endif // SCHEDULE_LIB_H