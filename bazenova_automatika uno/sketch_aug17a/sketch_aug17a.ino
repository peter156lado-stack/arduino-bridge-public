#include <OneWire.h>
#include <DallasTemperature.h>
#include <SD.h>
#include <SoftwareSerial.h>

// ==================================================
// MONITOR-ONLY ZAKLAD UNO
// ==================================================

const byte ONE_WIRE_PIN = 2;
const byte SONAR_TRIG_PIN = 3;
const byte SONAR_ECHO_PIN = 4;
const byte MEGA_LINK_RX_PIN = 7;
const byte MEGA_LINK_TX_PIN = 8;
// Prva fyzicka implementacia SMART agreement. HIGH = COM-NO, LOW = COM-NC.
const byte UNO_HL_RELAY_1_PIN = 9;
// UNO_TOTAL_STOP: buduca vedoma safety autorita. Boot/default = LOW / COM-NC.
const byte UNO_TOTAL_STOP_PIN = A0;
// XKC commissioning vstup cez samostatny PC817/HY-M154 kanal.
const byte UNO_XKC_PIN = A2;

const unsigned long TEMPERATURE_INTERVAL_MS = 5300UL;
// Po vypadku napajania sa DS18B20 vrati na 12 bit (max. 750 ms).
// Tento cas umozni obnovu este pred opatovnym nastavenim na 10 bit.
const unsigned long TEMPERATURE_CONVERSION_MS = 800UL;
const unsigned long ONE_WIRE_REINIT_INTERVAL_MS = 5000UL;
const unsigned long SONAR_INTERVAL_US = 250000UL;
const unsigned long SONAR_TRIGGER_LOW_US = 3UL;
const unsigned long SONAR_TRIGGER_HIGH_US = 10UL;
const unsigned long SONAR_TIMEOUT_US = 30000UL;
const float SONAR_MIN_CM = 2.0f;
const float SONAR_MAX_CM = 450.0f;
const byte SONAR_POTVRDENIE_POCET = 3;
const unsigned long DIAGNOSTIC_INTERVAL_MS = 10000UL;

// Volitelne diagnosticke logovanie na MicroSD.
const byte SD_CS_PIN = 10;
const unsigned long SD_LOG_INTERVAL_MS = 60000UL;
const unsigned long SD_RECOVERY_INTERVAL_MS = 60000UL;
// Diagnosticka/telemetricka linka Mega <-> Uno; nejde o safety heartbeat.
const unsigned long MEGA_LINK_BAUD = 38400UL;
const unsigned long MEGA_LINK_FRAME_INTERVAL_MS = 1000UL;
const unsigned long MEGA_LINK_TIMEOUT_MS = 10000UL;
const unsigned long MEGA_REMOTE_TIMEOUT_MS = 10000UL;
const unsigned long UNO_SMART_STABILIZACIA_MS = 180000UL;
const unsigned long CAS_SYNC_INTERVAL_MS = 60UL * 60UL * 1000UL;
const byte LINK_MAGIC_1 = 0xBA;
const byte LINK_MAGIC_2 = 0x5E;
const byte LINK_PROTOCOL_VERSION = 5;
const byte LINK_TYPE_UNO_TO_MEGA = 0x01;
const byte LINK_TYPE_MEGA_TO_UNO = 0x02;
const byte UNO_FRAME_SIZE = 22;
const byte MEGA_FRAME_SIZE = 24;

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);
// Priame TTL UART prepojenie Mega <-> Uno bez optoclenov; neinvertovana logika.
SoftwareSerial megaLinkSerial(MEGA_LINK_RX_PIN, MEGA_LINK_TX_PIN, false);

bool unoXkcLowWater = false;
bool unoXkcInicializovany = false;

void aktualizujUnoXkc() {
  const bool novyLowWater = digitalRead(UNO_XKC_PIN) == HIGH;

  if (!unoXkcInicializovany) {
    unoXkcLowWater = novyLowWater;
    unoXkcInicializovany = true;
    return;
  }

  if (novyLowWater == unoXkcLowWater) return;
  unoXkcLowWater = novyLowWater;
  Serial.println(unoXkcLowWater ? F("EVENT: UNO_XKC=LOW_WATER")
                               : F("RECOVERY: UNO_XKC=WATER"));
}

// Fixne ROM adresy fyzicky otestovanych snimacov.
const DeviceAddress T1_ADDRESS = {
  0x28, 0x70, 0xA6, 0x08, 0x91, 0x25, 0x06, 0x55
};
const DeviceAddress T2_ADDRESS = {
  0x28, 0xF8, 0x23, 0x5B, 0x00, 0x00, 0x00, 0x37
};
const DeviceAddress T3_ADDRESS = {
  0x28, 0x29, 0x81, 0x5E, 0x00, 0x00, 0x00, 0xF5
};
const DeviceAddress TBOX_ADDRESS = {
  0x28, 0x16, 0x2E, 0x09, 0x00, 0x03, 0x24, 0x29
};

float teplotaT1 = DEVICE_DISCONNECTED_C;
float teplotaT2 = DEVICE_DISCONNECTED_C;
float teplotaT3 = DEVICE_DISCONNECTED_C;
float teplotaTBOX = DEVICE_DISCONNECTED_C;
bool T1_OK = false;
bool T1_CHYBA = true;
bool T2_OK = false;
bool T2_CHYBA = true;
bool T3_OK = false;
bool T3_CHYBA = true;
bool TBOX_OK = false;
bool TBOX_CHYBA = true;

bool konverziaTeplotyBezi = false;
unsigned long casStartuKonverzie = 0;
unsigned long casPoslednehoStartuTeploty = 0;
unsigned long casPoslednejReinicializacieOneWire = 0;

enum SonarStavMerania : byte {
  SONAR_CAKA,
  SONAR_TRIGGER_LOW,
  SONAR_TRIGGER_HIGH,
  SONAR_CAKA_NA_ECHO_HIGH,
  SONAR_CAKA_NA_ECHO_LOW
};

enum SonarVysledok : byte {
  SONAR_OK,
  SONAR_TIMEOUT,
  SONAR_MIMO_ROZSAH
};

