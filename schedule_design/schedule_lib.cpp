#include "schedule_lib.h"

// Display constructor
GxEPD2_3C < GxEPD2_750c_Z08, GxEPD2_750c_Z08::HEIGHT / 4 > display(GxEPD2_750c_Z08(/*CS=*/ CS, /*DC=*/ DC, /*RST=*/ RST, /*BUSY=*/ BUSY)); // GDEW075Z08 800x480, GD7965

uint16_t displayHeight;
uint16_t displayWidth;
LayoutConfig config;
uint16_t currentHour = 8;
char curr_class_pos;

// Variables to check if the schedule needs to be refreshed
char prevAnnouncements[256];
char prevClasses[NUM_CLASSES][32];
int16_t prevDurations[NUM_CLASSES];
char prevStartHour;

// Must be called before drawing schedule for the first time. Sets up the indicated schedule
void setupLayout(Layout layout, bool lines, bool saveEnergy, bool staticSchedule) {
  display.setRotation(layout == HORIZONTAL_LAYOUT ? 0 : 1);
  displayHeight = display.height();
  displayWidth = display.width();
  switch (layout) {
    case DEFAULT_LAYOUT:
      config = {
        .showLines = lines,
        .showQR = true,
        .showAnnouncements = true,
        .showCurrNext = false,
        .saveEnergy = saveEnergy,
        .staticSchedule = saveEnergy || staticSchedule, // If energy saver is on, the schedule will be static to save energy

        .numClassesDisplayed = 7,

        .topMargin = 0,
        .bottomMargin = 0,
      
        .endClassLine = displayWidth,
        .startAnnouncementsLine = 0,

        .name = "df"
      };
      config.classWidth =  (displayWidth - config.hourLine) - (displayWidth - config.hourLine)%CLASS_BP_WIDTH; // Take all usable space
      config.classHeight = ((displayHeight/2)/config.numClassesDisplayed); // Take half the screen
      config.rows[0] = config.classHeight*config.numClassesDisplayed + MARGIN*3;
      break;

    case VERBOSE_LAYOUT:
      config = {
        .showLines = lines,
        .showQR = true,
        .showAnnouncements = true,
        .showCurrNext = true,
        .saveEnergy = false, // It has to update the "current" and "next" class, can't be energy efficient
        .staticSchedule = saveEnergy || staticSchedule,

        .numClassesDisplayed = 13,
        
        .classWidth = (uint16_t)(CLASS_BP_WIDTH*(7.5)) - (uint16_t)(CLASS_BP_WIDTH*(7.5))%CLASS_BP_WIDTH,

        .name = "vb"
      };
      config.classHeight = (displayHeight/config.numClassesDisplayed);
      config.bottomMargin = (displayHeight-(config.classHeight*config.numClassesDisplayed))/2;
      config.topMargin = config.bottomMargin + ((displayHeight-config.classHeight*config.numClassesDisplayed) % 2 != 0);
      config.endClassLine = config.startAnnouncementsLine = config.hourLine+config.classWidth+MARGIN*2;
      config.rows[0] = (CHAR_HEIGHT+MARGIN*2)*7;
      config.rows[1] = displayHeight - (QR_SIZE + (CHAR_HEIGHT + MARGIN)*2);
      break;

    case SIMPLE_LAYOUT:
      config = {
        .showLines = lines,
        .showQR = false,
        .showAnnouncements = false,
        .showCurrNext = false,
        .saveEnergy = saveEnergy,
        .staticSchedule = saveEnergy || staticSchedule, // If energy saver is on, the schedule will be static to save energy

        .numClassesDisplayed = 13,

        .endClassLine = displayWidth,
        .startAnnouncementsLine = 0,

        .name = "sm"
      };
      config.classHeight = (displayHeight/config.numClassesDisplayed);
      config.classWidth =  (displayWidth - config.hourLine) - (displayWidth - config.hourLine)%CLASS_BP_WIDTH;
      config.bottomMargin = (displayHeight-(config.classHeight*config.numClassesDisplayed))/2;
      config.topMargin = config.bottomMargin + ((displayHeight-config.classHeight*config.numClassesDisplayed) % 2 != 0);
      break;

      case SPACEY_LAYOUT:
      config = {
        .showLines = lines,
        .showQR = false,
        .showAnnouncements = false,
        .showCurrNext = false,
        .saveEnergy = saveEnergy,
        .staticSchedule = saveEnergy || staticSchedule, // If energy saver is on, the schedule will be static to save energy

        .numClassesDisplayed = 13,

        .endClassLine = displayWidth,
        .startAnnouncementsLine = 0,

        .name = "sp"
      };
      config.classHeight = (displayHeight/config.numClassesDisplayed);
      config.classWidth =  (displayWidth - config.hourLine) - (displayWidth - config.hourLine)%CLASS_BP_WIDTH;
      config.bottomMargin = (displayHeight-(config.classHeight*config.numClassesDisplayed))/2;
      config.topMargin = config.bottomMargin + ((displayHeight-config.classHeight*config.numClassesDisplayed) % 2 != 0);
      config.rows[0] = config.classHeight*(uint16_t)((config.numClassesDisplayed+1)/2) + MARGIN*3;
      break;

    case HORIZONTAL_LAYOUT:
      display.setRotation(0);
      displayHeight = display.height();
      displayWidth = display.width();

      config = {
        .showLines = lines,
        .showQR = true,
        .showAnnouncements = true,
        .showCurrNext = false,
        .saveEnergy = saveEnergy,
        .staticSchedule = staticSchedule,

        .numClassesDisplayed = 7,

        .rows = {0, 0},

        .name = "hz"
      };
      config.classHeight = (displayHeight/config.numClassesDisplayed);
      config.bottomMargin = (displayHeight-(config.classHeight*config.numClassesDisplayed))/2;
      config.topMargin = config.bottomMargin + ((displayHeight-config.classHeight*config.numClassesDisplayed) % 2 != 0);
      uint16_t usableWidth = (displayWidth / 2) - config.hourLine;
      config.classWidth = usableWidth - (usableWidth % CLASS_BP_WIDTH);
      config.endClassLine = config.startAnnouncementsLine = config.hourLine+config.classWidth+MARGIN*2;
      config.rows[1] = displayHeight - (QR_SIZE + (CHAR_HEIGHT + MARGIN)*2);
      break;
  }
}

