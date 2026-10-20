#pragma once
#include <Arduino.h>

struct TeamData {
  const char* abbrev;
  const char* name;
  uint8_t primaryR;
  uint8_t primaryG;
  uint8_t primaryB;
  uint8_t secondaryR;
  uint8_t secondaryG;
  uint8_t secondaryB;
};

// Complete official 32 NHL Teams with primary & secondary RGB colors
const TeamData NHL_TEAMS[] = {
  {"ANA", "Anaheim Ducks",         244, 122,  56,  185, 151,  91},
  {"BOS", "Boston Bruins",         255, 184,  28,    0,   0,   0},
  {"BUF", "Buffalo Sabres",          0,  38,  84,  252, 181,  20},
  {"CGY", "Calgary Flames",        200,  16,  46,  241, 190,  72},
  {"CAR", "Carolina Hurricanes",   204,   0,   0,    0,   0,   0},
  {"CHI", "Chicago Blackhawks",    207,  10,  44,    0,   0,   0},
  {"COL", "Colorado Avalanche",    111,  38,  61,   35,  97, 146},
  {"CBJ", "Columbus Blue Jackets",   0,  38,  84,  206,  17,  38},
  {"DAL", "Dallas Stars",            0, 104,  71,  143, 143, 140},
  {"DET", "Detroit Red Wings",     206,  17,  38,  255, 255, 255},
  {"EDM", "Edmonton Oilers",         4,  30,  66,  255,  76,   0},
  {"FLA", "Florida Panthers",        4,  30,  66,  200,  16,  46},
  {"LAK", "Los Angeles Kings",      17,  17,  17,  162, 170, 173},
  {"MIN", "Minnesota Wild",         21,  71,  52,  166,  25,  46},
  {"MTL", "Montreal Canadiens",    175,  30,  45,   25,  33, 104},
  {"NSH", "Nashville Predators",   255, 184,  28,    4,  30,  66},
  {"NJD", "New Jersey Devils",     206,  17,  38,    0,   0,   0},
  {"NYI", "New York Islanders",      0,  83, 155,  244, 121,  32},
  {"NYR", "New York Rangers",        0,  56, 168,  206,  17,  38},
  {"OTT", "Ottawa Senators",       200,  16,  46,  214, 159,  61},
  {"PHI", "Philadelphia Flyers",   247,  73,   2,    0,   0,   0},
  {"PIT", "Pittsburgh Penguins",   252, 181,  20,    0,   0,   0},
  {"SJS", "San Jose Sharks",         0, 109, 117,  234, 114,   0},
  {"SEA", "Seattle Kraken",          0,  22,  40,  153, 217, 217},
  {"STL", "St. Louis Blues",         0,  47, 135,  252, 181,  20},
  {"TBL", "Tampa Bay Lightning",     0,  40, 104,  255, 255, 255},
  {"TOR", "Toronto Maple Leafs",     0,  32,  91,  255, 255, 255},
  {"UTA", "Utah Hockey Club",      113, 175, 229,    9,   9,   9},
  {"VAN", "Vancouver Canucks",       0,  32,  91,    0, 132,  61},
  {"VGK", "Vegas Golden Knights",  180, 151,  90,   51,  63,  72},
  {"WSH", "Washington Capitals",     4,  30,  66,  200,  16,  46},
  {"WPG", "Winnipeg Jets",           4,  30,  66,    0,  76, 151}
};

const int TOTAL_NHL_TEAMS = sizeof(NHL_TEAMS) / sizeof(NHL_TEAMS[0]);

inline const TeamData* findTeam(const char* abbrev) {
  for (int i = 0; i < TOTAL_NHL_TEAMS; i++) {
    if (strcmp(NHL_TEAMS[i].abbrev, abbrev) == 0) {
      return &NHL_TEAMS[i];
    }
  }
  // Default to New York Rangers if not found
  return &NHL_TEAMS[18];
}