SonarStavMerania sonarStavMerania = SONAR_CAKA;
SonarVysledok sonarStav = SONAR_TIMEOUT;
float sonarVzdialenostCm = 0.0f;
unsigned long sonarCasStavuUs = 0;
unsigned long sonarEchoStartUs = 0;
unsigned long sonarPoslednyStartUs = 0;
byte sonarPoSebeOk = 0;
byte sonarPoSebeChyba = 0;

void prijmiSonarVysledok(SonarVysledok novyStav, float novaVzdialenostCm = 0.0f) {
  if (novyStav == SONAR_OK) {
    sonarPoSebeChyba = 0;
    if (sonarPoSebeOk < SONAR_POTVRDENIE_POCET) sonarPoSebeOk++;
    if (sonarPoSebeOk >= SONAR_POTVRDENIE_POCET) {
      sonarVzdialenostCm = novaVzdialenostCm;
      sonarStav = SONAR_OK;
    }
    return;
  }

  sonarPoSebeOk = 0;
  if (sonarPoSebeChyba < SONAR_POTVRDENIE_POCET) sonarPoSebeChyba++;
  if (sonarPoSebeChyba >= SONAR_POTVRDENIE_POCET) sonarStav = novyStav;
}

bool sdLoggerDostupny = false;
unsigned long casPoslednehoSDLogu = 0;
unsigned long casPoslednehoSDPokusu = 0;

struct MegaVysledok {
  float pool;
  float t2;
  bool poolOk;
  bool t2Ok;
  bool rtcOk;
  byte poolZdroj;
  byte t2Zdroj;
  byte diagnostika;
  byte megaStav;
  unsigned int sekvencia;
  unsigned long rtcSekundyDna;
  byte rtcRok;
  byte rtcMesiac;
  byte rtcDen;
};

MegaVysledok megaVysledok = {
  0.0f, 0.0f, false, false, false,
  0, 0, 0, 2, 0, 0UL, 0, 0, 0
};

byte megaLinkRxBuffer[MEGA_FRAME_SIZE];
byte megaLinkTxBuffer[UNO_FRAME_SIZE];
byte megaLinkRxDlzka = 0;
byte megaLinkTxPozicia = UNO_FRAME_SIZE;
unsigned int unoLinkSekvencia = 0;
unsigned long megaLinkPosledneRxMs = 0;
unsigned long megaRemotePoslednyRamecMs = 0;
bool megaLinkOk = false;
bool megaLinkMaRamec = false;
bool unoAgreementOn = false;
bool unoSmartStabilizaciaBezi = false;
unsigned long unoSmartStabilnyOdMs = 0;
bool MEGA_RESULT_VALID = false;
unsigned long megaCrcFail = 0;
unsigned long megaFrameInvalid = 0;
unsigned long megaLinkTimeoutCount = 0;
unsigned long megaSeqGapCount = 0;
unsigned int megaPoslednaSekvencia = 0;
bool megaSekvenciaPlatna = false;

bool softCasPlatny = false;
bool softDatumPlatny = false;
unsigned long softCasZakladSekundy = 0;
unsigned long softCasZakladMs = 0;
byte softDatumZakladRok = 0;
byte softDatumZakladMesiac = 0;
byte softDatumZakladDen = 0;
unsigned long casPoslednejSynchronizacie = 0;
unsigned int poslednyEventPodpis = 0xFFFFU;
unsigned long poslednyDiagnostickyPodpis = 0xFFFFFFFFUL;
bool diagnostickyPodpisPlatny = false;

// ==================================================
// KOSTRA BUDUCICH STAVOV - ZIADNE PINY ANI OVLADANIE
// ==================================================

struct BuduceBezpecnostneStavy {
  bool resetAckStlaceny;
  bool heartbeatMegaPrijaty;
  bool heartbeatUnoAktivny;
  bool smartBasicAgreement;
  bool filBlock;
  bool solarBlock;
};

BuduceBezpecnostneStavy buduceStavy = {
  false, false, false, false, false, false
};

bool teplotaJePlatna(float hodnota) {
  return hodnota != DEVICE_DISCONNECTED_C &&
         hodnota != 85.0f &&
         hodnota >= -55.0f &&
         hodnota <= 125.0f;
}

byte linkCrc8(const byte *data, byte dlzka) {
  byte crc = 0;
  for (byte i = 0; i < dlzka; i++) {
    crc ^= data[i];
    for (byte bit = 0; bit < 8; bit++)
      crc = (crc & 0x80) ? (byte)((crc << 1) ^ 0x07) : (byte)(crc << 1);
  }
  return crc;
}

void zapisU16(byte *data, byte pozicia, unsigned int hodnota) {
  data[pozicia] = (byte)hodnota;
  data[pozicia + 1] = (byte)(hodnota >> 8);
}

unsigned int citajU16(const byte *data, byte pozicia) {
  return (unsigned int)data[pozicia] | ((unsigned int)data[pozicia + 1] << 8);
}

void zapisI16(byte *data, byte pozicia, int hodnota) { zapisU16(data, pozicia, (unsigned int)hodnota); }
int citajI16(const byte *data, byte pozicia) { return (int)citajU16(data, pozicia); }

unsigned long citajU32(const byte *data, byte pozicia) {
  return (unsigned long)data[pozicia] |
         ((unsigned long)data[pozicia + 1] << 8) |
         ((unsigned long)data[pozicia + 2] << 16) |
         ((unsigned long)data[pozicia + 3] << 24);
}

int teplotaNaStotiny(float hodnota) {
  return (int)(hodnota * 100.0f + (hodnota >= 0.0f ? 0.5f : -0.5f));
}

float stotinyNaTeplotu(int hodnota) { return (float)hodnota / 100.0f; }

bool jePrestupnyRok(byte rok) {
  const unsigned int celyRok = 2000U + rok;
  return (celyRok % 4U == 0U && celyRok % 100U != 0U) || celyRok % 400U == 0U;
}

byte pocetDniVMesiaci(byte rok, byte mesiac) {
  static const byte dni[] PROGMEM = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  if (mesiac < 1 || mesiac > 12) return 0;
  if (mesiac == 2 && jePrestupnyRok(rok)) return 29;
  return pgm_read_byte(&dni[mesiac - 1]);
}