// Turns painting of lines on or off
void setLines(bool lines) {
  config.showLines = lines;
}

// Draws a "color" outline for the text to be visible
void drawOutline(uint16_t x, uint16_t y, uint16_t w, uint16_t h, char text[], uint16_t color) {
  display.setTextColor(color);
  for (int i = -w; i <= w; ++i) {
    for (int j = -h; j <= h; ++j) {
      if (i == 0 && j == 0) continue;
      display.setCursor(x + i, y + j);
      display.print(text);
    }
  }
}

/* Draws a class of the schedule. The height of the rectangle increases with the duration
    position  <- [0, 12], which "timeslot" it is in
    name      <- name of the class
    duration  <- how many hours the class lasts
    color     <- color of the class (red if it is the active one)
*/
void drawClass(char position, char name[], char duration, bool currentClass) {
  uint16_t x = (config.endClassLine - config.hourLine - config.classWidth)/2 + config.hourLine;
  uint16_t h = (config.classHeight * duration - 2*MARGIN) - (config.classHeight * duration - 2*MARGIN)%CLASS_BP_HEIGHT;
  uint16_t y = config.classHeight*position + (config.classHeight*duration)/2 + config.topMargin - h/2;

  // Center text
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(name, 0, 0, &tbx, &tby, &tbw, &tbh);

  uint16_t tx = x + (config.classWidth - tbw) / 2;
  uint16_t ty = y + (h - tbh) / 2 + CHAR_HEIGHT;

  if(strcmp(name, CLOSED) != 0) { // If the class is closed no need to draw the bitmap
    if (config.saveEnergy || !currentClass) {
      // Draw dotted background in bitmap
      for (int i = 0; i < config.classWidth; i += CLASS_BP_WIDTH) { // Repeat for as many times as needed for intended width
        for (int j = 0; j < h; j += CLASS_BP_HEIGHT) {
          display.drawBitmap(x+i, y+j, class_bg, CLASS_BP_WIDTH, CLASS_BP_HEIGHT, GxEPD_BLACK, GxEPD_WHITE);
        }
      }
      // Border
      display.drawRect(x, y, config.classWidth, h, GxEPD_BLACK);
    }
    else {
      // Border
      display.fillRect(x-2, y-2, config.classWidth+4, h+4, GxEPD_BLACK); // Thicker border
      // Draw dotted background in bitmap
      for (int i = 0; i < config.classWidth; i += CLASS_BP_WIDTH) { // Repeat for as many times as needed for intended width
        for (int j = 0; j < h; j += CLASS_BP_HEIGHT) {
          display.drawBitmap(x+i, y+j, current_class_bg, CLASS_BP_WIDTH, CLASS_BP_HEIGHT, GxEPD_BLACK, GxEPD_WHITE);
        }
      }
    }
  }
  else {
    display.writeLine (config.hourLine, config.classHeight*position, config.endClassLine, config.classHeight*(position+1), GxEPD_BLACK);
    display.writeLine (config.hourLine, config.classHeight*(position+1), config.endClassLine, config.classHeight*position, GxEPD_BLACK);
  }

  // White outline
  drawOutline(tx, ty, 3, 3, name, GxEPD_WHITE);

  // Actual text
  display.setTextColor(GxEPD_BLACK);
  display.setCursor(tx, ty);
  display.print(name);
}

