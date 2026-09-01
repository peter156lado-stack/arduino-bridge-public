// ==================================================
// BAZENOVA AUTOMATIKA
// HLAVNY PROGRAM
// ==================================================

// Jediny autoritativny runtime rezim na Mega. Typ je zamerne v hlavnej
// karte, aby ho videli automaticky generovane Arduino prototypy.
enum SystemMode : byte {
  MODE_SMART,
  MODE_DEGRADED,
  MODE_BASIC,
  MODE_STOP
};

// Boot a cela 180 s agreement stabilizacia patria do BASIC.
SystemMode systemMode = MODE_BASIC;

extern const byte MEGA_AGREEMENT_WATCHDOG_PIN;
extern bool megaAgreementOn;
extern bool testR9Aktivny;
void vypisMegaProblemSummary();
void vypisMegaSmartStable();

unsigned long casPoslednehoCyklu = 0;
unsigned long casPoslednejMegaDiagnostiky = 0;
bool megaDiagnostickyVypis = false;

// Lahka diagnostika trvania posledneho dokonceneho procesneho cyklu.
unsigned long megaCycleLastUs = 0, megaCycleMaxUs = 0;
unsigned long megaTeplotyLastUs = 0, megaTeplotyMaxUs = 0;
unsigned long megaDsLastUs = 0, megaDsMaxUs = 0;
unsigned long megaAhtLastUs = 0, megaAhtMaxUs = 0;
unsigned long megaSpracovanieLastUs = 0, megaSpracovanieMaxUs = 0;
unsigned long megaBezpecnostLastUs = 0, megaBezpecnostMaxUs = 0;
unsigned long megaRegulaciaLastUs = 0, megaRegulaciaMaxUs = 0;
unsigned long megaVystupyLastUs = 0, megaVystupyMaxUs = 0;
unsigned long megaRtcLastUs = 0, megaRtcMaxUs = 0;
unsigned long megaRtcAktualnyCyklusUs = 0;
unsigned long megaCycleOver1000msCount = 0;
unsigned long megaCycleOver1500msCount = 0;
bool megaProcesnyCyklusBezi = false;

void aktualizujMegaTimingMax(unsigned long &maximum, unsigned long hodnota) {
  if (hodnota > maximum) maximum = hodnota;
}

void zaznamenajMegaRtcCas(unsigned long zaciatokUs) {
  if (megaProcesnyCyklusBezi)
    megaRtcAktualnyCyklusUs += micros() - zaciatokUs;
}

void vypisMegaTiming() {
  Serial.println(F("MEGA TIMING (LAST COMPLETED):"));
  Serial.print(F("CYCLE=")); Serial.print(megaCycleLastUs);
  Serial.print(F("us MAX=")); Serial.println(megaCycleMaxUs);
  Serial.print(F("TEMP=")); Serial.print(megaTeplotyLastUs);
  Serial.print(F(" MAX=")); Serial.print(megaTeplotyMaxUs);
  Serial.print(F(" DS=")); Serial.print(megaDsLastUs);
  Serial.print(F(" MAX=")); Serial.println(megaDsMaxUs);
  Serial.print(F("AHT=")); Serial.print(megaAhtLastUs);
  Serial.print(F(" MAX=")); Serial.print(megaAhtMaxUs);
  Serial.print(F(" RTC=")); Serial.print(megaRtcLastUs);
  Serial.print(F(" MAX=")); Serial.println(megaRtcMaxUs);
  Serial.print(F("PROC=")); Serial.print(megaSpracovanieLastUs);
  Serial.print(F(" MAX=")); Serial.print(megaSpracovanieMaxUs);
  Serial.print(F(" SAFE=")); Serial.print(megaBezpecnostLastUs);
  Serial.print(F(" MAX=")); Serial.println(megaBezpecnostMaxUs);
  Serial.print(F("REG=")); Serial.print(megaRegulaciaLastUs);
  Serial.print(F(" MAX=")); Serial.print(megaRegulaciaMaxUs);
  Serial.print(F(" OUT=")); Serial.print(megaVystupyLastUs);
  Serial.print(F(" MAX=")); Serial.println(megaVystupyMaxUs);
  Serial.print(F("OVER_1000MS=")); Serial.print(megaCycleOver1000msCount);
  Serial.print(F(" OVER_1500MS=")); Serial.println(megaCycleOver1500msCount);
}

void setup() {

  // ------------------------------------------------
  // START SERIAL MONITOR
  // ------------------------------------------------

  Serial.begin(115200);

  // Watchdog/povolovacia vetva D31 je zakazana skor, nez sa spusti V5 linka.
  digitalWrite(MEGA_AGREEMENT_WATCHDOG_PIN, LOW);
  pinMode(MEGA_AGREEMENT_WATCHDOG_PIN, OUTPUT);
  digitalWrite(MEGA_AGREEMENT_WATCHDOG_PIN, LOW);
  inicializaciaWifiLink();
  inicializaciaUnoLinkTest();

  inicializaciaRTC();
  skenujI2C();
  inicializaciaAHT10();

  inicializaciaTlacidiel();

  casPoslednehoCyklu = millis() - 2000UL;
  casPoslednejMegaDiagnostiky = millis() - 10000UL;
  // ------------------------------------------------
  // INICIALIZACIA VSTUPOV
  // ------------------------------------------------

  inicializaciaVstupov();


  // ------------------------------------------------
  // INICIALIZACIA VYSTUPOV
  // ------------------------------------------------

  inicializaciaVystupov();

  inicializaciaHMI();


  // ------------------------------------------------
  // UVODNA SPRAVA
  // ------------------------------------------------

  Serial.println("================================");
  Serial.println("      BAZENOVA AUTOMATIKA");
  Serial.println("================================");
  Serial.println("SYSTEM START");
  Serial.println("--------------------------------");
}


