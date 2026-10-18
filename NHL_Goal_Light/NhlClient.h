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

struct UpcomingGame {
  bool valid;
  int gameId;
  String date;         // e.g. "2026-10-19"
  String timeUTC;      // e.g. "2026-10-19T23:30:00Z"
  String gameState;    // e.g. "FUT", "PRE", "LIVE", "FINAL"
  String opponent;     // e.g. "SJS"
  String opponentName; // e.g. "Sharks"
  String homeAbbrev;   // e.g. "TOR"
  String awayAbbrev;   // e.g. "SJS"
  String homeName;     // e.g. "Maple Leafs"
  String awayName;     // e.g. "Sharks"
  String venue;        // e.g. "Scotiabank Arena"
  String broadcasts;   // e.g. "SN+, TVAS, NBCSCA"
  bool isHome;         // true if teamAbbrev is home team
};

class NhlClient {
private:
  String teamAbbrev;
  GameStatus currentStatus;
  UpcomingGame upcoming;
  int lastWatchedScore;
  int lastOpponentScore;
  unsigned long lastPollTime;
  unsigned long pollIntervalMs;
  bool initialSnapshot;

public:
  NhlClient() : teamAbbrev("TOR"),
                lastWatchedScore(-1),
                lastOpponentScore(-1),
                lastPollTime(0),
                pollIntervalMs(5000),
                initialSnapshot(true) {
    currentStatus.state = STATE_UNKNOWN;
    currentStatus.gameId = 0;
    currentStatus.homeScore = 0;
    currentStatus.awayScore = 0;
    currentStatus.period = 0;
    currentStatus.inIntermission = false;
    upcoming.valid = false;
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
    upcoming.valid = false;
    pollIntervalMs = 5000;
  }

  const GameStatus& getStatus() const { return currentStatus; }
  const UpcomingGame& getUpcoming() const { return upcoming; }

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
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    http.begin(client, url);
    http.setUserAgent("NHL-GoalLight/1.0");
    http.setTimeout(10000);

    int code = http.GET();
    if (code == 301 || code == 302 || code == 307 || code == 308) {
      String redirectUrl = http.getLocation();
      if (redirectUrl.length() > 0) {
        http.end();
        http.begin(client, redirectUrl);
        code = http.GET();
      }
    }

    if (code != 200) {
      Serial.printf("[NHL] Schedule HTTP error: %d\n", code);
      http.end();
      pollIntervalMs = 30000;
      return;
    }

    String payload = http.getString();
    http.end();