bool datumJePlatny(byte rok, byte mesiac, byte den) {
  const byte maximum = pocetDniVMesiaci(rok, mesiac);
  return maximum != 0 && den >= 1 && den <= maximum;
}

void pridajDen(byte &rok, byte &mesiac, byte &den) {
  den++;
  if (den <= pocetDniVMesiaci(rok, mesiac)) return;
  den = 1;
  mesiac++;
  if (mesiac <= 12) return;
  mesiac = 1;
  rok++;
}

void prijmiMegaRamec() {
  if (megaLinkRxBuffer[0] != LINK_MAGIC_1 || megaLinkRxBuffer[1] != LINK_MAGIC_2 ||
      megaLinkRxBuffer[2] != LINK_PROTOCOL_VERSION || megaLinkRxBuffer[3] != LINK_TYPE_MEGA_TO_UNO ||
      megaLinkRxBuffer[4] != MEGA_FRAME_SIZE) {
    megaFrameInvalid++;
    return;
  }
  if (linkCrc8(megaLinkRxBuffer, MEGA_FRAME_SIZE - 1) != megaLinkRxBuffer[MEGA_FRAME_SIZE - 1]) {
    megaCrcFail++;


    return;
  }
  MegaVysledok novy;
  const byte validity = megaLinkRxBuffer[7];
  novy.sekvencia = citajU16(megaLinkRxBuffer, 5);
  novy.poolOk = validity & 0x01;
  novy.t2Ok = validity & 0x02;
  novy.rtcOk = validity & 0x04;
  novy.poolZdroj = megaLinkRxBuffer[8];
  novy.t2Zdroj = megaLinkRxBuffer[9];
  novy.diagnostika = megaLinkRxBuffer[10];
  novy.megaStav = megaLinkRxBuffer[11];
  if (novy.poolZdroj > 3 || novy.t2Zdroj > 3 ||
      (novy.diagnostika & 0x80) != 0 || novy.megaStav > 2) {
    megaFrameInvalid++;
    return;
  }
  novy.pool = stotinyNaTeplotu(citajI16(megaLinkRxBuffer, 12));
  novy.t2 = stotinyNaTeplotu(citajI16(megaLinkRxBuffer, 14));
  novy.rtcSekundyDna = citajU32(megaLinkRxBuffer, 16);
  novy.rtcRok = megaLinkRxBuffer[20];
  novy.rtcMesiac = megaLinkRxBuffer[21];
  novy.rtcDen = megaLinkRxBuffer[22];
  if (novy.rtcSekundyDna >= 86400UL ||
      (novy.rtcOk && !datumJePlatny(novy.rtcRok, novy.rtcMesiac, novy.rtcDen))) {
    megaFrameInvalid++;
    return;
  }
  if (megaSekvenciaPlatna && novy.sekvencia != (unsigned int)(megaPoslednaSekvencia + 1U)) megaSeqGapCount++;
  megaPoslednaSekvencia = novy.sekvencia;
  megaSekvenciaPlatna = true;
  megaVysledok = novy;
  megaRemotePoslednyRamecMs = millis();
  megaLinkPosledneRxMs = megaRemotePoslednyRamecMs;
  megaLinkMaRamec = true;
  // UNO odpoveda iba na uplne prijaty a validny Mega ramec. Predosla
  // odpoved je pri master intervale 1 s uz dokoncena; buffer neprepisuj.
  if (megaLinkTxPozicia >= UNO_FRAME_SIZE) pripravUnoTelemetriu();
}

void prijmiMegaByte(byte hodnota) {
  if (megaLinkRxDlzka == 0 && hodnota != LINK_MAGIC_1) return;
  if (megaLinkRxDlzka == 1 && hodnota != LINK_MAGIC_2) {
    megaLinkRxDlzka = hodnota == LINK_MAGIC_1 ? 1 : 0;
    return;
  }
  megaLinkRxBuffer[megaLinkRxDlzka++] = hodnota;
  if (megaLinkRxDlzka == MEGA_FRAME_SIZE) {
    prijmiMegaRamec();
    megaLinkRxDlzka = 0;
  }
}

byte aktualnyUnoStav() {
  // 0 = UNO_OK, 1 = UNO_SENSOR_DEGRADED, 2 = UNO_CHYBA.
  // Chyba meracieho senzora ani strata vysledku Mega neznamena poruchu Una.
  if (!T1_OK || !T2_OK || !T3_OK || !TBOX_OK || sonarStav != SONAR_OK) return 1;
  return 0;  // UNO_CHYBA je rezervovane pre buducu kriticku supervisor/safety poruchu.
}

void pripravUnoTelemetriu() {
  unoLinkSekvencia++;
  megaLinkTxBuffer[0] = LINK_MAGIC_1;
  megaLinkTxBuffer[1] = LINK_MAGIC_2;
  megaLinkTxBuffer[2] = LINK_PROTOCOL_VERSION;
  megaLinkTxBuffer[3] = LINK_TYPE_UNO_TO_MEGA;
  megaLinkTxBuffer[4] = UNO_FRAME_SIZE;
  zapisU16(megaLinkTxBuffer, 5, unoLinkSekvencia);
  byte validity = 0;
  if (T1_OK) validity |= 0x01;
  if (T2_OK) validity |= 0x02;
  if (TBOX_OK) validity |= 0x04;
  if (sonarStav == SONAR_OK) validity |= 0x08;
  if (T3_OK) validity |= 0x10;
  megaLinkTxBuffer[7] = validity;
  megaLinkTxBuffer[8] = aktualnyUnoStav();
  megaLinkTxBuffer[9] = (byte)sonarStav;
  zapisI16(megaLinkTxBuffer, 10, T1_OK ? teplotaNaStotiny(teplotaT1) : 0);
  zapisI16(megaLinkTxBuffer, 12, T2_OK ? teplotaNaStotiny(teplotaT2) : 0);
  zapisI16(megaLinkTxBuffer, 14, TBOX_OK ? teplotaNaStotiny(teplotaTBOX) : 0);
  unsigned int sonarDesatiny = 0;
  if (sonarStav == SONAR_OK) sonarDesatiny = (unsigned int)(sonarVzdialenostCm * 10.0f + 0.5f);
  zapisU16(megaLinkTxBuffer, 16, sonarDesatiny);
  zapisI16(megaLinkTxBuffer, 18, T3_OK ? teplotaNaStotiny(teplotaT3) : 0);
  // V5 flagy: bit 0 = UNO agreement, bit 1 = lokalny XKC LOW WATER.
  megaLinkTxBuffer[20] = (unoAgreementOn ? 0x01 : 0x00) |
                         (unoXkcLowWater ? 0x02 : 0x00);
  megaLinkTxBuffer[21] = linkCrc8(megaLinkTxBuffer, UNO_FRAME_SIZE - 1);
  megaLinkTxPozicia = 0;
}

