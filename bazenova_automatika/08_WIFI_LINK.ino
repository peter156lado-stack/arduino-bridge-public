// ==================================================
// 08_WIFI_LINK - MEGA <-> ESP8266 CEZ SERIAL3
// ==================================================

#include <stdlib.h>
#include <string.h>

const unsigned long WIFI_BAUD = 115200UL;
const unsigned long WIFI_DATA_INTERVAL = 1000UL;
const size_t WIFI_RX_BUFFER_VELKOST = 48;
// Web telemetry additions only; the Mega<->Uno V4 frames and safety behaviour stay unchanged.
const size_t WIFI_TX_BUFFER_VELKOST = 512;

char wifiRxBuffer[WIFI_RX_BUFFER_VELKOST];
size_t wifiRxDlzka = 0;
bool wifiRxPretecenie = false;

char wifiTxBuffer[WIFI_TX_BUFFER_VELKOST];
size_t wifiTxDlzka = 0;
size_t wifiTxPozicia = 0;
unsigned long casPoslednychDatWifi = 0;

bool wifiPridajText(const char *text) {

  size_t dlzka = strlen(text);
  if (wifiTxDlzka + dlzka >= WIFI_TX_BUFFER_VELKOST) return false;

  memcpy(wifiTxBuffer + wifiTxDlzka, text, dlzka);
  wifiTxDlzka += dlzka;
  wifiTxBuffer[wifiTxDlzka] = '\0';
  return true;
}

bool wifiPridajFloat(float hodnota, bool platna) {

  if (!platna || isnan(hodnota) || isinf(hodnota)) {
    return wifiPridajText("NA");
  }

  char textHodnoty[16];
  dtostrf(hodnota, 0, 1, textHodnoty);
  return wifiPridajText(textHodnoty);
}

void wifiPridajDvojciferne(byte hodnota) {

  char cislo[3];
  cislo[0] = '0' + hodnota / 10;
  cislo[1] = '0' + hodnota % 10;
  cislo[2] = '\0';
  wifiPridajText(cislo);
}

void pripravWifiData() {

  wifiTxDlzka = 0;
  wifiTxPozicia = 0;
  wifiTxBuffer[0] = '\0';

  wifiPridajText("DATA;VER=1;T1=");
  wifiPridajFloat(t1, T1_OK);
  wifiPridajText(";T2=");
  wifiPridajFloat(t2, T2_OK);
  wifiPridajText(";T3=");
  wifiPridajFloat(t3, T3_OK);
  wifiPridajText(";T4=");
  wifiPridajFloat(t4, T4_OK);
  wifiPridajText(";OUT=");
  wifiPridajFloat(teplotaVonku, aht10Dostupny);
  wifiPridajText(";RH=");
  wifiPridajFloat(vlhkostVonku, aht10Dostupny);
  wifiPridajText(";BAZEN=");
  wifiPridajFloat(teplotaBazena, T1_OK || T4_OK);
  wifiPridajText(";SET=");
  wifiPridajFloat(MAX_BAZEN, true);
  wifiPridajText(";SETMIN=");
  wifiPridajFloat(MAX_BAZEN_MIN, true);
  wifiPridajText(";SETMAX=");
  wifiPridajFloat(MAX_BAZEN_MAX, true);

  wifiPridajText(";SYS=");
  if (!system_OK) wifiPridajText("HAVARIA");
  else if (systemDegradovany) wifiPridajText("DEGRADED");
  else wifiPridajText("OK");

  wifiPridajText(";SOLAR=");
  wifiPridajText(solarZapnuty ? "1" : "0");
  wifiPridajText(";FIL=");
  wifiPridajText(filtraciaZapnuta ? "1" : "0");
  wifiPridajText(";FILMAN=");
  wifiPridajText(manualFiltracia6h ? "1" : "0");
  wifiPridajText(";CHR=");
  wifiPridajText(chrlicManualAktivny ? "1" : "0");
  // Optional web-only diagnostics. Older ESP firmware safely ignores these fields.
  wifiPridajText(";TBOX=");
  wifiPridajFloat(MEGA_TBOX_OK ? megaTbox : unoRemoteTboxHodnota(),
                   MEGA_TBOX_OK || unoRemoteTboxPlatna());
  wifiPridajText(";MODE=");
  switch (systemMode) {
    case MODE_SMART: wifiPridajText("SMART"); break;
    case MODE_DEGRADED: wifiPridajText("DEGRADED"); break;
    case MODE_BASIC: wifiPridajText("BASIC"); break;
    case MODE_STOP: wifiPridajText("STOP"); break;
    default: wifiPridajText("NA"); break;
  }
  wifiPridajText(";ULINK=");
  wifiPridajText(unoLinkStavOk ? "1" : "0");
  wifiPridajText(";MAGR=");
  wifiPridajText(megaAgreementOn ? "1" : "0");
  wifiPridajText(";UAGR=");
  wifiPridajText(unoAgreementOnRemote ? "1" : "0");

  // Rele moduly su aktivne v LOW. Iba R9 a R10 su v projekte riadene.
  wifiPridajText(";R1=NA;R2=NA;R3=NA;R4=NA;R5=NA;R6=NA;R7=NA;R8=NA");
  wifiPridajText(";R9=");
  wifiPridajText(digitalRead(R9) == LOW ? "1" : "0");
  wifiPridajText(";R10=");
  wifiPridajText(digitalRead(R10) == LOW ? "1" : "0");
  wifiPridajText(";R11=NA;R12=NA;R13=NA;R14=NA;R15=NA;R16=NA");

  byte hodina;
  byte minuta;
  byte sekunda;
  byte den;
  byte mesiac;
  byte rok;

  if (nacitajRTC(hodina, minuta, sekunda, den, mesiac, rok)) {
    wifiPridajText(";TIME=");
    wifiPridajDvojciferne(hodina);
    wifiPridajText(":");
    wifiPridajDvojciferne(minuta);
    wifiPridajText(":");
    wifiPridajDvojciferne(sekunda);
    wifiPridajText(";DATE=");
    wifiPridajDvojciferne(den);
    wifiPridajText(".");
    wifiPridajDvojciferne(mesiac);
    wifiPridajText(".20");
    wifiPridajDvojciferne(rok);
  }
  else {
    wifiPridajText(";TIME=NA;DATE=NA");
  }

  wifiPridajText("\n");
}

