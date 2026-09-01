// ==================================================
// 05_VYSTUPY – FYZICKÉ VÝSTUPY
// ==================================================


// --------------------------------------------------
// DEFINÍCIE RELÉ
// --------------------------------------------------

// Fyzicke kanaly 1-4 existujucej 16R dosky patria BASIC vrstve.
// Mega ich nesmie ovladat; povodna vazba R1-R4 na D30-D33 bola odstranena.
// D30 je XKC commissioning vstup. D31-D33 maju samostatne fyzicke funkcie
// mimo 16R dosky.

const byte MEGA_TOTAL_STOP_PIN = 32;
// W1209_FACKOVAC - Mega D33
const byte W1209_SUPERVISION_RELAY_PIN = 33;

// Fyzicke kanaly 5-8 zostavaju v reléovej casti Mega/SMART.
const int R5  = 34;
const int R6  = 35;
const int R7  = 36;
const int R8  = 37;

// Fyzicke kanaly 9-16 zostavaju v reléovej casti Mega/SMART.
const int R9  = 22;  // filtracia
const int R10 = 23;  // solar / chrlic
const int R11 = 24;
const int R12 = 25;
const int R13 = 26;
const int R14 = 27;
const int R15 = 28;
const int R16 = 29;

bool filtraciaZapnuta = false;
extern bool chrlicManualAktivny;
extern bool manualFiltracia6h;
extern bool testR9Aktivny;

// Vysledna poziadavka z RTC alebo millis fallbacku. Manual FIL 6H a servisny
// test sa pripocitavaju samostatne, aby ich vlastne casovace zostali nezavisle.
bool filtraciaZakladnaPoziadavka = false;

// Jediny agregacny bod runtime poziadavky pre D32. Dalsie samostatne
// schvalene explicitne TOTAL STOP dovody sa smu v buducnosti pridat iba OR.
bool megaTotalStopRequest() {
  return megaXkcTrip;
}

// Jediny zapisovatel fyzickeho MEGA_TOTAL_STOP vystupu.
void aktualizujMegaTotalStopVystup() {
  digitalWrite(MEGA_TOTAL_STOP_PIN, megaTotalStopRequest() ? HIGH : LOW);
}

enum FiltraciaNapajanieStav : byte {
  FIL_RUN_NORMAL,
  FIL_RESET_OFF
};

FiltraciaNapajanieStav filtraciaNapajanieStav = FIL_RUN_NORMAL;
bool filtraciaSuviseOnCasBezi = false;
unsigned long filtraciaSuviseOnOdMs = 0;
unsigned long filtraciaResetOffOdMs = 0;


// --------------------------------------------------
// INICIALIZÁCIA VÝSTUPOV
// --------------------------------------------------

void inicializaciaVystupov() {

  // Energize-to-trip: bez potvrdeneho XKC tripu je boot/reset LOW / COM-NC.
  aktualizujMegaTotalStopVystup();
  pinMode(MEGA_TOTAL_STOP_PIN, OUTPUT);
  aktualizujMegaTotalStopVystup();

  // W1209 supervision ma autoritu iba v budúcom explicitnom BASIC mode.
  // Aktualny kod nema autoritativny SYSTEM_MODE ani potvrdenie realneho
  // dodavania tepla BASIC vetvou, preto pin zostava bez zasahu v LOW/COM-NC.
  digitalWrite(W1209_SUPERVISION_RELAY_PIN, LOW);
  pinMode(W1209_SUPERVISION_RELAY_PIN, OUTPUT);
  digitalWrite(W1209_SUPERVISION_RELAY_PIN, LOW);

  pinMode(R9, OUTPUT);
  pinMode(R10, OUTPUT);

  // Bezpečný základný stav
  // HIGH = relé VYPNUTÉ
  digitalWrite(R9, HIGH);
  filtraciaZapnuta = false;
  filtraciaZakladnaPoziadavka = false;
  filtraciaNapajanieStav = FIL_RUN_NORMAL;
  filtraciaSuviseOnCasBezi = false;
  digitalWrite(R10, HIGH);
  Serial.println("R9 FILTRACIA -> OFF [zdroj: inicializacia]");
}