unsigned long aktualnySoftCasSekundy() {
  if (!softCasPlatny) return 0UL;
  return (softCasZakladSekundy + (millis() - softCasZakladMs) / 1000UL) % 86400UL;
}

bool aktualnySoftDatum(byte &rok, byte &mesiac, byte &den) {
  if (!softDatumPlatny || !softCasPlatny) return false;
  rok = softDatumZakladRok;
  mesiac = softDatumZakladMesiac;
  den = softDatumZakladDen;
  unsigned long dniOdZakladu =
    (softCasZakladSekundy + (millis() - softCasZakladMs) / 1000UL) / 86400UL;
  while (dniOdZakladu-- > 0) pridajDen(rok, mesiac, den);
  return true;
}

void synchronizujSoftCasAkTreba() {
  if (!MEGA_RESULT_VALID || !megaVysledok.rtcOk) return;
  const unsigned long teraz = millis();
  byte aktualnyRok, aktualnyMesiac, aktualnyDen;
  const bool datumUzSedi = aktualnySoftDatum(aktualnyRok, aktualnyMesiac, aktualnyDen) &&
                           aktualnyRok == megaVysledok.rtcRok &&
                           aktualnyMesiac == megaVysledok.rtcMesiac &&
                           aktualnyDen == megaVysledok.rtcDen;
  if (softCasPlatny && datumUzSedi && teraz - casPoslednejSynchronizacie < CAS_SYNC_INTERVAL_MS) return;
  softCasZakladSekundy = megaVysledok.rtcSekundyDna;
  softCasZakladMs = teraz;
  softDatumZakladRok = megaVysledok.rtcRok;
  softDatumZakladMesiac = megaVysledok.rtcMesiac;
  softDatumZakladDen = megaVysledok.rtcDen;
  casPoslednejSynchronizacie = teraz;
  softCasPlatny = true;
  softDatumPlatny = true;
}

void aktualizujMegaVysledok() {
  const unsigned long teraz = millis();
  MEGA_RESULT_VALID = megaRemotePoslednyRamecMs != 0 &&
                      teraz - megaRemotePoslednyRamecMs < MEGA_REMOTE_TIMEOUT_MS;
  synchronizujSoftCasAkTreba();
}

bool megaRemoteXkcLowWater() {
  return (megaVysledok.diagnostika & 0x40) != 0;
}

bool unoXkcConflict() {
  return MEGA_RESULT_VALID && unoXkcLowWater != megaRemoteXkcLowWater();
}

void inicializujMegaLinkTest() {
  megaLinkSerial.begin(MEGA_LINK_BAUD);
}

void nastavUnoAgreement(bool povolit) {
  if (unoAgreementOn == povolit) return;
  unoAgreementOn = povolit;
  digitalWrite(UNO_HL_RELAY_1_PIN, unoAgreementOn ? HIGH : LOW);
  Serial.println(unoAgreementOn ? F("RECOVERY: UNO_AGREEMENT=ON")
                                : F("EVENT: UNO_AGREEMENT=OFF"));
}

void aktualizujUnoAgreementStabilizaciu(bool podmienkyOk, unsigned long teraz) {
  if (!podmienkyOk) {
    if (unoSmartStabilizaciaBezi) {
      unoSmartStabilizaciaBezi = false;
      Serial.println(F("EVENT: UNO_SMART_STABLE=BLOCKED"));
    }
    nastavUnoAgreement(false);
    return;
  }

  if (!unoSmartStabilizaciaBezi) {
    unoSmartStabilizaciaBezi = true;
    unoSmartStabilnyOdMs = teraz;
    Serial.println(F("RECOVERY: UNO_SMART_STABLE=START"));
  }

  if (teraz - unoSmartStabilnyOdMs >= UNO_SMART_STABILIZACIA_MS)
    nastavUnoAgreement(true);
}

void aktualizujMegaLinkTest() {
  const unsigned long teraz = millis();
  byte prijate = 0;
  while (megaLinkSerial.available() > 0 && prijate < 24) {
    prijmiMegaByte((byte)megaLinkSerial.read());
    prijate++;
  }

  if (megaLinkTxPozicia < UNO_FRAME_SIZE) {
    megaLinkSerial.write(megaLinkTxBuffer[megaLinkTxPozicia++]);
  }

  // Parser mohol prave nastavit posledny RX cez novsie millis(). Timeout preto
  // vyhodnot az z casu odobrateho po spracovani prijmu.
  const unsigned long linkTerazMs = millis();
  const bool novyLinkOk = megaLinkMaRamec && linkTerazMs - megaLinkPosledneRxMs < MEGA_LINK_TIMEOUT_MS;
  if (megaLinkOk && !novyLinkOk) megaLinkTimeoutCount++;
  if (novyLinkOk != megaLinkOk) {
    megaLinkOk = novyLinkOk;
    Serial.println(megaLinkOk ? F("RECOVERY: LINK_OK") : F("EVENT: LINK_CHYBA"));
  }
  const bool megaDataCerstve = megaRemotePoslednyRamecMs != 0 &&
                               linkTerazMs - megaRemotePoslednyRamecMs < MEGA_REMOTE_TIMEOUT_MS;
  // megaStav 1 je doveryhodny DEGRADED stav; agreement blokuje iba kriticka 2.
  // SYSTEM_MODE zostava vylucne autoritou Mega.
  const bool smartPodmienkyOk = novyLinkOk && megaDataCerstve && megaVysledok.megaStav != 2;
  aktualizujUnoAgreementStabilizaciu(smartPodmienkyOk, linkTerazMs);
  aktualizujMegaVysledok();
}

