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
  int secondsRemaining;
  bool clockRunning;
  bool inIntermission;
  String situationCode;
  String lastPlay;
  unsigned long lastStoppageMillis;
  unsigned long lastResumeMillis;
  int stoppageSecondsRemaining;
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
  unsigned long lastClockTick;
  bool initialSnapshot;

public:
  NhlClient() : teamAbbrev("TOR"),
                lastWatchedScore(-1),
                lastOpponentScore(-1),
                lastPollTime(0),
                pollIntervalMs(2000),
                lastClockTick(0),
                initialSnapshot(true) {
    currentStatus.state = STATE_UNKNOWN;
    currentStatus.gameId = 0;
    currentStatus.homeScore = 0;
    currentStatus.awayScore = 0;
    currentStatus.period = 0;
    currentStatus.timeRemaining = "--:--";
    currentStatus.secondsRemaining = 1200;
    currentStatus.clockRunning = false;
    currentStatus.inIntermission = false;
    currentStatus.lastStoppageMillis = 0;
    currentStatus.lastResumeMillis = 0;
    currentStatus.stoppageSecondsRemaining = 1200;
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
    currentStatus.timeRemaining = "--:--";
    currentStatus.secondsRemaining = 1200;
    currentStatus.clockRunning = false;
    currentStatus.inIntermission = false;
    currentStatus.lastStoppageMillis = 0;
    currentStatus.lastResumeMillis = 0;
    currentStatus.stoppageSecondsRemaining = 1200;
    lastClockTick = 0;
    upcoming.valid = false;
    pollIntervalMs = 2000;
  }

  const GameStatus& getStatus() const { return currentStatus; }
  const UpcomingGame& getUpcoming() const { return upcoming; }

  // Ticks the game clock down second-by-second when play is active
  void tick() {
    if (currentStatus.state != STATE_LIVE || currentStatus.inIntermission || !currentStatus.clockRunning) {
      return;
    }
    unsigned long now = millis();
    if (now - lastClockTick >= 1000) {
      lastClockTick = now;
      if (currentStatus.secondsRemaining > 0) {
        currentStatus.secondsRemaining--;
        int mins = currentStatus.secondsRemaining / 60;
        int secs = currentStatus.secondsRemaining % 60;
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d:%02d", mins, secs);
        currentStatus.timeRemaining = String(buf);
      } else {
        currentStatus.clockRunning = false;
      }
    }
  }

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
        pollIntervalMs = 15000;        // 15 sec
        break;
      case STATE_LIVE:
        // High-frequency 2-second polling during active play, 20s during intermission
        pollIntervalMs = currentStatus.inIntermission ? 20000 : 2000;
        break;
      case STATE_FINAL:
        pollIntervalMs = 60000UL * 30; // 30 min
        break;
      default:
        pollIntervalMs = 15000;
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
        pollIntervalMs = 2000;
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

  String formatPlayObject(const String &playObj, int homeId, int awayId) {
    int pNum = extractInt(playObj, "\"number\":", 0);
    if (pNum <= 0) pNum = currentStatus.period;
    String tip = extractString(playObj, "\"timeInPeriod\":\"", 0);
    if (tip.length() == 0) tip = currentStatus.timeRemaining;
    String typeDescKey = extractString(playObj, "\"typeDescKey\":\"", 0);
    int ownerId = extractInt(playObj, "\"eventOwnerTeamId\":", 0);
    String tm = (ownerId == homeId) ? currentStatus.homeAbbrev : ((ownerId == awayId) ? currentStatus.awayAbbrev : "");

    String prefix = "P" + String(pNum) + " " + tip + " - ";

    if (typeDescKey == "stoppage") {
      String reason = extractString(playObj, "\"reason\":\"", 0);
      if (reason.length() == 0) reason = "stoppage";
      reason.replace("-", " ");
      if (reason.length() > 0) reason[0] = toupper(reason[0]);
      return prefix + "Stoppage: " + reason;
    } else if (typeDescKey == "shot-on-goal") {
      String shotType = extractString(playObj, "\"shotType\":\"", 0);
      int aSog = extractInt(playObj, "\"awaySOG\":", 0);
      int hSog = extractInt(playObj, "\"homeSOG\":", 0);
      String stStr = (shotType.length() > 0) ? " (" + shotType + ")" : "";
      String sogStr = (aSog >= 0 && hSog >= 0) ? " [SOG: " + currentStatus.awayAbbrev + " " + String(aSog) + " - " + currentStatus.homeAbbrev + " " + String(hSog) + "]" : "";
      return prefix + "Shot on Goal: " + tm + stStr + sogStr;
    } else if (typeDescKey == "goal") {
      String shotType = extractString(playObj, "\"shotType\":\"", 0);
      int aSc = extractInt(playObj, "\"awayScore\":", 0);
      int hSc = extractInt(playObj, "\"homeScore\":", 0);
      String stStr = (shotType.length() > 0) ? " (" + shotType + ")" : "";
      String scStr = (aSc >= 0 && hSc >= 0) ? " [Score: " + currentStatus.awayAbbrev + " " + String(aSc) + " - " + currentStatus.homeAbbrev + " " + String(hSc) + "]" : "";
      return prefix + "GOAL: " + tm + stStr + scStr;
    } else if (typeDescKey == "penalty") {
      String desc = extractString(playObj, "\"descKey\":\"", 0);
      int dur = extractInt(playObj, "\"duration\":", 0);
      if (desc.length() == 0) desc = "penalty";
      desc.replace("-", " ");
      if (desc.length() > 0) desc[0] = toupper(desc[0]);
      return prefix + "Penalty: " + tm + " " + desc + " (" + String(dur > 0 ? dur : 2) + "m)";
    } else if (typeDescKey == "faceoff") {
      return prefix + "Faceoff: Won by " + tm;
    } else if (typeDescKey == "hit") {
      return prefix + "Hit by " + tm;
    } else if (typeDescKey == "blocked-shot") {
      return prefix + "Blocked Shot by " + tm;
    } else if (typeDescKey == "missed-shot") {
      String reason = extractString(playObj, "\"reason\":\"", 0);
      reason.replace("-", " ");
      return prefix + "Missed Shot: " + tm + (reason.length() > 0 ? " (" + reason + ")" : "");
    } else if (typeDescKey == "giveaway") {
      return prefix + "Giveaway: " + tm;
    } else if (typeDescKey == "takeaway") {
      return prefix + "Takeaway: " + tm;
    } else if (typeDescKey == "period-start") {
      return prefix + "Period " + String(pNum) + " Started";
    } else if (typeDescKey == "period-end") {
      return prefix + "Period " + String(pNum) + " Ended";
    } else if (typeDescKey.length() > 0) {
      typeDescKey.replace("-", " ");
      return prefix + typeDescKey;
    }
    return prefix + "Play in Progress";
  }

  bool pollBoxscore(bool &outOpponentGoal, bool &outGameWin, bool &outPeriodStart) {
    // Append random cache buster parameter to bypass Cloudflare/CDN caching
    String url = "https://api-web.nhle.com/v1/gamecenter/" + String(currentStatus.gameId) + "/play-by-play?_=" + String(millis()) + String(random(1000, 9999));

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.begin(client, url);
    http.setUserAgent("Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko)");
    http.setTimeout(8000);

    int code = http.GET();
    if (code != 200) {
      Serial.printf("[NHL] play-by-play error %d, falling back to landing\n", code);
      http.end();
      return pollLandingFallback(outOpponentGoal, outGameWin, outPeriodStart);
    }

    WiFiClient *stream = http.getStreamPtr();
    String headBlock = "";
    headBlock.reserve(3072);
    String rolling = "";
    rolling.reserve(4096);
    uint8_t buf[1024];
    unsigned long streamStart = millis();

    while (http.connected() && (millis() - streamStart < 6000)) {
      size_t avail = stream->available();
      if (avail > 0) {
        size_t toRead = (avail > sizeof(buf) - 1) ? sizeof(buf) - 1 : avail;
        int c = stream->readBytes((char*)buf, toRead);
        if (c > 0) {
          buf[c] = '\0';
          if (headBlock.length() < 3000) {
            headBlock += (char*)buf;
            if (headBlock.length() > 3000) {
              headBlock = headBlock.substring(0, 3000);
            }
          }
          rolling += (char*)buf;
          if (rolling.length() > 4096) {
            rolling = rolling.substring(rolling.length() - 4096);
          }
          if (rolling.indexOf("],\"rosterSpots\":") >= 0) {
            break;
          }
        }
      } else {
        delay(2);
      }
    }
    http.end();

    if (headBlock.length() < 200) {
      Serial.println("[NHL] Incomplete payload from play-by-play stream");
      return pollLandingFallback(outOpponentGoal, outGameWin, outPeriodStart);
    }

    // Parse gameState
    String stateStr = extractString(headBlock, "\"gameState\":\"", 0);
    if (stateStr == "FUT") currentStatus.state = STATE_FUTURE;
    else if (stateStr == "PRE") currentStatus.state = STATE_PREGAME;
    else if (stateStr == "LIVE" || stateStr == "CRIT") currentStatus.state = STATE_LIVE;
    else if (stateStr == "FINAL" || stateStr == "OFF") currentStatus.state = STATE_FINAL;

    // Parse teams and scores
    String homeBlock = extractObject(headBlock, "\"homeTeam\":", 0);
    String awayBlock = extractObject(headBlock, "\"awayTeam\":", 0);

    currentStatus.homeAbbrev = extractString(homeBlock, "\"abbrev\":\"", 0);
    currentStatus.awayAbbrev = extractString(awayBlock, "\"abbrev\":\"", 0);
    currentStatus.homeScore = extractInt(homeBlock, "\"score\":", 0);
    currentStatus.awayScore = extractInt(awayBlock, "\"score\":", 0);
    int homeId = extractInt(homeBlock, "\"id\":", 0);
    int awayId = extractInt(awayBlock, "\"id\":", 0);

    // Parse Period
    String periodBlock = extractObject(headBlock, "\"periodDescriptor\":", 0);
    if (periodBlock.length() > 0) {
      currentStatus.period = extractInt(periodBlock, "\"number\":", 0);
    }
    if (currentStatus.period <= 0) currentStatus.period = 1;

    // Parse Clock
    String clockBlock = extractObject(headBlock, "\"clock\":", 0);
    if (clockBlock.length() > 0) {
      int sec = extractInt(clockBlock, "\"secondsRemaining\":", 0);
      bool running = (clockBlock.indexOf("\"running\":true") >= 0);
      bool interm = (clockBlock.indexOf("\"inIntermission\":true") >= 0);

      if (sec >= 0) {
        currentStatus.secondsRemaining = sec;
      }

      // Track referee stoppage & resume transitions
      if (currentStatus.clockRunning && !running) {
        currentStatus.lastStoppageMillis = millis();
        currentStatus.stoppageSecondsRemaining = currentStatus.secondsRemaining;
        Serial.printf("[Clock] Referee stoppage at %02d:%02d\n", currentStatus.secondsRemaining / 60, currentStatus.secondsRemaining % 60);
      } else if (!currentStatus.clockRunning && running) {
        currentStatus.lastResumeMillis = millis();
        Serial.printf("[Clock] Play resumed at %02d:%02d\n", currentStatus.secondsRemaining / 60, currentStatus.secondsRemaining % 60);
      }

      currentStatus.clockRunning = running;
      currentStatus.inIntermission = interm;

      if (currentStatus.inIntermission) {
        currentStatus.timeRemaining = "Intermission";
        currentStatus.clockRunning = false;
      } else if (sec >= 0) {
        int m = sec / 60;
        int s = sec % 60;
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d:%02d", m, s);
        currentStatus.timeRemaining = String(buf);
      } else {
        currentStatus.timeRemaining = extractString(clockBlock, "\"timeRemaining\":\"", 0);
      }
      lastClockTick = millis();
    }

    // Extract Last Play from rolling buffer
    int rPos = rolling.indexOf("],\"rosterSpots\":");
    String playsEnd = (rPos >= 0) ? rolling.substring(0, rPos) : rolling;
    int lastPlayStart = playsEnd.lastIndexOf("{\"eventId\":");
    if (lastPlayStart >= 0) {
      String playObj = playsEnd.substring(lastPlayStart);
      currentStatus.lastPlay = formatPlayObject(playObj, homeId, awayId);
    } else if (currentStatus.state == STATE_LIVE) {
      currentStatus.lastPlay = "P" + String(currentStatus.period) + " " + currentStatus.timeRemaining + " - Live Play in Progress";
    } else if (currentStatus.state == STATE_FINAL) {
      currentStatus.lastPlay = "Game Final: " + currentStatus.awayAbbrev + " " + String(currentStatus.awayScore) + " - " + currentStatus.homeAbbrev + " " + String(currentStatus.homeScore);
    } else if (currentStatus.state == STATE_PREGAME) {
      currentStatus.lastPlay = "Pre-Game Warmups in Progress";
    }

    int watchedScore = (currentStatus.homeAbbrev == teamAbbrev) ? currentStatus.homeScore : currentStatus.awayScore;
    int opponentScore = (currentStatus.homeAbbrev == teamAbbrev) ? currentStatus.awayScore : currentStatus.homeScore;

    adjustInterval();

    // If first poll after boot, record snapshot silently
    if (initialSnapshot) {
      lastWatchedScore = watchedScore;
      lastOpponentScore = opponentScore;
      initialSnapshot = false;
      Serial.printf("[NHL] Snapshot captured: %s %d - %d | Last play: %s\n",
                    teamAbbrev.c_str(), watchedScore, opponentScore, currentStatus.lastPlay.c_str());
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

  bool pollLandingFallback(bool &outOpponentGoal, bool &outGameWin, bool &outPeriodStart) {
    String url = "https://api-web.nhle.com/v1/gamecenter/" + String(currentStatus.gameId) + "/landing?_=" + String(millis()) + String(random(1000, 9999));

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.begin(client, url);
    http.setUserAgent("Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko)");
    http.setTimeout(8000);

    int code = http.GET();
    if (code != 200) {
      Serial.printf("[NHL] Landing HTTP error: %d\n", code);
      http.end();
      return false;
    }

    String payload = http.getString();
    http.end();

    String stateStr = extractString(payload, "\"gameState\":\"", 0);
    if (stateStr == "FUT") currentStatus.state = STATE_FUTURE;
    else if (stateStr == "PRE") currentStatus.state = STATE_PREGAME;
    else if (stateStr == "LIVE" || stateStr == "CRIT") currentStatus.state = STATE_LIVE;
    else if (stateStr == "FINAL" || stateStr == "OFF") currentStatus.state = STATE_FINAL;

    String homeBlock = extractObject(payload, "\"homeTeam\":", 0);
    String awayBlock = extractObject(payload, "\"awayTeam\":", 0);

    currentStatus.homeAbbrev = extractString(homeBlock, "\"abbrev\":\"", 0);
    currentStatus.awayAbbrev = extractString(awayBlock, "\"abbrev\":\"", 0);
    currentStatus.homeScore = extractInt(homeBlock, "\"score\":", 0);
    currentStatus.awayScore = extractInt(awayBlock, "\"score\":", 0);

    String clockBlock = extractObject(payload, "\"clock\":", 0);
    String periodBlock = extractObject(payload, "\"periodDescriptor\":", 0);

    if (clockBlock.length() > 0) {
      int sec = extractInt(clockBlock, "\"secondsRemaining\":", 0);
      bool running = (clockBlock.indexOf("\"running\":true") >= 0);
      bool interm = (clockBlock.indexOf("\"inIntermission\":true") >= 0);

      if (sec >= 0) {
        currentStatus.secondsRemaining = sec;
      }

      if (currentStatus.clockRunning && !running) {
        currentStatus.lastStoppageMillis = millis();
        currentStatus.stoppageSecondsRemaining = currentStatus.secondsRemaining;
      } else if (!currentStatus.clockRunning && running) {
        currentStatus.lastResumeMillis = millis();
      }

      currentStatus.clockRunning = running;
      currentStatus.inIntermission = interm;

      if (currentStatus.inIntermission) {
        currentStatus.timeRemaining = "Intermission";
        currentStatus.clockRunning = false;
      } else if (sec >= 0) {
        int m = sec / 60;
        int s = sec % 60;
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d:%02d", m, s);
        currentStatus.timeRemaining = String(buf);
      } else {
        currentStatus.timeRemaining = extractString(clockBlock, "\"timeRemaining\":\"", 0);
      }
      lastClockTick = millis();
    }
    if (periodBlock.length() > 0) {
      currentStatus.period = extractInt(periodBlock, "\"number\":", 0);
    }
    if (currentStatus.period <= 0) currentStatus.period = 1;

    adjustInterval();

    int watchedScore = (currentStatus.homeAbbrev == teamAbbrev) ? currentStatus.homeScore : currentStatus.awayScore;
    int opponentScore = (currentStatus.homeAbbrev == teamAbbrev) ? currentStatus.awayScore : currentStatus.homeScore;

    if (initialSnapshot) {
      lastWatchedScore = watchedScore;
      lastOpponentScore = opponentScore;
      initialSnapshot = false;
      return false;
    }

    bool watchedGoal = false;
    if (watchedScore > lastWatchedScore) {
      watchedGoal = true;
    } else if (opponentScore > lastOpponentScore) {
      outOpponentGoal = true;
    }

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
