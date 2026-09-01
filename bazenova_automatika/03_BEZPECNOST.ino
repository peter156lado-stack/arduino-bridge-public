// ==================================================
// 03_BEZPECNOST – KONTROLA SENZOROV A SYSTÉMU
// ==================================================

bool T1_OK;
bool T2_OK;
bool T3_OK;
bool T4_OK;

bool system_OK;
bool systemDegradovany;
float teplotaBazena;

// Funkcne validity oddeluju poruchu jedneho senzora od ostatnych funkcii.
bool POOL_TEMP_VALID;
bool SOLAR_CONTROL_VALID;
bool FIL_CONTROL_VALID;
bool TBOX_DIAG_VALID;
bool T2_EFFECTIVE_VALID;
bool T3_EFFECTIVE_VALID;
float teplotaSolarVystup;
float teplotaSolarPanel;
ZdrojHodnoty zdrojTeplotyBazena = ZDROJ_ZIADNY;
ZdrojHodnoty zdrojT2 = ZDROJ_ZIADNY;
ZdrojHodnoty zdrojT3 = ZDROJ_ZIADNY;

extern bool REMOTE_DATA_VALID;
extern bool unoAgreementOnRemote;
extern bool T1_CONFLICT;
extern bool T2_CONFLICT;
extern bool MEGA_T1_SUSPECT;
extern bool UNO_T1_SUSPECT;
extern bool MEGA_T2_SUSPECT;
extern bool UNO_T2_SUSPECT;
extern bool unoLinkStavOk;
extern bool XKC_CONFLICT;
extern bool megaXkcTrip;
bool megaSmartPodmienkyOk();

// Lokalny potvrdeny XKC trip je explicitna STOP podmienka Mega. Vzdialeny
// XKC stav ani diagnosticky konflikt sem nevstupuju.
bool explicitnySystemStopAktivny() {
  return megaXkcTrip;
}

const __FlashStringHelper *nazovSystemMode(SystemMode mode) {
  switch (mode) {
    case MODE_SMART: return F("SMART");
    case MODE_DEGRADED: return F("DEGRADED");
    case MODE_BASIC: return F("BASIC");
    case MODE_STOP: return F("STOP");
    default: return F("BASIC");
  }
}

const __FlashStringHelper *dovodSystemMode(SystemMode mode) {
  if (mode == MODE_STOP) return F("EXPLICIT_STOP");

  if (mode == MODE_BASIC) {
    if (!unoLinkStavOk) return F("UNO_LINK_CHYBA");
    if (!REMOTE_DATA_VALID) return F("REMOTE_STALE");
    if (!megaAgreementOn) return F("MEGA_AGREEMENT_OFF");
    if (!unoAgreementOnRemote) return F("UNO_AGREEMENT_OFF");
    return F("SMART_NEDOVERYHODNY");
  }

  if (mode == MODE_DEGRADED) {
    if (zdrojTeplotyBazena == ZDROJ_LOKALNY_FALLBACK) return F("POOL_LOCAL_FALLBACK");
    if (zdrojTeplotyBazena == ZDROJ_VZDIALENY_FALLBACK) return F("POOL_REMOTE_FALLBACK");
    if (!T2_OK && T2_EFFECTIVE_VALID) return F("T2_REMOTE_FALLBACK");
    if (!T2_OK) return F("T2_LOCAL_CHYBA");
    if (!T3_OK && T3_EFFECTIVE_VALID) return F("T3_REMOTE_FALLBACK");
    if (!T3_OK) return F("T3_LOCAL_CHYBA");
    if (!MEGA_TBOX_OK) return F("MEGA_TBOX_CHYBA");
    return F("SCHVALENA_DEGRADACIA");
  }

  return F("FULL_SMART");
}

void aktualizujSystemMode() {
  SystemMode novyMode;

  // Pevna priorita: STOP > BASIC > DEGRADED > SMART.
  if (explicitnySystemStopAktivny()) {
    novyMode = MODE_STOP;
  }
  else {
    const bool smartAutoritaPotvrdena = megaAgreementOn &&
                                         unoAgreementOnRemote &&
                                         unoLinkStavOk &&
                                         REMOTE_DATA_VALID;
    if (!smartAutoritaPotvrdena) novyMode = MODE_BASIC;
    else if (systemDegradovany) novyMode = MODE_DEGRADED;
    else novyMode = MODE_SMART;
  }

  if (novyMode == systemMode) return;
  systemMode = novyMode;
  Serial.print(F("EVENT: SYSTEM_MODE="));
  Serial.print(nazovSystemMode(systemMode));
  Serial.print(F(" REASON="));
  Serial.println(dovodSystemMode(systemMode));
}