void reinicializujOneWireAkTreba(unsigned long teraz) {
  if (!T1_CHYBA && !T2_CHYBA && !T3_CHYBA && !TBOX_CHYBA) {
    return;
  }

  if (teraz - casPoslednejReinicializacieOneWire <
      ONE_WIRE_REINIT_INTERVAL_MS) {
    return;
  }

  // begin() znovu prehlada zbernicu. Fixne ROM adresy zostavaju nezmenene.
  // Operacia je kratka a vykona sa len raz za 5 sekund pri chybe.
  sensors.begin();
  sensors.setResolution(T1_ADDRESS, 10);
  sensors.setResolution(T2_ADDRESS, 10);
  sensors.setResolution(T3_ADDRESS, 10);
  sensors.setResolution(TBOX_ADDRESS, 10);
  sensors.setWaitForConversion(false);
  casPoslednejReinicializacieOneWire = teraz;
}

void aktualizujTeploty() {
  const unsigned long teraz = millis();

  reinicializujOneWireAkTreba(teraz);

  if (!konverziaTeplotyBezi &&
      teraz - casPoslednehoStartuTeploty >= TEMPERATURE_INTERVAL_MS) {
    sensors.requestTemperatures();
    casPoslednehoStartuTeploty = teraz;
    casStartuKonverzie = teraz;
    konverziaTeplotyBezi = true;
  }

  if (!konverziaTeplotyBezi ||
      teraz - casStartuKonverzie < TEMPERATURE_CONVERSION_MS) {
    return;
  }

  // Kazdy snimac sa cita a vyhodnocuje samostatne.
  teplotaT1 = sensors.getTempC(T1_ADDRESS);
  T1_OK = teplotaJePlatna(teplotaT1);
  T1_CHYBA = !T1_OK;

  teplotaT2 = sensors.getTempC(T2_ADDRESS);
  T2_OK = teplotaJePlatna(teplotaT2);
  T2_CHYBA = !T2_OK;

  teplotaT3 = sensors.getTempC(T3_ADDRESS);
  T3_OK = teplotaJePlatna(teplotaT3);
  T3_CHYBA = !T3_OK;

  teplotaTBOX = sensors.getTempC(TBOX_ADDRESS);
  TBOX_OK = teplotaJePlatna(teplotaTBOX);
  TBOX_CHYBA = !TBOX_OK;

  konverziaTeplotyBezi = false;
}

void dokonciSonarTimeout() {
  prijmiSonarVysledok(SONAR_TIMEOUT);
  sonarStavMerania = SONAR_CAKA;
  sonarPoslednyStartUs = micros();
}

void aktualizujSonar() {
  const unsigned long terazUs = micros();

  switch (sonarStavMerania) {
    case SONAR_CAKA:
      if (terazUs - sonarPoslednyStartUs >= SONAR_INTERVAL_US) {
        digitalWrite(SONAR_TRIG_PIN, LOW);
        sonarCasStavuUs = terazUs;
        sonarStavMerania = SONAR_TRIGGER_LOW;
      }
      break;

    case SONAR_TRIGGER_LOW:
      if (terazUs - sonarCasStavuUs >= SONAR_TRIGGER_LOW_US) {
        digitalWrite(SONAR_TRIG_PIN, HIGH);
        sonarCasStavuUs = terazUs;
        sonarStavMerania = SONAR_TRIGGER_HIGH;
      }
      break;

    case SONAR_TRIGGER_HIGH:
      if (terazUs - sonarCasStavuUs >= SONAR_TRIGGER_HIGH_US) {
        digitalWrite(SONAR_TRIG_PIN, LOW);
        sonarCasStavuUs = terazUs;
        sonarStavMerania = SONAR_CAKA_NA_ECHO_HIGH;
      }
      break;

    case SONAR_CAKA_NA_ECHO_HIGH:
      if (digitalRead(SONAR_ECHO_PIN) == HIGH) {
        sonarEchoStartUs = terazUs;
        sonarStavMerania = SONAR_CAKA_NA_ECHO_LOW;
      }
      else if (terazUs - sonarCasStavuUs >= SONAR_TIMEOUT_US) {
        dokonciSonarTimeout();
      }
      break;

    case SONAR_CAKA_NA_ECHO_LOW:
      if (digitalRead(SONAR_ECHO_PIN) == LOW) {
        const unsigned long echoDlzkaUs = terazUs - sonarEchoStartUs;
        const float novaVzdialenostCm = echoDlzkaUs * 0.0343f / 2.0f;

        if (novaVzdialenostCm >= SONAR_MIN_CM &&
            novaVzdialenostCm <= SONAR_MAX_CM) {
          prijmiSonarVysledok(SONAR_OK, novaVzdialenostCm);
        }
        else {
          prijmiSonarVysledok(SONAR_MIMO_ROZSAH);
        }

        sonarStavMerania = SONAR_CAKA;
        sonarPoslednyStartUs = terazUs;
      }
      else if (terazUs - sonarEchoStartUs >= SONAR_TIMEOUT_US) {
        dokonciSonarTimeout();
      }
      break;
  }
}

const __FlashStringHelper *textSonarStavu() {
  switch (sonarStav) {
    case SONAR_OK:
      return F("SONAR_OK");
    case SONAR_MIMO_ROZSAH:
      return F("SONAR_MIMO_ROZSAH");
    default:
      return F("SONAR_TIMEOUT");
  }
}

void vypisHodnotuCSV(File &subor, float hodnota, bool platna, byte desatinneMiesta) {
  if (platna) {
    subor.print(hodnota, desatinneMiesta);
  }
  else {
    subor.print(F("NA"));
  }
}

void vypniSDLogger(const __FlashStringHelper *chyba) {
  sdLoggerDostupny = false;
  casPoslednehoSDPokusu = millis();
  Serial.print(F("SD: "));
  Serial.println(chyba);
}

