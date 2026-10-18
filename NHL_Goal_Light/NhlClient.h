#pragma once
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "Config.h"

enum NhlGameState {
  STATE_UNKNOWN,
  STATE_NO_GAME,
  STATE_FUTURE,
  STATE_PREGAME,
  STATE_LIVE,
  STATE_FINAL
};

struct GameStatus {
  NhlGameState state;
  int gameId;
  String homeAbbrev;
  String awayAbbrev;
  int homeScore;
  int awayScore;
  int period;
  String timeRemaining;
  bool inIntermission;
  String situationCode;
};

class NhlClient {
private:
  String teamAbbrev;
  GameStatus currentStatus;
  int lastWatchedScore;
  int lastOpponentScore;
  unsigned long lastPollTime;
  unsigned long pollIntervalMs;
  bool initialSnapshot;

public:
  NhlClient() : teamAbbrev("NYR"),
                lastWatchedScore(-1),
                lastOpponentScore(-1),
                lastPollTime(0),
                pollIntervalMs(30000),
                initialSnapshot(true) {
    currentStatus.state = STATE_UNKNOWN;
    currentStatus.gameId = 0;
    currentStatus.homeScore = 0;
    currentStatus.awayScore = 0;
    currentStatus.period = 0;
    currentStatus.inIntermission = false;
  }

  void setTeam(const char* abbrev) {
    teamAbbrev = String(abbrev);
    reset();
  }

  void reset() {
    lastWatchedScore = -1;
    lastOpponentScore = -1;
    initialSnapshot = true;
    currentStatus.state = STATE_UNKNOWN;
    currentStatus.gameId = 0;
    pollIntervalMs = 5000;
  }

  const GameStatus& getStatus() const { return currentStatus; }

  // Returns true if preferred team just scored!
  bool poll(bool &outOpponentGoal, bool &outGameWin, bool &outPeriodStart) {
    outOpponentGoal = false;
    outGameWin = false;
    outPeriodStart = false;

    unsigned long now = millis();
    if (now - lastPollTime < pollIntervalMs && lastPollTime != 0) {
      return false;
    }
    lastPollTime = now;

    if (WiFi.status() != WL_CONNECTED) {
      return false;
    }

    // Step 1: If game ID is not discovered or game is finished, discover game
    if (currentStatus.gameId == 0 || currentStatus.state == STATE_FINAL || currentStatus.state == STATE_NO_GAME) {
      discoverGame();
      adjustInterval();
      return false;
    }

    // Step 2: Poll live boxscore
    return pollBoxscore(outOpponentGoal, outGameWin, outPeriodStart);
  }

private:
  void adjustInterval() {
    switch (currentStatus.state) {
      case STATE_NO_GAME:
        pollIntervalMs = 60000UL * 30; // 30 min
        break;
      case STATE_FUTURE:
        pollIntervalMs = 60000UL * 10; // 10 min
        break;
      case STATE_PREGAME:
        pollIntervalMs = 20000;        // 20 sec
        break;
      case STATE_LIVE:
        pollIntervalMs = currentStatus.inIntermission ? 45000 : 4000; // 4s live, 45s intermission
        break;
      case STATE_FINAL:
        pollIntervalMs = 60000UL * 60; // 1 hr
        break;
      default:
        pollIntervalMs = 30000;
        break;
    }
  }

  void discoverGame() {
    Serial.println("[NHL] Checking schedule for team: " + teamAbbrev);
    String url = "https://api-web.nhle.com/v1/club-schedule/" + teamAbbrev + "/week/now";

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.begin(client, url);
    http.setUserAgent("NHL-GoalLight/1.0");
    http.setTimeout(10000);

    int code = http.GET();
    if (code != 200) {
      Serial.printf("[NHL] Schedule HTTP error: %d\n", code);
      http.end();
      pollIntervalMs = 30000;
      return;
    }

    String payload = http.getString();
    http.end();

    // Check if team has games in schedule
    int teamPos = payload.indexOf("\"abbrev\":\"" + teamAbbrev + "\"");
    if (teamPos < 0) {
      Serial.println("[NHL] No games found this week for " + teamAbbrev);
      currentStatus.state = STATE_NO_GAME;
      return;
    }

    // Find nearest game ID
    int idPos = payload.indexOf("\"id\":");
    if (idPos > 0) {
      currentStatus.gameId = extractInt(payload, "\"id\":", 0);
      Serial.printf("[NHL] Discovered Game ID: %d\n", currentStatus.gameId);
      currentStatus.state = STATE_FUTURE;
      pollIntervalMs = 5000; // Switch to quick check
    }
  }