void zapniFiltraciu(const char *zdroj) {

  if (!filtraciaZapnuta) {

    filtraciaZapnuta = true;
    digitalWrite(R9, LOW);
    filtraciaSuviseOnOdMs = millis();
    filtraciaSuviseOnCasBezi = true;

    Serial.print("R9 FILTRACIA -> ON [zdroj: ");
    Serial.print(zdroj);
    Serial.println("]");
  }
}

void vypniFiltraciu(const char *zdroj) {

  if (filtraciaZapnuta) {

    filtraciaZapnuta = false;
    digitalWrite(R9, HIGH);
    filtraciaSuviseOnCasBezi = false;

    Serial.print("R9 FILTRACIA -> OFF [zdroj: ");
    Serial.print(zdroj);
    Serial.println("]");
  }
}

bool filtraciaJePoziadovana() {

  return filtraciaZakladnaPoziadavka || manualFiltracia6h || testR9Aktivny;
}

void nastavZakladnuPoziadavkuFiltracie(bool poziadavka) {

  filtraciaZakladnaPoziadavka = poziadavka;
}

// Neblokujuca vrstva medzi vyslednou poziadavkou a fyzickym R9/D22.
// Start resetu sa povoluje az po cerstvom vyhodnoteni RTC/fallback poziadavky
// v regulaciaFiltracie(). Samotne 2 s OFF a zrusenie po strate poziadavky sa
// obsluhuju v kazdom loop priechode.
void aktualizujNapajanieFiltracie(bool povolitStartResetu) {

  const unsigned long teraz = millis();
  const bool poziadavka = filtraciaJePoziadovana();

  if (filtraciaNapajanieStav == FIL_RESET_OFF) {

    if (!poziadavka) {
      filtraciaNapajanieStav = FIL_RUN_NORMAL;
      filtraciaResetOffOdMs = 0;
      Serial.println(F("FIL RESET: CANCELLED - NO DEMAND"));
      return;
    }

    if (teraz - filtraciaResetOffOdMs >= FILTRACIA_RESET_OFF_CAS) {
      filtraciaNapajanieStav = FIL_RUN_NORMAL;
      filtraciaResetOffOdMs = 0;
      zapniFiltraciu("internal 6h timer continue");
      Serial.println(F("FIL RESET: ON / CONTINUE"));
    }
    return;
  }

  if (!poziadavka) {
    vypniFiltraciu("ziadna aktivna poziadavka");
    return;
  }

  if (!filtraciaZapnuta) {
    zapniFiltraciu("agregovana poziadavka");
  }

  if (povolitStartResetu && filtraciaSuviseOnCasBezi &&
      teraz - filtraciaSuviseOnOdMs >= FILTRACIA_MAX_SUVISLE_ON) {
    Serial.println(F("FIL RESET: INTERNAL 6H TIMER"));
    filtraciaNapajanieStav = FIL_RESET_OFF;
    filtraciaResetOffOdMs = teraz;
    vypniFiltraciu("internal 6h timer reset");
    Serial.println(F("FIL RESET: OFF"));
  }
}


// --------------------------------------------------
// OVLÁDANIE VÝSTUPOV
// --------------------------------------------------

void vystupy() {

  // ------------------------------------------------
  // HAVARIA
  // ------------------------------------------------

  if (!SOLAR_CONTROL_VALID) {

    digitalWrite(R10, HIGH);

    if (megaDiagnostickyVypis) Serial.println("!!! SOLAR_CONTROL_INVALID - R10 VYPNUTE !!!");

  }


  // ------------------------------------------------
  // NORMÁLNA PREVÁDZKA
  // ------------------------------------------------

  else if (solarZapnuty || chrlicManualAktivny) {

    digitalWrite(R10, LOW);

  }
  else {

    digitalWrite(R10, HIGH);

  }

  if (megaDiagnostickyVypis) {
  Serial.print("SOLAR: ");
  Serial.println(solarZapnuty ? "ON" : "OFF");

  Serial.print("FILTRACIA: ");
  Serial.print(filtraciaZapnuta ? "ON" : "OFF");
  Serial.println(filtraciaRiadiRTC ? " [RTC]" : " [ZALOHA millis]");
  }
}
