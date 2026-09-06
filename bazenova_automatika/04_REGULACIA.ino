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

enum FiltraciaZdroj : byte {
  FIL_ZDROJ_NONE,
  FIL_ZDROJ_RTC_FIXED,
  FIL_ZDROJ_SOLAR_EXTRA,
  FIL_ZDROJ_FALLBACK_MILLIS,
  FIL_ZDROJ_MANUAL,
  FIL_ZDROJ_TEST
};

FiltraciaZdroj filtraciaAutomatickyZdroj = FIL_ZDROJ_NONE;
FiltraciaZdroj poslednyVypisanyZdrojFiltracie = FIL_ZDROJ_NONE;
bool filtraciaZdrojBolVypisany = false;

unsigned long filtraciaAutoDenneMs = 0;
unsigned long filtraciaSolarExtraDenneMs = 0;
unsigned long filtraciaAutoPoslednaAktualizaciaMs = 0;
bool filtraciaAutoPocitadloInicializovane = false;
bool filtraciaAutoPoziadavkaAktivna = false;
bool filtraciaSolarExtraPoziadavkaAktivna = false;

bool filtraciaDatumInicializovany = false;
byte filtraciaDatumDen = 0;
byte filtraciaDatumMesiac = 0;
byte filtraciaDatumRok = 0;
bool filtraciaDennyLimitBolDosiahnuty = false;
bool filtraciaSolarExtraLimitBolDosiahnuty = false;


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

  return hodina >= FILTRACIA_RTC_FIXED_OD &&
         hodina < FILTRACIA_RTC_FIXED_DO;
}

void pripocitajFiltracnyCas(unsigned long &pocitadlo,
                            unsigned long prirastok,
                            unsigned long maximum) {

  if (pocitadlo >= maximum) return;

  unsigned long zostava = maximum - pocitadlo;
  pocitadlo += prirastok >= zostava ? zostava : prirastok;
}

void aktualizujDennePocitadloFiltracie(unsigned long teraz) {

  if (!filtraciaAutoPocitadloInicializovane) {
    filtraciaAutoPocitadloInicializovane = true;
    filtraciaAutoPoslednaAktualizaciaMs = teraz;
    return;
  }

  // Unsigned subtraction zachovava korektne meranie aj cez millis rollover.
  unsigned long uplynulo = teraz - filtraciaAutoPoslednaAktualizaciaMs;
  filtraciaAutoPoslednaAktualizaciaMs = teraz;

  if (filtraciaAutoPoziadavkaAktivna) {
    pripocitajFiltracnyCas(filtraciaAutoDenneMs, uplynulo,
                          FILTRACIA_AUTO_DENNY_LIMIT);
  }

  if (filtraciaSolarExtraPoziadavkaAktivna) {
    pripocitajFiltracnyCas(filtraciaSolarExtraDenneMs, uplynulo,
                          FILTRACIA_SOLAR_EXTRA_DENNY_LIMIT);
  }
}

void aktualizujDatumFiltracie(byte den, byte mesiac, byte rok) {

  if (!filtraciaDatumInicializovany) {
    filtraciaDatumInicializovany = true;
    filtraciaDatumDen = den;
    filtraciaDatumMesiac = mesiac;
    filtraciaDatumRok = rok;
    return;
  }

  if (den == filtraciaDatumDen && mesiac == filtraciaDatumMesiac &&
      rok == filtraciaDatumRok) {
    return;
  }

  filtraciaDatumDen = den;
  filtraciaDatumMesiac = mesiac;
  filtraciaDatumRok = rok;
  filtraciaAutoDenneMs = 0;
  filtraciaSolarExtraDenneMs = 0;
  filtraciaDennyLimitBolDosiahnuty = false;
  filtraciaSolarExtraLimitBolDosiahnuty = false;
  Serial.println(F("FIL DAILY: RESET NEW DATE"));
}

void aktualizujLimityFiltracie() {

  bool dennyLimit = filtraciaAutoDenneMs >= FILTRACIA_AUTO_DENNY_LIMIT;
  bool solarExtraLimit =
      filtraciaSolarExtraDenneMs >= FILTRACIA_SOLAR_EXTRA_DENNY_LIMIT;

  if (dennyLimit && !filtraciaDennyLimitBolDosiahnuty) {
    Serial.println(F("FIL DAILY LIMIT REACHED"));
  }

  if (solarExtraLimit && !filtraciaSolarExtraLimitBolDosiahnuty) {
    Serial.println(F("FIL SOLAR EXTRA LIMIT REACHED"));
  }

  filtraciaDennyLimitBolDosiahnuty = dennyLimit;
  filtraciaSolarExtraLimitBolDosiahnuty = solarExtraLimit;
}

byte aktualnyZdrojFiltracie() {

  if (testR9Aktivny) return FIL_ZDROJ_TEST;
  if (manualFiltracia6h) return FIL_ZDROJ_MANUAL;
  return filtraciaAutomatickyZdroj;
}