// Draws, on the left of the screen, the hours of the schedule
void drawHours(char start) {
  // Giving some margin on the left
  uint16_t x = MARGIN;
  // y position ->  config.topMargin for the border
  //                (config.classHeight + CHAR_HEIGHT)/2 to center inside its spot
  uint16_t y = config.topMargin + (config.classHeight + CHAR_HEIGHT) / 2;
  char pos = 1;  

  // Line to separate hours with the schedule
  if (config.showLines) display.drawFastVLine(config.hourLine, config.topMargin, config.numClassesDisplayed*config.classHeight, GxEPD_BLACK);
  if (config.showLines) display.drawFastHLine(0, config.topMargin, config.endClassLine, GxEPD_BLACK);
  display.setFont(&FreeMono9pt7b);
  for (char i = 0; i < config.numClassesDisplayed; ++i) {
    char hours[7] = "      ";  // 6 chars + null

    // Format hour string
    snprintf(hours, sizeof(hours), "%d-%dh", i+start, i+start+1);

    if (config.saveEnergy || i+start != currentHour) {
      display.setCursor(x, y);
      display.print(hours);
    }
    else {
      display.setFont(&FreeMonoBold9pt7b);
      display.setCursor(x, y);
      display.print(hours);
      display.setFont(&FreeMono9pt7b);
    }

    if (config.showLines) display.drawFastHLine(0, config.topMargin + config.classHeight * pos++, config.endClassLine, GxEPD_BLACK);
    y += config.classHeight;
  }
  display.setFont(&FreeMonoBold9pt7b);
}

// Given an array of strings, where the string is the class and the position is the hour it is done in,
// print each single class in the schedule
void drawClasses(char classes[][32], int16_t durations[], char start) {
  char i = start - START_HOUR;

  char pos = 0; 
  while (pos < config.numClassesDisplayed) {
    if (durations[i] == 0) {
      ++i;
      ++pos;
      continue; // If the classroom is empty, continue
    }
    int16_t duration = durations[i];
    if (duration < 0) {
      int timePassed = -duration;
      i -= timePassed;
      duration = durations[i]-timePassed;
    }

    drawClass(pos, classes[i], min((int)duration,config.numClassesDisplayed-pos), i == curr_class_pos);
    i += durations[i];
    pos += duration;
  }

  if (config.showLines) display.drawFastVLine(config.endClassLine, 0, displayHeight, GxEPD_BLACK);
}

// Draws a QR code of the website where the full schedule can be found
void drawQR() {
  uint16_t qr_x = (displayWidth + config.startAnnouncementsLine - QR_SIZE) / 2;
  uint16_t qr_y = displayHeight - QR_SIZE;
  display.drawBitmap(qr_x, qr_y, qr, QR_SIZE, QR_SIZE, GxEPD_BLACK);

  const char text[] = "Horari sencer:";
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);

  uint16_t text_x = (displayWidth + config.startAnnouncementsLine - tbw) / 2;
  uint16_t text_y = qr_y - CHAR_HEIGHT;

  // if (config.showLines && config.topMargin!=0) display.drawFastHLine(config.startAnnouncementsLine, text_y - CHAR_HEIGHT - MARGIN*2, displayWidth - config.startAnnouncementsLine, GxEPD_BLACK);
  display.setCursor(text_x, text_y);
  display.print(text);
}

