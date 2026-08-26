// ==================================================
// 07_NASTAVENIA – CENTRÁLNE NASTAVENIA
// ==================================================


// ==================================================
// SOLÁR
// ==================================================

// Zdroj efektivnej senzorovej hodnoty. UART fallback je diagnosticka/
// prevadzkova vrstva, nie safety dokaz.
enum ZdrojHodnoty : byte {
  ZDROJ_ZIADNY,
  ZDROJ_LOKALNY_PRIMARNY,
  ZDROJ_LOKALNY_FALLBACK,
  ZDROJ_VZDIALENY_FALLBACK
};

// Rozdiel T3 - T1 potrebný na zapnutie soláru
// Heartbeat externeho watchdogu
extern const int HEARTBEAT_PIN = 44;

// Prva fyzicka implementacia SMART agreement. HIGH = COM-NO, LOW = COM-NC.
const byte MEGA_HL_RELAY_2_PIN = 31;

const float SOLAR_ZAP = 2.0;

// Rozdiel T3 - T1 pri ktorom sa solár vypne
const float SOLAR_VYP = 1.0;


// ==================================================
// FILTRACIA
// ==================================================

// Opakovany cas zapnutej filtracie.
const unsigned long FILTRACIA_CAS_ON = 6UL * 60UL * 60UL * 1000UL;

// Opakovany cas vypnutej filtracie.
const unsigned long FILTRACIA_CAS_OFF = 6UL * 60UL * 60UL * 1000UL;

// Maximalny cas jedneho suvisleho fyzickeho povolenia Bestway filtracie.
// Je nezavisly od RTC, millis fallbacku aj casu manualnej poziadavky.
const unsigned long FILTRACIA_MAX_SUVISLE_ON = 6UL * 60UL * 60UL * 1000UL;

// Kratke odpojenie napajania obnovi interny priblizne 6 h casovac Bestway.
const unsigned long FILTRACIA_RESET_OFF_CAS = 2000UL;

// RTC harmonogram filtracie v hodinach (format 24 h).
const byte FILTRACIA_RTC_ON_1_OD = 0;
const byte FILTRACIA_RTC_ON_1_DO = 6;
const byte FILTRACIA_RTC_ON_2_OD = 12;
const byte FILTRACIA_RTC_ON_2_DO = 18;

// Manualne tlacidlo filtracie: D48, aktivacia na 6 hodin.
const int TLACIDLO_FILTRACIA_6H_PIN = 48;
const unsigned long FILTRACIA_MANUAL_6H_CAS = 6UL * 60UL * 60UL * 1000UL;

// Manualne tlacidlo chrlica: D49, aktivacia na 1 hodinu.
const int TLACIDLO_CHRLIC_1H_PIN = 49;
const unsigned long CHRLIC_MANUAL_1H_CAS = 1UL * 60UL * 60UL * 1000UL;

// Softverovy debounce tlacidiel zapojenych medzi pin a GND.
const unsigned long TLACIDLO_DEBOUNCE_CAS = 50UL;


// ==================================================
// MAXIMÁLNA TEPLOTA BAZÉNA
// ==================================================

const float MAX_BAZEN_MIN = 20.0;
const float MAX_BAZEN_MAX = 35.0;
const float MAX_BAZEN_PREDVOLENA = 32.0;
float MAX_BAZEN = MAX_BAZEN_PREDVOLENA;

// Tlacidla nastavenia teploty, zapojene medzi pin a GND.
const int TLACIDLO_SET_PIN = 45;
const int TLACIDLO_PLUS_PIN = 46;
const int TLACIDLO_MINUS_PIN = 47;
const unsigned long NASTAVENIE_TEPLOTY_TIMEOUT = 10000UL;