void vytvorDennyNazov(char *nazov, bool eventSubor) {
  byte rok, mesiac, den;
  if (!aktualnySoftDatum(rok, mesiac, den)) {
    strcpy_P(nazov, eventSubor ? PSTR("UNDTEVT.CSV") : PSTR("UNDTLOG.CSV"));
    return;
  }
  nazov[0] = eventSubor ? 'E' : 'L';
  nazov[1] = '0' + (rok / 10U) % 10U;
  nazov[2] = '0' + rok % 10U;
  nazov[3] = '0' + mesiac / 10U;
  nazov[4] = '0' + mesiac % 10U;
  nazov[5] = '0' + den / 10U;
  nazov[6] = '0' + den % 10U;
  nazov[7] = '.';
  nazov[8] = 'C';
  nazov[9] = 'S';
  nazov[10] = 'V';
  nazov[11] = '\0';
}

void vypisDvojciferneCSV(File &subor, byte hodnota) {
  if (hodnota < 10) subor.print('0');
  subor.print(hodnota);
}

void vypisCasCSV(File &subor) {
  if (!softCasPlatny) {
    subor.print(F("NA"));
    return;
  }
  const unsigned long sekundy = aktualnySoftCasSekundy();
  vypisDvojciferneCSV(subor, sekundy / 3600UL);
  subor.print(':');
  vypisDvojciferneCSV(subor, (sekundy / 60UL) % 60UL);
  subor.print(':');
  vypisDvojciferneCSV(subor, sekundy % 60UL);
}

void zapisHlavickuPrevadzkovehoLogu(File &subor) {
  subor.println(F("time,millis,UNO_T1_C,T1_OK,UNO_T2_C,T2_OK,UNO_T3_C,T3_OK,UNO_TBOX_C,TBOX_OK,SONAR_CM,SONAR_STATE,UNO_STATE,MEGA_LINK,MEGA_RESULT,MEGA_STATE,POOL_C,POOL_SOURCE,T2_EFFECTIVE_C,T2_SOURCE,DIAGNOSTIC_FLAGS"));
}

void zapisHlavickuEventLogu(File &subor) {
  subor.println(F("time,millis,event_signature,UNO_STATE,MEGA_LINK,MEGA_RESULT,MEGA_STATE,POOL_SOURCE,T2_SOURCE,DIAGNOSTIC_FLAGS"));
}

void inicializujSDLogger() {
  // Jeden kratky pokus; pri chybe sa dalsi vykona az po intervale recovery.
  casPoslednehoSDPokusu = millis();

  if (!SD.begin(SD_CS_PIN)) {
    vypniSDLogger(F("CHYBA INICIALIZACIE - LOGGER DEAKTIVOVANY"));
    return;
  }

  char nazov[13];
  vytvorDennyNazov(nazov, false);
  File subor = SD.open(nazov, FILE_WRITE);
  if (!subor) {
    vypniSDLogger(F("CHYBA OTVORENIA - LOGGER DEAKTIVOVANY"));
    return;
  }

  if (subor.size() == 0) {
    zapisHlavickuPrevadzkovehoLogu(subor);

    if (subor.getWriteError()) {
      subor.close();
      vypniSDLogger(F("CHYBA ZAPISU HLAVICKY - LOGGER DEAKTIVOVANY"));
      return;
    }
  }

  subor.close();
  sdLoggerDostupny = true;
  casPoslednehoSDLogu = millis();
  Serial.print(F("SD: OK - "));
  Serial.println(nazov);
}

void aktualizujSDRecovery() {
  if (sdLoggerDostupny ||
      millis() - casPoslednehoSDPokusu < SD_RECOVERY_INTERVAL_MS) {
    return;
  }

  Serial.println(F("SD: POKUS O OBNOVU"));
  inicializujSDLogger();
}

void zapisSDLogAkTreba() {
  if (!sdLoggerDostupny) {
    return;
  }

  const unsigned long teraz = millis();
  if (teraz - casPoslednehoSDLogu < SD_LOG_INTERVAL_MS) {
    return;
  }
  casPoslednehoSDLogu = teraz;
  char nazov[13];
  vytvorDennyNazov(nazov, false);
  File subor = SD.open(nazov, FILE_WRITE);
  if (!subor) {
    vypniSDLogger(F("CHYBA OTVORENIA PRI LOGOVANI - LOGGER DEAKTIVOVANY"));
    return;
  }

  subor.clearWriteError();
  if (subor.size() == 0) zapisHlavickuPrevadzkovehoLogu(subor);
  vypisCasCSV(subor);
  subor.print(',');
  subor.print(teraz);
  subor.print(',');
  vypisHodnotuCSV(subor, teplotaT1, T1_OK, 2);
  subor.print(',');
  subor.print(T1_OK ? 1 : 0);
  subor.print(',');
  vypisHodnotuCSV(subor, teplotaT2, T2_OK, 2);
  subor.print(',');
  subor.print(T2_OK ? 1 : 0);
  subor.print(',');
  vypisHodnotuCSV(subor, teplotaT3, T3_OK, 2);
  subor.print(',');
  subor.print(T3_OK ? 1 : 0);
  subor.print(',');
  vypisHodnotuCSV(subor, teplotaTBOX, TBOX_OK, 2);
  subor.print(',');
  subor.print(TBOX_OK ? 1 : 0);
  subor.print(',');
  vypisHodnotuCSV(subor, sonarVzdialenostCm, sonarStav == SONAR_OK, 1);
  subor.print(',');
  subor.print(textSonarStavu());
  subor.print(',');
  subor.print(aktualnyUnoStav());
  subor.print(',');
  subor.print(megaLinkOk ? 1 : 0);
  subor.print(',');
  subor.print(MEGA_RESULT_VALID ? 1 : 0);
  subor.print(',');
  subor.print(MEGA_RESULT_VALID ? megaVysledok.megaStav : 255);
  subor.print(',');
  vypisHodnotuCSV(subor, megaVysledok.pool, MEGA_RESULT_VALID && megaVysledok.poolOk, 2);
  subor.print(',');
  subor.print(MEGA_RESULT_VALID ? megaVysledok.poolZdroj : 255);
  subor.print(',');
  vypisHodnotuCSV(subor, megaVysledok.t2, MEGA_RESULT_VALID && megaVysledok.t2Ok, 2);
  subor.print(',');
  subor.print(MEGA_RESULT_VALID ? megaVysledok.t2Zdroj : 255);
  subor.print(',');
  subor.println(MEGA_RESULT_VALID ? megaVysledok.diagnostika : 0);

  if (subor.getWriteError()) {
    subor.close();
    vypniSDLogger(F("CHYBA ZAPISU PRI LOGOVANI - LOGGER DEAKTIVOVANY"));
    return;
  }

  subor.close();
  Serial.println(F("SD: ZAZNAM OK"));
}