// Writes the current and the next scheduled class of the day
void drawCurrentNextClass(char classes[][32], int16_t durations[]) {
  char currClass[64];
  char nextClass[64];
  uint16_t ind_next_start;

  uint16_t x = config.startAnnouncementsLine + 2*MARGIN;
  uint16_t y = MARGIN+CHAR_HEIGHT;
  display.setCursor(x, y);
  display.print("Classe en curs:");

  display.setCursor(x, y+(CHAR_HEIGHT+MARGIN*2)*4);
  display.print("Propera classe:");
  y += CHAR_HEIGHT+MARGIN*2;
  display.setCursor(x, y);
  display.setFont(&FreeMono9pt7b);

  if (curr_class_pos >= 0) {
    snprintf(currClass, sizeof(currClass), classes[curr_class_pos]);
    y += CHAR_HEIGHT+MARGIN*2;
    display.print(currClass);
    display.setCursor(x, y);
    y += CHAR_HEIGHT+MARGIN*2;
    snprintf(currClass, sizeof(currClass), "%d:00-%d:00", START_HOUR+curr_class_pos, START_HOUR+curr_class_pos+durations[curr_class_pos]);
    display.print(currClass);
    y += (CHAR_HEIGHT+MARGIN*2)*2;
  }
  else {
    display.print("-");
    y += (CHAR_HEIGHT+MARGIN*2)*3;
  }

  char i = curr_class_pos >= 0 ? curr_class_pos + durations[curr_class_pos] : config.numClassesDisplayed;
  while (i < config.numClassesDisplayed) {
    if (durations[i] != 0 && strcmp(classes[i], currClass) != 0) {
      snprintf(nextClass, sizeof(nextClass), classes[i]);
      ind_next_start = i;
      i = config.numClassesDisplayed;
    }
    ++i;
  }
  if (i == config.numClassesDisplayed) {
    display.setCursor(x, y);
    display.print("-");
  }
  else {
    display.setCursor(x, y);
    y += CHAR_HEIGHT+MARGIN*2;
    display.print(nextClass);
    display.setCursor(x, y);
    y += CHAR_HEIGHT+MARGIN*2;
    snprintf(nextClass, sizeof(nextClass), "%d:00-%d:00", START_HOUR+ind_next_start, START_HOUR+ind_next_start+durations[ind_next_start]);
    display.print(nextClass);
  }

  display.setFont(&FreeMonoBold9pt7b);
  if (config.showLines) display.drawFastHLine(config.startAnnouncementsLine, config.rows[0], displayWidth - config.startAnnouncementsLine, GxEPD_BLACK);
}

// Auxiliary function to print announcements with line breaks that do not warp around the whole screen
void printWithLineBreaks(const char* text, uint16_t x, uint16_t y, uint8_t maxCharsPerLine) {
  int lineHeight = CHAR_HEIGHT + MARGIN;
  int currentY = y;
  const char* lineStart = text;
  int lineLength = 0;
  char temp[128];

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

      strncpy(temp, lineStart, lineLength);
      temp[lineLength] = '\0';
      display.print(temp);
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
      strncpy(temp, lineStart, lineLength);
      temp[lineLength] = '\0';
      display.print(temp);      currentY += lineHeight;

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
    strncpy(temp, lineStart, lineLength);
    temp[lineLength] = '\0';
    display.print(temp);
  }
}

// Writes the announcements
void drawAnnouncements(char announcement[]) {
  uint16_t x = config.startAnnouncementsLine + MARGIN*2;
  uint16_t y = config.rows[0]+MARGIN;
  char icon_w = 15;
  char icon_h = 15;
  display.drawBitmap(x, y,  avisos_bw, icon_w, icon_h, GxEPD_BLACK);
  display.drawBitmap(x, y, avisos_red, icon_w, icon_h, GxEPD_RED);
  display.setCursor(x+icon_w+MARGIN, y+icon_h);
  display.print("Avisos:");

  y+= icon_h+CHAR_HEIGHT+MARGIN*2;
  display.setFont(&FreeMono9pt7b);

  if (strcmp(announcement, "") == 0) {
    const char text[] = "-";

    display.setCursor(x, y);
    display.print(text);
  }
  else {
    printWithLineBreaks(announcement, x, y, (displayWidth-(config.startAnnouncementsLine+MARGIN*4))/CHAR_WIDTH);
  }

  display.setFont(&FreeMonoBold9pt7b);
}