void aktualizujDiagnostikuZdrojaFiltracie() {

  FiltraciaZdroj zdroj = (FiltraciaZdroj)aktualnyZdrojFiltracie();
  if (filtraciaZdrojBolVypisany && zdroj == poslednyVypisanyZdrojFiltracie) {
    return;
  }

  filtraciaZdrojBolVypisany = true;
  poslednyVypisanyZdrojFiltracie = zdroj;
  Serial.print(F("FIL SOURCE: "));

  switch (zdroj) {
    case FIL_ZDROJ_RTC_FIXED: Serial.println(F("RTC_FIXED")); break;
    case FIL_ZDROJ_SOLAR_EXTRA: Serial.println(F("SOLAR_EXTRA")); break;
    case FIL_ZDROJ_FALLBACK_MILLIS: Serial.println(F("FALLBACK_MILLIS")); break;
    case FIL_ZDROJ_MANUAL: Serial.println(F("MANUAL")); break;
    case FIL_ZDROJ_TEST: Serial.println(F("TEST")); break;
    default: Serial.println(F("NONE")); break;
  }
}

void vypisFiltraciaDenne() {

  if (!filtraciaRiadiRTC || !filtraciaDatumInicializovany) {
    Serial.println(F("FIL DAILY: RTC INVALID / LIMIT NOT GUARANTEED"));
    return;
  }

  unsigned long minuty = filtraciaAutoDenneMs / 60000UL;
  byte hodiny = minuty / 60UL;
  byte zvysneMinuty = minuty % 60UL;

  Serial.print(F("FIL DAILY: "));
  if (hodiny < 10) Serial.print('0');
  Serial.print(hodiny);
  Serial.print(':');
  if (zvysneMinuty < 10) Serial.print('0');
  Serial.print(zvysneMinuty);
  Serial.println(F(" / 10:00"));
}

void regulaciaFiltracie() {

  unsigned long teraz = millis();

  // Najprv pripocitaj cas predchadzajucej automatickej poziadavky. Pocitadlo
  // sa nikdy nesnazi spatne dopoctat dobu pred bootom alebo vypadkom RTC.
  aktualizujDennePocitadloFiltracie(teraz);

  if (!filtraciaCasovacInicializovany) {

    filtraciaCasovacInicializovany = true;
    filtraciaJeVCykleON = true;
    casZmenyCyklaFiltracie = teraz;
  }

  byte hodina;
  byte minuta;
  byte sekunda;
  byte den;
  byte mesiac;
  byte rok;

  if (rtcCasJePlatny() && nacitajRTC(hodina, minuta, sekunda, den, mesiac, rok)) {

    filtraciaRiadiRTC = true;

    aktualizujDatumFiltracie(den, mesiac, rok);
    aktualizujLimityFiltracie();

    bool rtcFixed = filtraciaMaBezatPodlaRTC(hodina);
    bool solarExtraPovolene =
        solarZapnuty && !rtcFixed &&
        filtraciaAutoDenneMs < FILTRACIA_AUTO_DENNY_LIMIT &&
        filtraciaSolarExtraDenneMs < FILTRACIA_SOLAR_EXTRA_DENNY_LIMIT;

    filtraciaAutoPoziadavkaAktivna = rtcFixed || solarExtraPovolene;
    filtraciaSolarExtraPoziadavkaAktivna = solarExtraPovolene;
    filtraciaAutomatickyZdroj = rtcFixed ? FIL_ZDROJ_RTC_FIXED :
        (solarExtraPovolene ? FIL_ZDROJ_SOLAR_EXTRA : FIL_ZDROJ_NONE);

    // Manual FIL 6H sa pripocitava az vo vystupnej vrstve. Jeho vlastny
    // zaciatok ani koniec preto denny limit ani Bestway power-cycle nemeni.
    nastavZakladnuPoziadavkuFiltracie(filtraciaAutoPoziadavkaAktivna);
    aktualizujNapajanieFiltracie(true);

    if (megaDiagnostickyVypis) vypisFiltraciaDenne();

    return;
  }

  filtraciaRiadiRTC = false;
  filtraciaAutoPoziadavkaAktivna = false;
  filtraciaSolarExtraPoziadavkaAktivna = false;

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

  // Bez platneho RTC sa nevytvara fiktivny kalendarny den. Povodny 6 h
  // ON/6 h OFF millis fallback zostava bezo zmeny a 10 h limit negarantuje.
  filtraciaAutomatickyZdroj = filtraciaMaBezatPodlaZalohy ?
      FIL_ZDROJ_FALLBACK_MILLIS : FIL_ZDROJ_NONE;
  nastavZakladnuPoziadavkuFiltracie(filtraciaMaBezatPodlaZalohy);
  aktualizujNapajanieFiltracie(true);

  if (megaDiagnostickyVypis) vypisFiltraciaDenne();
}