  bool pollBoxscore(bool &outOpponentGoal, bool &outGameWin, bool &outPeriodStart) {
    String url = "https://api-web.nhle.com/v1/gamecenter/" + String(currentStatus.gameId) + "/boxscore";

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.begin(client, url);
    http.setUserAgent("NHL-GoalLight/1.0");
    http.setTimeout(8000);

    int code = http.GET();
    if (code != 200) {
      Serial.printf("[NHL] Boxscore HTTP error: %d\n", code);
      http.end();
      return false;
    }

    // Read payload up until "playerByGameStats" to save RAM
    WiFiClient *stream = http.getStreamPtr();
    String payload;
    payload.reserve(8000);
    const char* stopMarker = "\"playerByGameStats\"";
    int stopLen = strlen(stopMarker);
    String window = "";

    unsigned long startRead = millis();
    while (http.connected() || stream->available()) {
      while (stream->available()) {
        char c = (char)stream->read();
        payload += c;
        window += c;
        if (window.length() > stopLen) {
          window.remove(0, window.length() - stopLen);
        }
        if (window == stopMarker) {
          break;
        }
      }
      if (window == stopMarker) break;
      if (millis() - startRead > 8000) break;
      delay(1);
    }
    http.end();

    // Parse gameState
    String stateStr = extractString(payload, "\"gameState\":\"", 0);
    if (stateStr == "FUT") currentStatus.state = STATE_FUTURE;
    else if (stateStr == "PRE") currentStatus.state = STATE_PREGAME;
    else if (stateStr == "LIVE" || stateStr == "CRIT") currentStatus.state = STATE_LIVE;
    else if (stateStr == "FINAL" || stateStr == "OFF") currentStatus.state = STATE_FINAL;

    // Parse teams and scores
    String homeBlock = extractObject(payload, "\"homeTeam\":", 0);
    String awayBlock = extractObject(payload, "\"awayTeam\":", 0);

    currentStatus.homeAbbrev = extractString(homeBlock, "\"abbrev\":\"", 0);
    currentStatus.awayAbbrev = extractString(awayBlock, "\"abbrev\":\"", 0);
    currentStatus.homeScore = extractInt(homeBlock, "\"score\":", 0);
    currentStatus.awayScore = extractInt(awayBlock, "\"score\":", 0);

    int watchedScore = (currentStatus.homeAbbrev == teamAbbrev) ? currentStatus.homeScore : currentStatus.awayScore;
    int opponentScore = (currentStatus.homeAbbrev == teamAbbrev) ? currentStatus.awayScore : currentStatus.homeScore;

    adjustInterval();

    // If first poll after boot, record snapshot silently
    if (initialSnapshot) {
      lastWatchedScore = watchedScore;
      lastOpponentScore = opponentScore;
      initialSnapshot = false;
      Serial.printf("[NHL] Snapshot captured: %s %d - %d\n", teamAbbrev.c_str(), watchedScore, opponentScore);
      return false;
    }

    bool watchedGoal = false;
    if (watchedScore > lastWatchedScore) {
      Serial.printf("[NHL] GOAL DETECTED FOR %s! (%d -> %d)\n", teamAbbrev.c_str(), lastWatchedScore, watchedScore);
      watchedGoal = true;
    } else if (opponentScore > lastOpponentScore) {
      Serial.printf("[NHL] Opponent goal detected! (%d -> %d)\n", lastOpponentScore, opponentScore);
      outOpponentGoal = true;
    }

    // Check for victory condition on FINAL
    if (currentStatus.state == STATE_FINAL && (lastWatchedScore != watchedScore || lastOpponentScore != opponentScore)) {
      if (watchedScore > opponentScore) {
        outGameWin = true;
      }
    }

    lastWatchedScore = watchedScore;
    lastOpponentScore = opponentScore;
    return watchedGoal;
  }

  int extractInt(const String &text, const char *marker, int startPos) {
    int pos = text.indexOf(marker, startPos);
    if (pos < 0) return -1;
    pos += strlen(marker);
    while (pos < text.length() && !isDigit(text[pos]) && text[pos] != '-') pos++;
    if (pos >= text.length()) return -1;
    int val = 0;
    while (pos < text.length() && isDigit(text[pos])) {
      val = val * 10 + (text[pos] - '0');
      pos++;
    }
    return val;
  }

  String extractString(const String &text, const char *marker, int startPos) {
    int pos = text.indexOf(marker, startPos);
    if (pos < 0) return "";
    pos += strlen(marker);
    int end = text.indexOf("\"", pos);
    if (end < 0) return "";
    return text.substring(pos, end);
  }

  String extractObject(const String &text, const char *marker, int startPos) {
    int pos = text.indexOf(marker, startPos);
    if (pos < 0) return "";
    int objStart = text.indexOf("{", pos);
    if (objStart < 0) return "";
    int depth = 0;
    for (int i = objStart; i < text.length(); i++) {
      if (text[i] == '{') depth++;
      else if (text[i] == '}') {
        depth--;
        if (depth == 0) return text.substring(objStart, i + 1);
      }
    }
    return "";
  }
};