unsigned int vytvorEventPodpis() {
  unsigned int podpis = 0;
  if (MEGA_RESULT_VALID) podpis |= 0x01;
  if (T1_OK) podpis |= 0x02;
  if (T2_OK) podpis |= 0x04;
  if (TBOX_OK) podpis |= 0x08;
  if (sonarStav == SONAR_OK) podpis |= 0x10;
  if (MEGA_RESULT_VALID && megaVysledok.diagnostika != 0) podpis |= 0x20;
  if (megaLinkOk) podpis |= 0x40;
  if (softCasPlatny) podpis |= 0x80;
  if (T3_OK) podpis |= 0x0100U;
  return podpis;
}

void zapisSenzorovyEventAkZmena() {
  const unsigned int podpis = vytvorEventPodpis();
  if (podpis == poslednyEventPodpis) return;
  poslednyEventPodpis = podpis;
  if (!sdLoggerDostupny) return;
  char nazov[13];
  vytvorDennyNazov(nazov, true);
  File subor = SD.open(nazov, FILE_WRITE);
  if (!subor) {
    vypniSDLogger(F("CHYBA EVENT LOGU - LOGGER DEAKTIVOVANY"));
    return;
  }

  if (subor.size() == 0) {
    zapisHlavickuEventLogu(subor);
  }
  vypisCasCSV(subor);
  subor.print(',');
  subor.print(millis());
  subor.print(',');
  subor.print(podpis);
  subor.print(',');
  subor.print(aktualnyUnoStav());
  subor.print(',');
  subor.print(megaLinkOk ? 1 : 0);
  subor.print(',');
  subor.print(MEGA_RESULT_VALID ? 1 : 0);
  subor.print(',');
  subor.print(MEGA_RESULT_VALID ? megaVysledok.megaStav : 255);
  subor.print(',');
  subor.print(MEGA_RESULT_VALID ? megaVysledok.poolZdroj : 255);
  subor.print(',');
  subor.print(MEGA_RESULT_VALID ? megaVysledok.t2Zdroj : 255);
  subor.print(',');
  subor.println(MEGA_RESULT_VALID ? megaVysledok.diagnostika : 0);

  if (subor.getWriteError()) {
    subor.close();
    vypniSDLogger(F("CHYBA ZAPISU EVENTU - LOGGER DEAKTIVOVANY"));
    return;
  }
  subor.close();
  Serial.println(F("SD: EVENT ZMENA STAVU"));
}

void vypisKratkeDiagnostickeZmeny(unsigned long stary, unsigned long novy) {
  if ((stary ^ novy) & (1UL << 0)) Serial.println((novy & (1UL << 0)) ? F("RECOVERY: T1_OK") : F("EVENT: T1_CHYBA"));
  if ((stary ^ novy) & (1UL << 1)) Serial.println((novy & (1UL << 1)) ? F("RECOVERY: T2_OK") : F("EVENT: T2_CHYBA"));
  if ((stary ^ novy) & (1UL << 2)) Serial.println((novy & (1UL << 2)) ? F("RECOVERY: TBOX_OK") : F("EVENT: TBOX_CHYBA"));
  if ((stary ^ novy) & (1UL << 23)) Serial.println((novy & (1UL << 23)) ? F("RECOVERY: T3_OK") : F("EVENT: T3_CHYBA"));
  if (((stary >> 3) & 0x03) != ((novy >> 3) & 0x03)) {
    const byte stav = (byte)((novy >> 3) & 0x03);
    if (stav == SONAR_OK) Serial.println(F("RECOVERY: SONAR_OK"));
    else if (stav == SONAR_TIMEOUT) Serial.println(F("EVENT: SONAR_TIMEOUT"));
    else Serial.println(F("EVENT: SONAR_MIMO_ROZSAH"));
  }
  if ((stary ^ novy) & (1UL << 5)) Serial.println((novy & (1UL << 5)) ? F("RECOVERY: SD_OK") : F("EVENT: SD_CHYBA"));
  // LINK zmenu vypisuje priamo linkovy automat, aby nevznikol duplicitny riadok.
  if ((stary ^ novy) & (1UL << 7)) Serial.println((novy & (1UL << 7)) ? F("RECOVERY: MEGA_DATA_OK") : F("EVENT: MEGA_DATA_STALE"));
  if (((stary >> 11) & 0xFF) != ((novy >> 11) & 0xFF)) Serial.println(F("EVENT: MEGA_STAV/DIAG_ZMENA"));
}