    parseSchedule(payload);
  }

  void parseSchedule(const String &payload) {
    int gamesPos = payload.indexOf("\"games\":[");
    if (gamesPos < 0) {
      upcoming.valid = false;
      currentStatus.state = STATE_NO_GAME;
      return;
    }

    int searchPos = gamesPos + 8;
    bool foundActive = false;
    UpcomingGame bestUpcoming;
    bestUpcoming.valid = false;

    while (searchPos < payload.length()) {
      int gameStart = payload.indexOf("{\"id\":", searchPos);
      if (gameStart < 0) break;
      
      int depth = 0;
      int gameEnd = -1;
      for (int i = gameStart; i < payload.length(); i++) {
        if (payload[i] == '{') depth++;
        else if (payload[i] == '}') {
          depth--;
          if (depth == 0) {
            gameEnd = i;
            break;
          }
        }
      }
      if (gameEnd < 0) break;

      String gObj = payload.substring(gameStart, gameEnd + 1);
      searchPos = gameEnd + 1;

      int gId = extractInt(gObj, "\"id\":", 0);
      String gState = extractString(gObj, "\"gameState\":\"", 0);
      String gDate = extractString(gObj, "\"gameDate\":\"", 0);
      String gTime = extractString(gObj, "\"startTimeUTC\":\"", 0);

      String homeBlock = extractObject(gObj, "\"homeTeam\":", 0);
      String awayBlock = extractObject(gObj, "\"awayTeam\":", 0);
      String hAbbrev = extractString(homeBlock, "\"abbrev\":\"", 0);
      String aAbbrev = extractString(awayBlock, "\"abbrev\":\"", 0);
      String hName = extractString(homeBlock, "\"default\":\"", 0);
      String aName = extractString(awayBlock, "\"default\":\"", 0);

      String venueBlock = extractObject(gObj, "\"venue\":", 0);
      String venueName = extractString(venueBlock, "\"default\":\"", 0);

      String bcasts = "";
      int netSearch = 0;
      while ((netSearch = gObj.indexOf("\"network\":\"", netSearch)) >= 0) {
        String net = extractString(gObj, "\"network\":\"", netSearch);
        if (net.length() > 0) {
          if (bcasts.length() > 0 && bcasts.indexOf(net) < 0) bcasts += ", ";
          if (bcasts.indexOf(net) < 0) bcasts += net;
        }
        netSearch += 11;
      }

      bool isHome = (hAbbrev == teamAbbrev);
      String opp = isHome ? aAbbrev : hAbbrev;
      String oppName = isHome ? aName : hName;

      UpcomingGame gCandidate;
      gCandidate.valid = true;
      gCandidate.gameId = gId;
      gCandidate.date = gDate;
      gCandidate.timeUTC = gTime;
      gCandidate.opponent = opp;
      gCandidate.opponentName = oppName;
      gCandidate.isHome = isHome;
      gCandidate.venue = venueName;
      gCandidate.broadcasts = bcasts;
      gCandidate.gameState = gState;

      // Active live game?
      if (gState == "LIVE" || gState == "CRIT") {
        upcoming = gCandidate;
        currentStatus.gameId = gId;
        currentStatus.state = STATE_LIVE;
        currentStatus.homeAbbrev = hAbbrev;
        currentStatus.awayAbbrev = aAbbrev;
        foundActive = true;
        pollIntervalMs = 4000;
        Serial.printf("[NHL] LIVE GAME ACTIVE! %s vs %s (ID: %d)\n", aAbbrev.c_str(), hAbbrev.c_str(), gId);
        break;
      }

      // Future/Pregame candidate
      if (gState == "FUT" || gState == "PRE") {
        if (!bestUpcoming.valid) {
          bestUpcoming = gCandidate;
        }
      } else if (gState == "FINAL" || gState == "OFF") {
        if (!bestUpcoming.valid) {
          bestUpcoming = gCandidate;
        }
      }
    }

    if (!foundActive) {
      if (bestUpcoming.valid) {
        upcoming = bestUpcoming;
        currentStatus.gameId = bestUpcoming.gameId;
        if (bestUpcoming.gameState == "PRE") {
          currentStatus.state = STATE_PREGAME;
          pollIntervalMs = 20000;
        } else if (bestUpcoming.gameState == "FINAL" || bestUpcoming.gameState == "OFF") {
          currentStatus.state = STATE_FINAL;
          pollIntervalMs = 60000UL * 30;
        } else {
          currentStatus.state = STATE_FUTURE;
          pollIntervalMs = 60000UL * 10;
        }
        Serial.printf("[NHL] Upcoming: %s vs %s on %s at %s (Venue: %s, TV: %s)\n",
                      bestUpcoming.isHome ? bestUpcoming.opponent.c_str() : teamAbbrev.c_str(),
                      bestUpcoming.isHome ? teamAbbrev.c_str() : bestUpcoming.opponent.c_str(),
                      bestUpcoming.date.c_str(), bestUpcoming.timeUTC.c_str(),
                      bestUpcoming.venue.c_str(), bestUpcoming.broadcasts.c_str());
      } else {
        upcoming.valid = false;
        currentStatus.state = STATE_NO_GAME;
        pollIntervalMs = 60000UL * 30;
      }
    }
  }

  bool pollBoxscore(bool &outOpponentGoal, bool &outGameWin, bool &outPeriodStart) {
    String url = "https://api-web.nhle.com/v1/gamecenter/" + String(currentStatus.gameId) + "/boxscore";

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); // Follow 301/302/307 redirects
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
