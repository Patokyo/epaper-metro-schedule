#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>

// =====================
// Wi-Fi Credentials
// =====================
const char* ssid     = "NOKIA-E2E1";
const char* password = "CAuWQ3Hq3J";

// =====================
// NTP Client Setup
// =====================
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // UTC, convert manually to AEST

// =====================
// E-Paper Pin Setup
// =====================
#define EPD_CS   15
#define EPD_DC   4
#define EPD_RST  2
#define EPD_BUSY 5

GxEPD2_BW<GxEPD2_213_B74, GxEPD2_213_B74::HEIGHT> display(GxEPD2_213_B74(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

struct MetroDeparture {
  int hour;
  int minute;
  int frequencyMinutes; // 0 if single departure
};

// =====================
// Tallawong Timetable
// =====================
MetroDeparture weekdays[] = {
  {4, 8, 10}, {6, 8, 4}, {9, 8, 5}, {14, 8, 4}, {18, 40, 10}, {23, 50, 10}
};

MetroDeparture friday[] = {
  {4, 8, 10}, {6, 8, 4}, {9, 8, 5}, {14, 8, 4}, {18, 40, 10}, {0, 50, 10}
};

MetroDeparture saturday[] = {
  {4, 8, 10}, {0, 48, 10}
};

MetroDeparture sunday[] = {
  {4, 8, 10}, {22, 18, 0}, {22, 28, 0}, {22, 38, 0}, {22, 48, 0}, {22, 58, 0}
};

// =====================
// Helper Functions
// =====================
String formatTime(int hour, int minute) {
  String ampm = "AM";
  if (hour >= 12) {
    ampm = "PM";
    if (hour > 12) hour -= 12;
  }
  if (hour == 0) hour = 12;
  String hourStr = String(hour);
  String minStr = (minute < 10) ? "0" + String(minute) : String(minute);
  return hourStr + ":" + minStr + ampm;
}

MetroDeparture* getTodaysTimetable(int weekday, int &size) {
  switch (weekday) {
    case 1: case 2: case 3: case 4:
      size = sizeof(tallawongWeekdays)/sizeof(MetroDeparture);
      return tallawongWeekdays;
    case 5:
      size = sizeof(tallawongFriday)/sizeof(MetroDeparture);
      return tallawongFriday;
    case 6:
      size = sizeof(tallawongSaturday)/sizeof(MetroDeparture);
      return tallawongSaturday;
    case 0:
      size = sizeof(tallawongSunday)/sizeof(MetroDeparture);
      return tallawongSunday;
    default:
      size = 0;
      return nullptr;
  }
}

// =====================
// Calculate Next Metro
// =====================
void getNextMetro(int hour, int minute, MetroDeparture* timetable, int size, int &nextHour, int &nextMinute, int &minsAway) {
  int currentTotal = hour * 60 + minute;
  minsAway = 24*60; // max
  nextHour = hour;
  nextMinute = minute;

  for (int i = 0; i < size; i++) {
    int startTotal = timetable[i].hour * 60 + timetable[i].minute;
    int freq = timetable[i].frequencyMinutes;
    int depTotal = startTotal;

    if (freq > 0) {
      int intervalsPassed = 0;
      if (currentTotal >= startTotal) {
        intervalsPassed = (currentTotal - startTotal) / freq + 1; // next departure after current
      }
      depTotal = startTotal + intervalsPassed * freq;
    }

    // Wrap around 24h
    depTotal %= 1440;

    int diff = depTotal - currentTotal;
    if (diff < 0) diff += 1440; // wrap past midnight

    if (diff < minsAway) {
      minsAway = diff;
      nextHour = depTotal / 60;
      nextMinute = depTotal % 60;
    }
  }
}

// =====================
// Setup & Loop
// =====================
void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();

  display.init(115200);
  display.setRotation(3);
  display.setTextColor(GxEPD_BLACK);
}

void loop() {
  timeClient.update();

  int utcHour = timeClient.getHours();
  int utcMinute = timeClient.getMinutes();

  int hour = utcHour + 10; // AEST
  if (hour >= 24) hour -= 24;
  int minute = utcMinute;

  time_t rawtime = timeClient.getEpochTime() + 10*3600;
  int wday = weekday(rawtime) - 1;
  if (wday < 0) wday = 6;

  int size;
  MetroDeparture* timetable = getTodaysTimetable(wday, size);

  int nextHour, nextMinute, minsAway;
  getNextMetro(hour, minute, timetable, size, nextHour, nextMinute, minsAway);

  String aestTime = formatAESTTime(hour, minute);

  // Display
  display.firstPage();
  do {
    display.setFont(&FreeMonoBold24pt7b);
    display.setCursor(20, 40);
    display.print(aestTime);

    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(10, 80);
    display.print("Next Metro in...");

    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(40, 110);
    display.print(String(minsAway) + " mins");
  } while (display.nextPage());

  Serial.println("Current time: " + aestTime + ", next metro in " + String(minsAway) + " min");

  delay(60000); // update every minute
}