void vypisSystemMode() {
  Serial.print(F("SYSTEM_MODE="));
  Serial.print(nazovSystemMode(systemMode));
  Serial.print(F(" REASON="));
  Serial.println(dovodSystemMode(systemMode));
}

enum MegaProblemKod : byte {
  MEGA_PROBLEM_NONE,
  MEGA_PROBLEM_T1,
  MEGA_PROBLEM_T2,
  MEGA_PROBLEM_T3,
  MEGA_PROBLEM_T4_ADRESA,
  MEGA_PROBLEM_T4,
  MEGA_PROBLEM_TBOX,
  MEGA_PROBLEM_AHT10,
  MEGA_PROBLEM_RTC,
  MEGA_PROBLEM_SONAR_TIMEOUT,
  MEGA_PROBLEM_SONAR_CHYBA,
  MEGA_PROBLEM_POOL_INVALID,
  MEGA_PROBLEM_T2_INVALID,
  MEGA_PROBLEM_SOLAR_INVALID,
  MEGA_PROBLEM_FIL_INVALID,
  MEGA_PROBLEM_TBOX_DIAG_INVALID,
  MEGA_PROBLEM_REMOTE_STALE,
  MEGA_PROBLEM_UNO_LINK,
  MEGA_PROBLEM_AGREEMENT,
  MEGA_PROBLEM_T1_CONFLICT,
  MEGA_PROBLEM_T2_CONFLICT,
  MEGA_PROBLEM_MEGA_T1_SUSPECT,
  MEGA_PROBLEM_UNO_T1_SUSPECT,
  MEGA_PROBLEM_MEGA_T2_SUSPECT,
  MEGA_PROBLEM_UNO_T2_SUSPECT
};

byte megaProblemPodlaPoradia(byte hladanePoradie) {
  byte poradie = 0;
#define VYBER_PROBLEM(podmienka, kod) \
  do { if (podmienka) { if (poradie == hladanePoradie) return kod; poradie++; } } while (0)
  VYBER_PROBLEM(!T1_OK, MEGA_PROBLEM_T1);
  VYBER_PROBLEM(!T2_OK, MEGA_PROBLEM_T2);
  VYBER_PROBLEM(!T3_OK, MEGA_PROBLEM_T3);
  VYBER_PROBLEM(!T4_ADRESA_NASTAVENA, MEGA_PROBLEM_T4_ADRESA);
  VYBER_PROBLEM(T4_ADRESA_NASTAVENA && !T4_OK, MEGA_PROBLEM_T4);
  VYBER_PROBLEM(!MEGA_TBOX_OK, MEGA_PROBLEM_TBOX);
  VYBER_PROBLEM(!aht10Dostupny, MEGA_PROBLEM_AHT10);
  VYBER_PROBLEM(!rtcDostupne, MEGA_PROBLEM_RTC);
  VYBER_PROBLEM(megaSonarStav == MEGA_SONAR_TIMEOUT, MEGA_PROBLEM_SONAR_TIMEOUT);
  VYBER_PROBLEM(megaSonarStav == MEGA_SONAR_CHYBA, MEGA_PROBLEM_SONAR_CHYBA);
  VYBER_PROBLEM(!POOL_TEMP_VALID, MEGA_PROBLEM_POOL_INVALID);
  VYBER_PROBLEM(!T2_EFFECTIVE_VALID, MEGA_PROBLEM_T2_INVALID);
  VYBER_PROBLEM(!SOLAR_CONTROL_VALID, MEGA_PROBLEM_SOLAR_INVALID);
  VYBER_PROBLEM(!FIL_CONTROL_VALID, MEGA_PROBLEM_FIL_INVALID);
  VYBER_PROBLEM(!TBOX_DIAG_VALID, MEGA_PROBLEM_TBOX_DIAG_INVALID);
  VYBER_PROBLEM(!REMOTE_DATA_VALID, MEGA_PROBLEM_REMOTE_STALE);
  VYBER_PROBLEM(!unoLinkStavOk, MEGA_PROBLEM_UNO_LINK);
  VYBER_PROBLEM(!megaAgreementOn && !megaSmartPodmienkyOk(), MEGA_PROBLEM_AGREEMENT);
  VYBER_PROBLEM(T1_CONFLICT, MEGA_PROBLEM_T1_CONFLICT);
  VYBER_PROBLEM(T2_CONFLICT, MEGA_PROBLEM_T2_CONFLICT);
  VYBER_PROBLEM(MEGA_T1_SUSPECT, MEGA_PROBLEM_MEGA_T1_SUSPECT);
  VYBER_PROBLEM(UNO_T1_SUSPECT, MEGA_PROBLEM_UNO_T1_SUSPECT);
  VYBER_PROBLEM(MEGA_T2_SUSPECT, MEGA_PROBLEM_MEGA_T2_SUSPECT);
  VYBER_PROBLEM(UNO_T2_SUSPECT, MEGA_PROBLEM_UNO_T2_SUSPECT);
#undef VYBER_PROBLEM
  return MEGA_PROBLEM_NONE;
}