void vypisDiagnostiku() {
  static unsigned long casPoslednehoVypisu = 0;
  const unsigned long teraz = millis();
  unsigned long podpis = 0;
  if (T1_OK) podpis |= 1UL << 0;
  if (T2_OK) podpis |= 1UL << 1;
  if (TBOX_OK) podpis |= 1UL << 2;
  podpis |= (unsigned long)((byte)sonarStav & 0x03) << 3;
  if (sdLoggerDostupny) podpis |= 1UL << 5;
  if (megaLinkOk) podpis |= 1UL << 6;
  if (MEGA_RESULT_VALID) podpis |= 1UL << 7;
  if (softCasPlatny) podpis |= 1UL << 8;
  podpis |= (unsigned long)(aktualnyUnoStav() & 0x03) << 9;
  if (MEGA_RESULT_VALID) {
    podpis |= (unsigned long)(megaVysledok.megaStav & 0x03) << 11;
    podpis |= (unsigned long)(megaVysledok.diagnostika & 0x3F) << 13;
    podpis |= (unsigned long)(megaVysledok.poolZdroj & 0x03) << 19;
    podpis |= (unsigned long)(megaVysledok.t2Zdroj & 0x03) << 21;
  }
  if (T3_OK) podpis |= 1UL << 23;
  if (!diagnostickyPodpisPlatny) {
    poslednyDiagnostickyPodpis = podpis;
    diagnostickyPodpisPlatny = true;
  }
  else if (podpis != poslednyDiagnostickyPodpis) {
    vypisKratkeDiagnostickeZmeny(poslednyDiagnostickyPodpis, podpis);
    poslednyDiagnostickyPodpis = podpis;
  }
  if (teraz - casPoslednehoVypisu < DIAGNOSTIC_INTERVAL_MS) return;
  casPoslednehoVypisu = teraz;

  Serial.print(F("UNO="));
  const byte unoStav = aktualnyUnoStav();
  if (unoStav == 0) Serial.print(F("OK"));
  else if (unoStav == 1) Serial.print(F("DEGRADED"));
  else Serial.print(F("CHYBA"));
  Serial.print(F(" LINK=")); Serial.print(megaLinkOk ? F("OK") : F("CHYBA"));
  Serial.print(F(" AGR=")); Serial.print(unoAgreementOn ? F("ON") : F("OFF"));
  Serial.print(F(" SD=")); Serial.println(sdLoggerDostupny ? F("OK") : F("CHYBA"));

  Serial.print(F("UNO_XKC="));
  Serial.print(unoXkcLowWater ? F("LOW_WATER") : F("WATER"));
  Serial.print(F(" MEGA_XKC_REMOTE="));
  if (MEGA_RESULT_VALID)
    Serial.print(megaRemoteXkcLowWater() ? F("LOW_WATER") : F("WATER"));
  else
    Serial.print(F("STALE"));
  Serial.print(F(" XKC_CONFLICT="));
  if (!MEGA_RESULT_VALID) Serial.println(F("NA"));
  else Serial.println(unoXkcConflict() ? F("YES") : F("NO"));

  Serial.print(F("T1="));
  if (T1_OK) { Serial.print(teplotaT1, 2); Serial.print(F(" OK")); }
  else Serial.print(F("-- ERR"));
  Serial.print(F(" T2="));
  if (T2_OK) { Serial.print(teplotaT2, 2); Serial.print(F(" OK")); }
  else Serial.print(F("-- ERR"));
  Serial.print(F(" T3="));
  if (T3_OK) { Serial.print(teplotaT3, 2); Serial.print(F(" OK")); }
  else Serial.print(F("-- ERR"));
  Serial.print(F(" TB="));
  if (TBOX_OK) { Serial.print(teplotaTBOX, 2); Serial.println(F(" OK")); }
  else Serial.println(F("-- ERR"));

  Serial.print(F("SON="));
  if (sonarStav == SONAR_OK) {
    Serial.print(sonarVzdialenostCm, 1);
    Serial.println(F(" OK"));
  }
  else if (sonarStav == SONAR_MIMO_ROZSAH) Serial.println(F("ERR/MIMO_ROZSAH"));
  else Serial.println(F("ERR/TIMEOUT"));

  Serial.print(F("LINKERR CRC=")); Serial.print(megaCrcFail);
  Serial.print(F(" INV=")); Serial.print(megaFrameInvalid);
  Serial.print(F(" TO=")); Serial.print(megaLinkTimeoutCount);
  Serial.print(F(" GAP=")); Serial.println(megaSeqGapCount);
}

void setup() {
  Serial.begin(115200);

  pinMode(UNO_XKC_PIN, INPUT_PULLUP);
  unoXkcLowWater = digitalRead(UNO_XKC_PIN) == HIGH;
  unoXkcInicializovany = true;

  // Fail-safe boot: bez schvalenej TOTAL STOP podmienky zostava COM-NC.
  digitalWrite(UNO_TOTAL_STOP_PIN, LOW);
  pinMode(UNO_TOTAL_STOP_PIN, OUTPUT);
  digitalWrite(UNO_TOTAL_STOP_PIN, LOW);

  // Fail-safe boot: agreement je zakazane skor, nez sa spusti V5 linka.
  digitalWrite(UNO_HL_RELAY_1_PIN, LOW);
  pinMode(UNO_HL_RELAY_1_PIN, OUTPUT);
  digitalWrite(UNO_HL_RELAY_1_PIN, LOW);

  inicializujMegaLinkTest();

  pinMode(SONAR_TRIG_PIN, OUTPUT);
  pinMode(SONAR_ECHO_PIN, INPUT);
  digitalWrite(SONAR_TRIG_PIN, LOW);

  sensors.begin();
  sensors.setResolution(T1_ADDRESS, 10);
  sensors.setResolution(T2_ADDRESS, 10);
  sensors.setResolution(T3_ADDRESS, 10);
  sensors.setResolution(TBOX_ADDRESS, 10);
  sensors.setWaitForConversion(false);

  inicializujSDLogger();

  // Prve merania sa mozu zacat okamzite.
  casPoslednehoStartuTeploty = millis() - TEMPERATURE_INTERVAL_MS;
  casPoslednejReinicializacieOneWire = millis();
  sonarPoslednyStartUs = micros() - SONAR_INTERVAL_US;

  Serial.println(F("UNO START V5"));
  Serial.println(F("UNO DS18B20 MAPA:"));
  Serial.println(F("UNO_T1 = BAZEN"));
  Serial.println(F("UNO_T2 = SOLAR VYSTUP"));
  Serial.println(F("UNO_T3 = SOLAR PANEL"));
  Serial.println(F("UNO_TBOX = ROZVADZAC"));
}

void loop() {
  aktualizujTeploty();
  aktualizujSonar();
  aktualizujUnoXkc();
  aktualizujMegaLinkTest();
  vypisDiagnostiku();
  aktualizujSDRecovery();
  zapisSenzorovyEventAkZmena();
  zapisSDLogAkTreba();
}