// Auxiliary function that gets the current hour from the internet
void updateCurrentHour(char classes[][32], int16_t durations[]) {
  // Get current time (to tell what class is next)
  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startAttempt > 5000) return;
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  
  // Init and get the time
  configTime(3600, 3600, "pool.ntp.org");

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    currentHour = timeinfo.tm_hour;
    uint16_t minute = timeinfo.tm_min;

    if (minute >= 50) {
      currentHour = (currentHour + 1) % 24; // Wrap around to 0 if it's 23:50+
    }
  } else {
    Serial.println("Failed to get time");
  }
  
  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  currentHour = constrain(currentHour, START_HOUR, LAST_HOUR);

  curr_class_pos = currentHour-START_HOUR;
  if (strcmp(classes[curr_class_pos], "") != 0) {
    while (durations[curr_class_pos]<0) --curr_class_pos;
  }
  else {
    curr_class_pos = -1;
  }
}

// Auxiliary function that returns if the schedule has changed since last time it was drawn
bool isScheduleChanged(char classes[][32], int16_t durations[], char announcements[], char startHour) {
  if (strcmp(prevAnnouncements, announcements) != 0 || prevStartHour != startHour) {
    return true;
  }

  for (int i = 0; i < NUM_CLASSES; i++) {
    if (strcmp(prevClasses[i], classes[i]) != 0 || prevDurations[i] != durations[i]) {
      return true;
    }
  }
  return false;
}

// Manages the schedule drawing
void drawSchedule(char classes[][32], int16_t durations[], char announcements[]) {
  updateCurrentHour(classes, durations);

  pinMode(PWR, OUTPUT);
  digitalWrite(PWR, HIGH);
  display.init(115200, true, 2, false); // For Waveshare with "clever" reset circuit

  display.firstPage();
  display.setFullWindow();


  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  // In the "spacey" layout, if there are no announcements there is no space reserved for it
  if (strcmp(config.name, "sp") == 0) {
    if (strcmp(announcements, "") == 0) { // No announcements, no need to write them down
      config.showAnnouncements = false;
      config.numClassesDisplayed = 13;
      config.classHeight = (displayHeight/config.numClassesDisplayed);
      config.bottomMargin = (displayHeight-(config.classHeight*config.numClassesDisplayed))/2;
      config.topMargin = config.bottomMargin + ((displayHeight-config.classHeight*config.numClassesDisplayed) % 2 != 0);
    }
    else {
      config.showAnnouncements = true;
      config.numClassesDisplayed = 7;
      config.bottomMargin = 0;
      config.topMargin = 0;
    }
  }

  char startHour;
  if (config.staticSchedule) {
    char middleHour = START_HOUR+config.numClassesDisplayed-1;
    startHour = currentHour < middleHour ? START_HOUR : middleHour;
  } 
  else {
    startHour = min(currentHour, (uint16_t)(LAST_HOUR+1-config.numClassesDisplayed));
  }

  if (config.saveEnergy && !isScheduleChanged(classes, durations, announcements, startHour)) return; // No changes in announcements or in classes, do not have to update at all

  strncpy(prevAnnouncements, announcements, sizeof(prevAnnouncements));
  prevStartHour = startHour;
  for (int i = 0; i < NUM_CLASSES; i++) {
    strcpy(prevClasses[i], classes[i]);
    prevDurations[i] = durations[i];
  }

  do {
    display.fillScreen(GxEPD_WHITE);

    drawHours(startHour);
    drawClasses(classes, durations, startHour);
    
    if (config.showCurrNext) drawCurrentNextClass(classes, durations); // TODO: change number to "hour"
    if(config.showAnnouncements) drawAnnouncements(announcements);
    if (config.showQR) drawQR();
    
  } while (display.nextPage());

  display.powerOff();
  digitalWrite(PWR, LOW);
}

// Manages the picture printing in the display
void drawPicture(bool portrait, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const unsigned char picture_bw[], const unsigned char picture_red[]) {
  pinMode(PWR, OUTPUT);
  digitalWrite(PWR, HIGH);
  display.init(115200, true, 2, false);
  display.firstPage();
  display.setFullWindow();

  display.setRotation(portrait);
  displayHeight = display.height();
  displayWidth = display.width();

  do {
    display.fillScreen(GxEPD_WHITE);
    display.drawBitmap(x, y, picture_bw, displayWidth, displayHeight, GxEPD_BLACK);
    display.drawBitmap(x, y, picture_red, displayWidth, displayHeight, GxEPD_RED);
  } while (display.nextPage());

  display.powerOff();
  digitalWrite(PWR, LOW);
}