byte pocetMegaProblemov() {
  byte pocet = 0;
  while (megaProblemPodlaPoradia(pocet) != MEGA_PROBLEM_NONE) pocet++;
  return pocet;
}

bool megaMaAktivnyProblem() {
  return megaProblemPodlaPoradia(0) != MEGA_PROBLEM_NONE;
}

const __FlashStringHelper *nazovMegaProblemu(byte kod) {
  switch (kod) {
    case MEGA_PROBLEM_T1: return F("T1_CHYBA");
    case MEGA_PROBLEM_T2: return F("T2_CHYBA");
    case MEGA_PROBLEM_T3: return F("T3_CHYBA");
    case MEGA_PROBLEM_T4_ADRESA: return F("T4_ADRESA_NENASTAVENA");
    case MEGA_PROBLEM_T4: return F("T4_CHYBA");
    case MEGA_PROBLEM_TBOX: return F("MEGA_TBOX_CHYBA");
    case MEGA_PROBLEM_AHT10: return F("AHT10_CHYBA");
    case MEGA_PROBLEM_RTC: return F("RTC_CHYBA");
    case MEGA_PROBLEM_SONAR_TIMEOUT: return F("MEGA_SONAR_TIMEOUT");
    case MEGA_PROBLEM_SONAR_CHYBA: return F("MEGA_SONAR_CHYBA");
    case MEGA_PROBLEM_POOL_INVALID: return F("POOL_INVALID");
    case MEGA_PROBLEM_T2_INVALID: return F("T2_EFFECTIVE_INVALID");
    case MEGA_PROBLEM_SOLAR_INVALID: return F("SOLAR_CONTROL_INVALID");
    case MEGA_PROBLEM_FIL_INVALID: return F("FIL_CONTROL_INVALID");
    case MEGA_PROBLEM_TBOX_DIAG_INVALID: return F("TBOX_DIAG_DATA_INVALID");
    case MEGA_PROBLEM_REMOTE_STALE: return F("REMOTE_STALE");
    case MEGA_PROBLEM_UNO_LINK: return F("UNO_LINK_CHYBA");
    case MEGA_PROBLEM_AGREEMENT: return F("MEGA_AGREEMENT_OFF");
    case MEGA_PROBLEM_T1_CONFLICT: return F("T1_CONFLICT");
    case MEGA_PROBLEM_T2_CONFLICT: return F("T2_CONFLICT");
    case MEGA_PROBLEM_MEGA_T1_SUSPECT: return F("MEGA_T1_SUSPECT");
    case MEGA_PROBLEM_UNO_T1_SUSPECT: return F("UNO_T1_SUSPECT");
    case MEGA_PROBLEM_MEGA_T2_SUSPECT: return F("MEGA_T2_SUSPECT");
    case MEGA_PROBLEM_UNO_T2_SUSPECT: return F("UNO_T2_SUSPECT");
    default: return F("NONE");
  }
}

void vypisMegaProblemSummary() {
  Serial.print(F("PROBLEM: "));
  const byte pocet = pocetMegaProblemov();
  if (pocet == 0) {
    Serial.println(F("NONE"));
    return;
  }
  for (byte i = 0; i < pocet; i++) {
    if (i > 0) Serial.print(F(" | "));
    Serial.print(nazovMegaProblemu(megaProblemPodlaPoradia(i)));
  }
  Serial.println();
}

