#pragma once
#include <Arduino.h>

#define MAX_EVENT_LOGS 80

struct EventLogItem {
  uint32_t ms;
  char text[80];
};

class EventLogManager {
public:
  void log(const char* fmt, ...) {
    char buf[80];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    Serial.println(buf);

    items[head].ms = millis();
    strncpy(items[head].text, buf, sizeof(items[head].text) - 1);
    items[head].text[sizeof(items[head].text) - 1] = '\0';

    head = (head + 1) % MAX_EVENT_LOGS;
    if (count < MAX_EVENT_LOGS) count++;
  }

  String getText() {
    String out;
    out.reserve(count * 64 + 128);
    out = "=== NHL GOAL LIGHT VERBOSE LOG ===\n";
    out += "Uptime: " + String(millis() / 1000UL) + "s | Entries: " + String(count) + "\n\n";
    int start = (count < MAX_EVENT_LOGS) ? 0 : head;
    for (int i = 0; i < count; i++) {
      int idx = (start + i) % MAX_EVENT_LOGS;
      uint32_t s = items[idx].ms / 1000UL;
      char h[24];
      snprintf(h, sizeof(h), "[+%02u:%02u:%02u] ", (s / 3600), (s / 60) % 60, s % 60);
      out += String(h) + String(items[idx].text) + "\n";
    }
    return out;
  }

private:
  EventLogItem items[MAX_EVENT_LOGS];
  int head = 0;
  int count = 0;
};

extern EventLogManager eventLog;