void loop() {

  aktualizujTlacidla();
  aktualizujRotaciuHMI();
  aktualizujWifiLink();
  aktualizujMegaXkc();
  aktualizujMegaSonar();
  aktualizujUnoLink();
  if (testR9Aktivny) {
    aktualizujTestReleR9();
  }
  // Dokoncenie 2 s OFF intervalu a okamzite zrusenie po strate poziadavky
  // bezia nezavisle od 2 s procesneho schedulera. Novy 6 h reset sa tu
  // nespusta; ten sa povoli az po cerstvom vyhodnoteni regulacie filtracie.
  aktualizujNapajanieFiltracie(false);

  unsigned long teraz = millis();

  if (teraz - casPoslednehoCyklu < 2000) {
    return;
  }

  casPoslednehoCyklu = teraz;
  const unsigned long megaCycleStartUs = micros();
  megaProcesnyCyklusBezi = true;
  megaRtcAktualnyCyklusUs = 0;
  megaDiagnostickyVypis = teraz - casPoslednejMegaDiagnostiky >= 10000UL;
  if (megaDiagnostickyVypis) {
    casPoslednejMegaDiagnostiky = teraz;
    Serial.println(F("--- DIAG 10 s ---"));
  }

  // =================================================
  // 1. VSTUPY
  // =================================================

  unsigned long castStartUs = micros();
  nacitajTeploty();
  megaTeplotyLastUs = micros() - castStartUs;
  aktualizujMegaTimingMax(megaTeplotyMaxUs, megaTeplotyLastUs);


  // =================================================
  // 2. SPRACOVANIE
  // =================================================

  castStartUs = micros();
  spracovanie();
  megaSpracovanieLastUs = micros() - castStartUs;
  aktualizujMegaTimingMax(megaSpracovanieMaxUs, megaSpracovanieLastUs);


  // =================================================
  // 3. BEZPECNOST
  // =================================================

  castStartUs = micros();
  bezpecnost();
  // Jedine autoritativne vyhodnotenie rezimu nasleduje po aktualnej safety
  // a fallback diagnostike. Regulacia ani vystupy sa tymto krokom nemenia.
  aktualizujSystemMode();
  megaBezpecnostLastUs = micros() - castStartUs;
  aktualizujMegaTimingMax(megaBezpecnostMaxUs, megaBezpecnostLastUs);

  // Súhrn patrí až za aktuálne vstupy, spracovanie a bezpečnosť. Príznak
  // megaDiagnostickyVypis zostáva nastavený už od začiatku cyklu, aby ho
  // mohli používať aj jednotlivé diagnostické výpisy v týchto funkciách.
  if (megaDiagnostickyVypis) {
    vypisMegaProblemSummary();
    Serial.print(F("MEGA_AGREEMENT="));
    Serial.println(megaAgreementOn ? F("ON") : F("OFF"));
    vypisMegaSmartStable();
    vypisSystemMode();
  }


  // =================================================
  // 4. REGULACIA
  // =================================================

  castStartUs = micros();
  regulacia();
  regulaciaFiltracie();
  megaRegulaciaLastUs = micros() - castStartUs;
  aktualizujMegaTimingMax(megaRegulaciaMaxUs, megaRegulaciaLastUs);


  // =================================================
  // 5. VYSTUPY
  // =================================================

  castStartUs = micros();
  vystupy();
  megaVystupyLastUs = micros() - castStartUs;
  aktualizujMegaTimingMax(megaVystupyMaxUs, megaVystupyLastUs);

  if (megaDiagnostickyVypis) vypisRTC();


  // =================================================
  // 6. HMI
  // =================================================

  HMI();


  // =================================================
  // CYKLUS
  // =================================================

  megaRtcLastUs = megaRtcAktualnyCyklusUs;
  aktualizujMegaTimingMax(megaRtcMaxUs, megaRtcLastUs);
  megaCycleLastUs = micros() - megaCycleStartUs;
  aktualizujMegaTimingMax(megaCycleMaxUs, megaCycleLastUs);
  if (megaCycleLastUs >= 1000000UL) megaCycleOver1000msCount++;
  if (megaCycleLastUs >= 1500000UL) megaCycleOver1500msCount++;
  megaProcesnyCyklusBezi = false;

  // LAST hodnoty su teraz uzavrete z rovnakeho procesneho cyklu.
  if (megaDiagnostickyVypis) {
    vypisMegaTiming();
    Serial.println("--------------------------------");
  }

}