void bezpecnost() {

  // ------------------------------------------------
  // KONTROLA ROZSAHU SENZOROV
  // ------------------------------------------------

  T1_OK = (t1 > -10 && t1 < 60);
  T2_OK = (t2 > -10 && t2 < 100);
  T3_OK = (t3 > -10 && t3 < 100);
  T4_OK = T4_ADRESA_NASTAVENA && (t4 > -10 && t4 < 60);

  // ------------------------------------------------
  // CELKOVÝ STAV SYSTÉMU
  // ------------------------------------------------

  zdrojTeplotyBazena = ZDROJ_ZIADNY;
  if (T1_OK) {
    teplotaBazena = t1;
    zdrojTeplotyBazena = ZDROJ_LOKALNY_PRIMARNY;
  }
  else if (T4_OK) {
    teplotaBazena = t4;
    zdrojTeplotyBazena = ZDROJ_LOKALNY_FALLBACK;
  }
  else if (unoRemoteT1Platna()) {
    teplotaBazena = unoRemoteT1Hodnota();
    zdrojTeplotyBazena = ZDROJ_VZDIALENY_FALLBACK;
  }
  else {
    teplotaBazena = DEVICE_DISCONNECTED_C;
  }

  POOL_TEMP_VALID = zdrojTeplotyBazena != ZDROJ_ZIADNY;

  zdrojT2 = ZDROJ_ZIADNY;
  if (T2_OK) {
    teplotaSolarVystup = t2;
    zdrojT2 = ZDROJ_LOKALNY_PRIMARNY;
  }
  else if (unoRemoteT2Platna()) {
    teplotaSolarVystup = unoRemoteT2Hodnota();
    zdrojT2 = ZDROJ_VZDIALENY_FALLBACK;
  }
  else {
    teplotaSolarVystup = DEVICE_DISCONNECTED_C;
  }

  T2_EFFECTIVE_VALID = zdrojT2 != ZDROJ_ZIADNY;

  zdrojT3 = ZDROJ_ZIADNY;
  if (T3_OK) {
    teplotaSolarPanel = t3;
    zdrojT3 = ZDROJ_LOKALNY_PRIMARNY;
  }
  else if (unoRemoteT3Platna()) {
    teplotaSolarPanel = unoRemoteT3Hodnota();
    zdrojT3 = ZDROJ_VZDIALENY_FALLBACK;
  }
  else {
    teplotaSolarPanel = DEVICE_DISCONNECTED_C;
  }

  T3_EFFECTIVE_VALID = zdrojT3 != ZDROJ_ZIADNY;
  SOLAR_CONTROL_VALID = POOL_TEMP_VALID && T3_EFFECTIVE_VALID;
  FIL_CONTROL_VALID = true;  // RTC ma lokalny millis fallback; safety sa tu zatial nemeni.
  TBOX_DIAG_VALID = MEGA_TBOX_OK || unoRemoteTboxPlatna();
  systemDegradovany = zdrojTeplotyBazena != ZDROJ_LOKALNY_PRIMARNY ||
                      !T2_OK || !T3_OK || !MEGA_TBOX_OK || !REMOTE_DATA_VALID;

  // Povodny globalny stav zostava iba pre kompatibilnu diagnostiku/HMI.
  system_OK = (T1_OK || T4_OK) && T2_OK && T3_EFFECTIVE_VALID;

  // ------------------------------------------------
  // VÝPIS STAVU
  // ------------------------------------------------

  if (megaDiagnostickyVypis) {
  Serial.print("T1: ");

  if (T1_OK)
    Serial.println("OK");
  else
    Serial.println("CHYBA");

  Serial.print("T2: ");

  if (T2_OK)
    Serial.println("OK");
  else
    Serial.println("CHYBA");

  Serial.print("T3: ");

  if (T3_OK)
    Serial.println("OK");
  else
    Serial.println("CHYBA");

  Serial.print("T4: ");

  if (!T4_ADRESA_NASTAVENA)
    Serial.println("ADRESA NENASTAVENA");
  else if (T4_OK)
    Serial.println("OK");
  else
    Serial.println("CHYBA");

  Serial.print("TEPLOTA BAZENA: ");
  Serial.print(teplotaBazena);
  Serial.print(" C [");
  Serial.print(textZdroja(zdrojTeplotyBazena));
  Serial.println("]");

  Serial.print("T2 EFEKTIVNA: ");
  if (T2_EFFECTIVE_VALID) Serial.print(teplotaSolarVystup);
  else Serial.print("--");
  Serial.print(" C [");
  Serial.print(textZdroja(zdrojT2));
  Serial.println("]");

  Serial.print("T3 EFEKTIVNA: ");
  if (T3_EFFECTIVE_VALID) Serial.print(teplotaSolarPanel);
  else Serial.print("--");
  Serial.print(" C [");
  if (zdrojT3 == ZDROJ_LOKALNY_PRIMARNY) Serial.print("MEGA_T3_PRIMARY");
  else if (zdrojT3 == ZDROJ_VZDIALENY_FALLBACK) Serial.print("UNO_T3_REMOTE_FALLBACK");
  else Serial.print("INVALID");
  Serial.println("]");

  Serial.print("FUNKCIE: POOL=");
  Serial.print(POOL_TEMP_VALID ? "OK" : "INVALID");
  Serial.print(" SOLAR=");
  Serial.print(SOLAR_CONTROL_VALID ? "OK" : "INVALID");
  Serial.print(" FIL=");
  Serial.print(FIL_CONTROL_VALID ? "OK" : "INVALID");
  Serial.print(F(" TBOX_DIAG_DATA="));
  Serial.print(TBOX_DIAG_VALID ? "OK" : "INVALID");
  Serial.print(" REMOTE=");
  Serial.println(REMOTE_DATA_VALID ? "OK" : "REMOTE_STALE");

  Serial.print("REMOTE AGE=");
  if (REMOTE_DATA_VALID) Serial.print(vekUnoRemoteDatMs());
  else Serial.print("--");
  Serial.print("ms LINK=");
  Serial.println(unoLinkStavOk ? "OK" : "CHYBA");

  Serial.print(F("MEGA_XKC="));
  Serial.print(megaXkcLowWater ? F("LOW_WATER") : F("WATER"));
  Serial.print(F(" UNO_XKC_REMOTE="));
  if (unoRemoteXkcPlatny())
    Serial.print(unoRemoteXkcLowWater() ? F("LOW_WATER") : F("WATER"));
  else
    Serial.print(F("STALE"));
  Serial.print(F(" XKC_CONFLICT="));
  if (!unoRemoteXkcPlatny()) Serial.print(F("NA"));
  else Serial.print(XKC_CONFLICT ? F("YES") : F("NO"));
  Serial.print(F(" XKC_CONFIRM="));
  Serial.print(megaXkcConfirmSekundy());
  Serial.print(F("s XKC_TRIP="));
  Serial.print(megaXkcTrip ? F("YES") : F("NO"));
  Serial.print(F(" XKC_RECOVERY="));
  Serial.print(megaXkcRecoverySekundy());
  Serial.println(F("s"));

  Serial.print("REMOTE: UNO_T1=");
  if (unoRemoteT1Platna()) Serial.print(unoRemoteT1Hodnota());
  else Serial.print("--");
  Serial.print(" UNO_T2=");
  if (unoRemoteT2Platna()) Serial.print(unoRemoteT2Hodnota());
  else Serial.print("--");
  Serial.print(" UNO_T3=");
  if (unoRemoteT3Platna()) Serial.print(unoRemoteT3Hodnota());
  else Serial.print("--");
  Serial.print(" UNO_TBOX=");
  if (unoRemoteTboxPlatna()) Serial.print(unoRemoteTboxHodnota());
  else Serial.print("--");
  Serial.print(" UNO_SONAR=");
  if (unoRemoteSonarPlatny()) Serial.print(unoRemoteSonarHodnota());
  else Serial.print("--");
  Serial.println();

  Serial.print("DIAG: T1=");
  Serial.print(T1_CONFLICT ? "CONFLICT" : "OK");
  Serial.print(" T2=");
  Serial.print(T2_CONFLICT ? "CONFLICT" : "OK");
  Serial.print(" SUSPECT=");
  if (MEGA_T1_SUSPECT) Serial.print("MEGA_T1 ");
  if (UNO_T1_SUSPECT) Serial.print("UNO_T1 ");
  if (MEGA_T2_SUSPECT) Serial.print("MEGA_T2 ");
  if (UNO_T2_SUSPECT) Serial.print("UNO_T2 ");
  if (!MEGA_T1_SUSPECT && !UNO_T1_SUSPECT && !MEGA_T2_SUSPECT && !UNO_T2_SUSPECT) Serial.print("NONE");
  Serial.println();

  Serial.print("SYSTEM: ");

  if (!system_OK)
    Serial.println("HAVARIA");
  else if (systemDegradovany)
    Serial.println("DEGRADED");
  else
    Serial.println("OK");
  }
}