void spracujWifiPrikaz(const char *prikaz) {

  if (strncmp(prikaz, "SETTEMP=", 8) == 0) {
    char *koniec;
    float novaTeplota = strtod(prikaz + 8, &koniec);

    if (*koniec == '\0' && !isnan(novaTeplota) && !isinf(novaTeplota) &&
        novaTeplota >= MAX_BAZEN_MIN && novaTeplota <= MAX_BAZEN_MAX) {
      MAX_BAZEN = round(novaTeplota * 10.0) / 10.0;
      ulozMaxBazenDoEEPROM();

      Serial.print("WIFI: SETTEMP=");
      Serial.println(MAX_BAZEN, 1);
    }
    return;
  }

  if (strcmp(prikaz, "FIL6H=TOGGLE") == 0) {
    prepnIManualnuFiltraciu();
    Serial.println("WIFI: FIL6H=TOGGLE");
    return;
  }

  if (strcmp(prikaz, "CHR1H=TOGGLE") == 0) {
    prepnIManualnyChrlic();
    Serial.println("WIFI: CHR1H=TOGGLE");
  }
}

void prijmiWifiPrikazy() {

  while (Serial3.available() > 0) {
    char znak = (char)Serial3.read();

    if (znak == '\n') {
      if (!wifiRxPretecenie && wifiRxDlzka > 0) {
        wifiRxBuffer[wifiRxDlzka] = '\0';
        spracujWifiPrikaz(wifiRxBuffer);
      }

      wifiRxDlzka = 0;
      wifiRxPretecenie = false;
    }
    else if (znak != '\r' && !wifiRxPretecenie) {
      if (wifiRxDlzka < WIFI_RX_BUFFER_VELKOST - 1) {
        wifiRxBuffer[wifiRxDlzka++] = znak;
      }
      else {
        wifiRxPretecenie = true;
      }
    }
  }
}

void odosliWifiDataBezCakania() {

  if (wifiTxPozicia >= wifiTxDlzka) return;

  int volneMiesto = Serial3.availableForWrite();
  if (volneMiesto <= 0) return;

  size_t zostava = wifiTxDlzka - wifiTxPozicia;
  size_t pocet = zostava < (size_t)volneMiesto ? zostava : (size_t)volneMiesto;
  Serial3.write((const uint8_t *)(wifiTxBuffer + wifiTxPozicia), pocet);
  wifiTxPozicia += pocet;
}

void inicializaciaWifiLink() {

  Serial3.begin(WIFI_BAUD);
  casPoslednychDatWifi = millis();
}

void aktualizujWifiLink() {

  prijmiWifiPrikazy();
  odosliWifiDataBezCakania();

  unsigned long teraz = millis();
  if (wifiTxPozicia >= wifiTxDlzka &&
      teraz - casPoslednychDatWifi >= WIFI_DATA_INTERVAL) {
    casPoslednychDatWifi = teraz;
    pripravWifiData();
    odosliWifiDataBezCakania();
  }
}
