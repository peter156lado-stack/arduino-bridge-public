// ==================================================
// 04_REGULACIA – RIADENIE SOLÁRU
// ==================================================

bool solarZapnuty = false;
bool cakaNaVypnutieSolara = false;
unsigned long casZaciatkuVypnutiaSolara = 0;

const unsigned long CAS_VYPNUTIA_SOLARA = 300000UL;

extern bool testR9Aktivny;
extern bool manualFiltracia6h;

bool filtraciaCasovacInicializovany = false;
bool filtraciaJeVCykleON = true;
unsigned long casZmenyCyklaFiltracie = 0;
bool filtraciaRiadiRTC = false;


// ==================================================
// REGULÁCIA SOLÁRU
// ==================================================

void regulacia() {

  // ------------------------------------------------
  // HAVARIJNÝ STAV
  // ------------------------------------------------

  if (!SOLAR_CONTROL_VALID) {

    solarZapnuty = false;
    cakaNaVypnutieSolara = false;

    if (megaDiagnostickyVypis) Serial.println("REGULACIA: SOLAR_CONTROL_INVALID - SOLAR VYP");

    return;
  }

  if (teplotaBazena >= MAX_BAZEN) {

    solarZapnuty = false;
    cakaNaVypnutieSolara = false;

    if (megaDiagnostickyVypis) Serial.println("SOLAR BLOKOVANY: MAX TEPLOTA BAZENA");

    return;
  }


  // ------------------------------------------------
  // ZAPNUTIE SOLÁRU
  // Panel musí byť aspoň o nastavený rozdiel
  // teplejší ako bazén
  // ------------------------------------------------

  if (!solarZapnuty && teplotaSolarPanel >= teplotaBazena + SOLAR_ZAP) {

    solarZapnuty = true;
    cakaNaVypnutieSolara = false;

    Serial.println("SOLAR: ZAP");
  }


  // ------------------------------------------------
  // VYPNUTIE SOLÁRU
  // Rozdiel klesne pod nastavenú hodnotu
  // ------------------------------------------------

  if (solarZapnuty && teplotaSolarPanel <= teplotaBazena - 2.0) {

    solarZapnuty = false;
    cakaNaVypnutieSolara = false;

    Serial.println("SOLAR: OKAMZITE VYP");

    return;
  }

  if (solarZapnuty && teplotaSolarPanel < teplotaBazena + SOLAR_VYP) {

    if (!cakaNaVypnutieSolara) {

      cakaNaVypnutieSolara = true;
      casZaciatkuVypnutiaSolara = millis();

      Serial.println("SOLAR: START CASOVACA VYP");
    }
    else if (millis() - casZaciatkuVypnutiaSolara >= CAS_VYPNUTIA_SOLARA) {

      solarZapnuty = false;
      cakaNaVypnutieSolara = false;

      Serial.println("SOLAR: VYP PO 5 MIN");
    }
  }
  else if (cakaNaVypnutieSolara) {

    cakaNaVypnutieSolara = false;

    Serial.println("SOLAR: CASOVAC VYP ZRUSENY");
  }
}


// ==================================================
// REGULACIA FILTRACIE
// ==================================================

bool filtraciaMaBezatPodlaRTC(byte hodina) {

  return (hodina >= FILTRACIA_RTC_ON_1_OD && hodina < FILTRACIA_RTC_ON_1_DO) ||
         (hodina >= FILTRACIA_RTC_ON_2_OD && hodina < FILTRACIA_RTC_ON_2_DO);
}

void regulaciaFiltracie() {

  unsigned long teraz = millis();

  if (!filtraciaCasovacInicializovany) {

    filtraciaCasovacInicializovany = true;
    filtraciaJeVCykleON = true;
    casZmenyCyklaFiltracie = teraz;
  }

  // Docasny manualny test R9 ma prednost pred casovym rezimom.
  if (testR9Aktivny) {
    aktualizujNapajanieFiltracie(true);
    return;
  }

  byte hodina;
  byte minuta;
  byte sekunda;
  byte den;
  byte mesiac;
  byte rok;

  if (rtcCasJePlatny() && nacitajRTC(hodina, minuta, sekunda, den, mesiac, rok)) {

    filtraciaRiadiRTC = true;

    // Manual FIL 6H sa pripocitava az vo vystupnej vrstve. Jeho vlastny
    // zaciatok ani koniec preto interny Bestway power-cycle nemeni.
    nastavZakladnuPoziadavkuFiltracie(filtraciaMaBezatPodlaRTC(hodina));
    aktualizujNapajanieFiltracie(true);

    return;
  }

  filtraciaRiadiRTC = false;

  bool filtraciaMaBezatPodlaZalohy;

  if (filtraciaJeVCykleON) {

    if (teraz - casZmenyCyklaFiltracie >= FILTRACIA_CAS_ON) {

      filtraciaJeVCykleON = false;
      casZmenyCyklaFiltracie = teraz;
    }

    filtraciaMaBezatPodlaZalohy = filtraciaJeVCykleON;
  }
  else {

    if (teraz - casZmenyCyklaFiltracie >= FILTRACIA_CAS_OFF) {

      filtraciaJeVCykleON = true;
      casZmenyCyklaFiltracie = teraz;
    }

    filtraciaMaBezatPodlaZalohy = filtraciaJeVCykleON;
  }

  nastavZakladnuPoziadavkuFiltracie(filtraciaMaBezatPodlaZalohy);
  aktualizujNapajanieFiltracie(true);
}
