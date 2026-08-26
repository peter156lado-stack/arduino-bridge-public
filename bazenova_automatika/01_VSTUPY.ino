// ==================================================
// 01_VSTUPY – MERANIE TEPLOTY
// ==================================================

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>

// --------------------------------------------------
// DS18B20
// --------------------------------------------------

#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// --------------------------------------------------
// MEGA HY-SRF05 - MONITOROVANIE HLADINY
// --------------------------------------------------

const byte MEGA_SONAR_TRIG_PIN = 38;
const byte MEGA_SONAR_ECHO_PIN = 39;

const unsigned long MEGA_SONAR_INTERVAL_US = 250000UL;
const unsigned long MEGA_SONAR_TRIGGER_LOW_US = 3UL;
const unsigned long MEGA_SONAR_TRIGGER_HIGH_US = 10UL;
const unsigned long MEGA_SONAR_TIMEOUT_US = 30000UL;
const float MEGA_SONAR_MIN_CM = 2.0f;
const float MEGA_SONAR_MAX_CM = 450.0f;

enum MegaSonarFaza : byte {
  MEGA_SONAR_CAKA,
  MEGA_SONAR_TRIGGER_LOW,
  MEGA_SONAR_TRIGGER_HIGH,
  MEGA_SONAR_CAKA_ECHO_HIGH,
  MEGA_SONAR_CAKA_ECHO_LOW
};

enum MegaSonarStav : byte {
  MEGA_SONAR_OK,
  MEGA_SONAR_TIMEOUT,
  MEGA_SONAR_CHYBA
};

MegaSonarFaza megaSonarFaza = MEGA_SONAR_CAKA;
MegaSonarStav megaSonarStav = MEGA_SONAR_TIMEOUT;
float megaSonarVzdialenostCm = 0.0f;
unsigned long megaSonarCasFazyUs = 0;
unsigned long megaSonarEchoStartUs = 0;
unsigned long megaSonarPoslednyStartUs = 0;

void dokonciMegaSonarTimeout() {
  megaSonarStav = MEGA_SONAR_TIMEOUT;
  megaSonarFaza = MEGA_SONAR_CAKA;
  megaSonarPoslednyStartUs = micros();
}

void aktualizujMegaSonar() {
  const unsigned long terazUs = micros();

  switch (megaSonarFaza) {
    case MEGA_SONAR_CAKA:
      if (terazUs - megaSonarPoslednyStartUs >= MEGA_SONAR_INTERVAL_US) {
        digitalWrite(MEGA_SONAR_TRIG_PIN, LOW);
        megaSonarCasFazyUs = terazUs;
        megaSonarFaza = MEGA_SONAR_TRIGGER_LOW;
      }
      break;

    case MEGA_SONAR_TRIGGER_LOW:
      if (terazUs - megaSonarCasFazyUs >= MEGA_SONAR_TRIGGER_LOW_US) {
        digitalWrite(MEGA_SONAR_TRIG_PIN, HIGH);
        megaSonarCasFazyUs = terazUs;
        megaSonarFaza = MEGA_SONAR_TRIGGER_HIGH;
      }
      break;

    case MEGA_SONAR_TRIGGER_HIGH:
      if (terazUs - megaSonarCasFazyUs >= MEGA_SONAR_TRIGGER_HIGH_US) {
        digitalWrite(MEGA_SONAR_TRIG_PIN, LOW);
        megaSonarCasFazyUs = terazUs;
        megaSonarFaza = MEGA_SONAR_CAKA_ECHO_HIGH;
      }
      break;

    case MEGA_SONAR_CAKA_ECHO_HIGH:
      if (digitalRead(MEGA_SONAR_ECHO_PIN) == HIGH) {
        megaSonarEchoStartUs = terazUs;
        megaSonarFaza = MEGA_SONAR_CAKA_ECHO_LOW;
      }
      else if (terazUs - megaSonarCasFazyUs >= MEGA_SONAR_TIMEOUT_US) {
        dokonciMegaSonarTimeout();
      }
      break;

    case MEGA_SONAR_CAKA_ECHO_LOW:
      if (digitalRead(MEGA_SONAR_ECHO_PIN) == LOW) {
        const unsigned long echoDlzkaUs = terazUs - megaSonarEchoStartUs;
        const float novaVzdialenostCm = echoDlzkaUs * 0.0343f / 2.0f;

        if (novaVzdialenostCm >= MEGA_SONAR_MIN_CM &&
            novaVzdialenostCm <= MEGA_SONAR_MAX_CM) {
          megaSonarVzdialenostCm = novaVzdialenostCm;
          megaSonarStav = MEGA_SONAR_OK;
        }
        else {
          megaSonarStav = MEGA_SONAR_CHYBA;
        }

        megaSonarFaza = MEGA_SONAR_CAKA;
        megaSonarPoslednyStartUs = terazUs;
      }
      else if (terazUs - megaSonarEchoStartUs >= MEGA_SONAR_TIMEOUT_US) {
        dokonciMegaSonarTimeout();
      }
      break;
  }
}

void vypisMegaSonar() {
  Serial.print("MEGA_SONAR: ");

  if (megaSonarStav == MEGA_SONAR_OK) {
    Serial.print(megaSonarVzdialenostCm, 1);
    Serial.println(" cm [MEGA_SONAR_OK]");
  }
  else if (megaSonarStav == MEGA_SONAR_CHYBA) {
    Serial.println("-- [MEGA_SONAR_CHYBA]");
  }
  else {
    Serial.println("-- [MEGA_SONAR_TIMEOUT]");
  }
}

// --------------------------------------------------
// AHT10 - VONKAJSIA REFERENCNA TEPLOTA A VLHKOST
// --------------------------------------------------

const byte AHT10_ADRESA = 0x38;
Adafruit_AHTX0 aht10;
bool aht10Dostupny = false;
float teplotaVonku = NAN;
float vlhkostVonku = NAN;
const unsigned long AHT10_INTERVAL_OBNOVY = 5000UL;
unsigned long casPoslednehoPokusuAHT10 = 0;

void inicializaciaAHT10() {

  aht10Dostupny = aht10.begin(&Wire, 0, AHT10_ADRESA);
  casPoslednehoPokusuAHT10 = millis();

  Serial.print("AHT10: ");
  Serial.println(aht10Dostupny ? "OK" : "CHYBA");
}

void nacitajAHT10() {

  if (!aht10Dostupny) {

    if (millis() - casPoslednehoPokusuAHT10 >= AHT10_INTERVAL_OBNOVY) {

      casPoslednehoPokusuAHT10 = millis();

      if (aht10.begin(&Wire, 0, AHT10_ADRESA)) {

        aht10Dostupny = true;
        Serial.println("AHT10: OBNOVENY");
      }
    }

    if (!aht10Dostupny) {
      return;
    }
  }

  sensors_event_t vlhkostUdalost;
  sensors_event_t teplotaUdalost;

  if (!aht10.getEvent(&vlhkostUdalost, &teplotaUdalost) ||
      isnan(teplotaUdalost.temperature) || isnan(vlhkostUdalost.relative_humidity)) {

    aht10Dostupny = false;
    teplotaVonku = NAN;
    vlhkostVonku = NAN;
    casPoslednehoPokusuAHT10 = millis();

    Serial.println("AHT10: CHYBA");
    return;
  }

  teplotaVonku = teplotaUdalost.temperature;
  vlhkostVonku = vlhkostUdalost.relative_humidity;
}

// --------------------------------------------------
// DS3231 RTC
// --------------------------------------------------

const byte RTC_ADRESA = 0x68;
bool rtcDostupne = false;

byte bcdNaDecimal(byte hodnota) {

  return (hodnota >> 4) * 10 + (hodnota & 0x0F);
}

byte decimalNaBCD(byte hodnota) {

  return ((hodnota / 10) << 4) | (hodnota % 10);
}

void vypisDvojciferne(byte hodnota) {

  if (hodnota < 10) {
    Serial.print('0');
  }

  Serial.print(hodnota);
}

bool nacitajRTC(byte &hodina, byte &minuta, byte &sekunda,
                byte &den, byte &mesiac, byte &rok) {

  const unsigned long rtcStartUs = micros();

  Wire.beginTransmission(RTC_ADRESA);
  Wire.write(0x00);

  if (Wire.endTransmission(false) != 0) {
    zaznamenajMegaRtcCas(rtcStartUs);
    return false;
  }

  if (Wire.requestFrom(RTC_ADRESA, (byte)7) != 7) {
    zaznamenajMegaRtcCas(rtcStartUs);
    return false;
  }

  byte suroveSekundy = Wire.read();
  byte suroveMinuty = Wire.read();
  byte suroveHodiny = Wire.read();
  Wire.read();  // den v tyzdni
  byte surovyDen = Wire.read();
  byte surovyMesiac = Wire.read();
  byte surovyRok = Wire.read();

  sekunda = bcdNaDecimal(suroveSekundy & 0x7F);
  minuta = bcdNaDecimal(suroveMinuty & 0x7F);

  if (suroveHodiny & 0x40) {
    hodina = bcdNaDecimal(suroveHodiny & 0x1F);

    if ((suroveHodiny & 0x20) && hodina < 12) {
      hodina += 12;
    }
  }
  else {
    hodina = bcdNaDecimal(suroveHodiny & 0x3F);
  }

  den = bcdNaDecimal(surovyDen & 0x3F);
  mesiac = bcdNaDecimal(surovyMesiac & 0x1F);
  rok = bcdNaDecimal(surovyRok);

  zaznamenajMegaRtcCas(rtcStartUs);
  return true;
}

bool nacitajRegisterRTC(byte registerRTC, byte &hodnota) {

  const unsigned long rtcStartUs = micros();

  Wire.beginTransmission(RTC_ADRESA);
  Wire.write(registerRTC);

  if (Wire.endTransmission(false) != 0) {
    zaznamenajMegaRtcCas(rtcStartUs);
    return false;
  }

  if (Wire.requestFrom(RTC_ADRESA, (byte)1) != 1) {
    zaznamenajMegaRtcCas(rtcStartUs);
    return false;
  }

  hodnota = Wire.read();
  zaznamenajMegaRtcCas(rtcStartUs);
  return true;
}

byte mesiacKompilacie() {

  if (__DATE__[0] == 'J' && __DATE__[1] == 'a') return 1;
  if (__DATE__[0] == 'F') return 2;
  if (__DATE__[0] == 'M' && __DATE__[2] == 'r') return 3;
  if (__DATE__[0] == 'A' && __DATE__[1] == 'p') return 4;
  if (__DATE__[0] == 'M' && __DATE__[2] == 'y') return 5;
  if (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n') return 6;
  if (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l') return 7;
  if (__DATE__[0] == 'A' && __DATE__[1] == 'u') return 8;
  if (__DATE__[0] == 'S') return 9;
  if (__DATE__[0] == 'O') return 10;
  if (__DATE__[0] == 'N') return 11;

  return 12;
}

bool rtcCasJePlatny() {

  byte statusRTC;
  byte hodina;
  byte minuta;
  byte sekunda;
  byte den;
  byte mesiac;
  byte rok;

  if (!nacitajRegisterRTC(0x0F, statusRTC) || (statusRTC & 0x80)) {
    return false;
  }

  if (!nacitajRTC(hodina, minuta, sekunda, den, mesiac, rok)) {
    return false;
  }

  return hodina < 24 && minuta < 60 && sekunda < 60 &&
         den >= 1 && den <= 31 && mesiac >= 1 && mesiac <= 12;
}

bool nastavRTCZKompilacie() {

  byte den = (__DATE__[4] == ' ' ? 0 : (__DATE__[4] - '0') * 10) + (__DATE__[5] - '0');
  byte mesiac = mesiacKompilacie();
  byte rok = (__DATE__[9] - '0') * 10 + (__DATE__[10] - '0');
  byte hodina = (__TIME__[0] - '0') * 10 + (__TIME__[1] - '0');
  byte minuta = (__TIME__[3] - '0') * 10 + (__TIME__[4] - '0');
  byte sekunda = (__TIME__[6] - '0') * 10 + (__TIME__[7] - '0');
  byte statusRTC;

  Wire.beginTransmission(RTC_ADRESA);
  Wire.write(0x00);
  Wire.write(decimalNaBCD(sekunda));
  Wire.write(decimalNaBCD(minuta));
  Wire.write(decimalNaBCD(hodina));
  Wire.write(1);  // den v tyzdni nie je pre tuto etapu pouzity
  Wire.write(decimalNaBCD(den));
  Wire.write(decimalNaBCD(mesiac));
  Wire.write(decimalNaBCD(rok));

  if (Wire.endTransmission() != 0) {
    return false;
  }

  if (!nacitajRegisterRTC(0x0F, statusRTC)) {
    return false;
  }

  Wire.beginTransmission(RTC_ADRESA);
  Wire.write(0x0F);
  Wire.write(statusRTC & 0x7F);

  return Wire.endTransmission() == 0;
}

void inicializaciaRTC() {

  Wire.begin();

  Wire.beginTransmission(RTC_ADRESA);
  rtcDostupne = Wire.endTransmission() == 0;

  if (rtcDostupne && !rtcCasJePlatny()) {
    rtcDostupne = nastavRTCZKompilacie();
  }

  Serial.print("RTC: ");
  Serial.println(rtcDostupne ? "OK" : "CHYBA");
}

void skenujI2C() {

  byte pocetZariadeni = 0;

  Serial.println("I2C SCAN: START");

  for (byte adresa = 1; adresa < 127; adresa++) {

    Wire.beginTransmission(adresa);

    if (Wire.endTransmission() == 0) {

      Serial.print("I2C: 0x");

      if (adresa < 0x10) {
        Serial.print('0');
      }

      Serial.println(adresa, HEX);
      pocetZariadeni++;
    }
  }

  if (pocetZariadeni == 0) {
    Serial.println("I2C: ZIADNE ZARIADENIE");
  }

  Serial.println("I2C SCAN: KONIEC");
}

void vypisRTC() {

  byte hodina;
  byte minuta;
  byte sekunda;
  byte den;
  byte mesiac;
  byte rok;

  rtcDostupne = nacitajRTC(hodina, minuta, sekunda, den, mesiac, rok);

  if (!rtcDostupne) {
    Serial.println("RTC: CHYBA");
    return;
  }

  Serial.print("CAS: ");
  vypisDvojciferne(hodina);
  Serial.print(':');
  vypisDvojciferne(minuta);
  Serial.print(':');
  vypisDvojciferne(sekunda);
  Serial.println();

  Serial.print("DATUM: ");
  vypisDvojciferne(den);
  Serial.print('.');
  vypisDvojciferne(mesiac);
  Serial.print(".20");
  vypisDvojciferne(rok);
  Serial.println();
}

// --------------------------------------------------
// ADRESY SENZOROV
// --------------------------------------------------

// T1 – bazén
DeviceAddress T1 = {
  0x28, 0x3A, 0xCF, 0x78,
  0x91, 0x25, 0x06, 0xA5
};

// T2 – solárny výstup
DeviceAddress T2 = {
  0x28, 0xE6, 0x74, 0x05,
  0x91, 0x25, 0x06, 0x16
};

// T3 – solárny panel
DeviceAddress T3 = {
  0x28, 0x07, 0x79, 0x7E,
  0x91, 0x25, 0x06, 0xD8
};

// T4 - dno bazena
DeviceAddress T4 = {
  0x28, 0xFE, 0xFA, 0x7D,
  0x91, 0x25, 0x06, 0x31
};
bool T4_ADRESA_NASTAVENA = true;

// MEGA_TBOX - vnutorna teplota rozvadzaca
DeviceAddress MEGA_TBOX = {
  0x28, 0xB1, 0x31, 0x05,
  0x00, 0x03, 0x24, 0xF1
};

// --------------------------------------------------
// NAMERANÉ TEPLOTY
// --------------------------------------------------

float t1 = DEVICE_DISCONNECTED_C;
float t2 = DEVICE_DISCONNECTED_C;
float t3 = DEVICE_DISCONNECTED_C;
float t4 = DEVICE_DISCONNECTED_C;
float megaTbox = DEVICE_DISCONNECTED_C;
bool MEGA_TBOX_OK = false;

const unsigned long MEGA_DS_CONVERSION_MS = 750UL;
bool megaDsKonverziaBezi = false;
unsigned long megaDsKonverziaStartMs = 0;

void spustiMegaDsKonverziu() {
  sensors.requestTemperatures();
  megaDsKonverziaStartMs = millis();
  megaDsKonverziaBezi = true;
}

// --------------------------------------------------
// FUNKCIA – NAČÍTANIE TEPLOT
// --------------------------------------------------

void nacitajTeploty() {

  megaDsLastUs = 0;
  const unsigned long teraz = millis();
  if (megaDsKonverziaBezi &&
      teraz - megaDsKonverziaStartMs >= MEGA_DS_CONVERSION_MS) {
    const unsigned long dsStartUs = micros();

    const float novyT1 = sensors.getTempC(T1);
    const float novyT2 = sensors.getTempC(T2);
    const float novyT3 = sensors.getTempC(T3);
    const float novyT4 = T4_ADRESA_NASTAVENA
                           ? sensors.getTempC(T4)
                           : DEVICE_DISCONNECTED_C;
    const float novyMegaTbox = sensors.getTempC(MEGA_TBOX);
    const bool novyMegaTboxOk = novyMegaTbox > -10 && novyMegaTbox < 100;

    // Zverejnenie celeho spolocneho snapshotu az po vsetkych citaniach.
    t1 = novyT1;
    t2 = novyT2;
    t3 = novyT3;
    t4 = novyT4;
    megaTbox = novyMegaTbox;
    MEGA_TBOX_OK = novyMegaTboxOk;

    // READ predchadzajucej konverzie -> START nasledujucej konverzie.
    spustiMegaDsKonverziu();
    megaDsLastUs = micros() - dsStartUs;
    aktualizujMegaTimingMax(megaDsMaxUs, megaDsLastUs);
  }

  const unsigned long ahtStartUs = micros();
  nacitajAHT10();
  megaAhtLastUs = micros() - ahtStartUs;
  aktualizujMegaTimingMax(megaAhtMaxUs, megaAhtLastUs);

  if (megaDiagnostickyVypis) {
  Serial.print("T1 bazen: ");
  Serial.print(t1);
  Serial.println(" C");

  Serial.print("T2 solar vystup: ");
  Serial.print(t2);
  Serial.println(" C");

  Serial.print("T3 solar panel: ");
  Serial.print(t3);
  Serial.println(" C");

  Serial.print("T4 dno bazena: ");
  if (T4_ADRESA_NASTAVENA) {
    Serial.print(t4);
    Serial.println(" C");
  }
  else {
    Serial.println("ADRESA NENASTAVENA");
  }

  if (aht10Dostupny) {
    Serial.print("VONKU: ");
    Serial.print(teplotaVonku, 1);
    Serial.print(" C  RH: ");
    Serial.print(vlhkostVonku, 1);
    Serial.println(" %");
  }
  else {
    Serial.println("VONKU: SENSOR ERR");
  }

  Serial.print("MEGA_TBOX rozvadzac: ");
  if (MEGA_TBOX_OK) {
    Serial.print(megaTbox);
    Serial.println(" C [MEGA_TBOX_OK]");
  }
  else {
    Serial.println("-- [MEGA_TBOX_CHYBA]");
  }

  vypisMegaSonar();
  }
}

bool porovnajAdresyDS18B20(const uint8_t *prva, const uint8_t *druha) {

  for (byte bajt = 0; bajt < 8; bajt++) {
    if (prva[bajt] != druha[bajt]) {
      return false;
    }
  }

  return true;
}

void vypisAdresyDS18B20() {

  DeviceAddress adresa;
  byte pocetSenzorov = sensors.getDeviceCount();

  Serial.println(F("MEGA DS18B20 MAPA:"));
  Serial.println(F("H1 / MEGA_T1 = BAZEN"));
  Serial.println(F("H2 / MEGA_T2 = SOLAR VYSTUP"));
  Serial.println(F("G2 / MEGA_T3 = SOLAR PANEL"));
  Serial.println(F("H3 / MEGA_T4 = DNO BAZENA"));
  Serial.println(F("MEGA_TBOX = ROZVADZAC"));

  for (byte index = 0; index < pocetSenzorov; index++) {

    if (!sensors.getAddress(adresa, index)) {
      continue;
    }

    Serial.print("DS18B20 #");
    Serial.print(index + 1);
    Serial.print(": ");

    for (byte bajt = 0; bajt < 8; bajt++) {

      if (adresa[bajt] < 0x10) {
        Serial.print('0');
      }

      Serial.print(adresa[bajt], HEX);

      if (bajt < 7) {
        Serial.print(' ');
      }
    }

    if (porovnajAdresyDS18B20(adresa, T1)) {
      Serial.print(F(" [H1 / MEGA_T1 = BAZEN]"));
    }
    else if (porovnajAdresyDS18B20(adresa, T2)) {
      Serial.print(F(" [H2 / MEGA_T2 = SOLAR VYSTUP]"));
    }
    else if (porovnajAdresyDS18B20(adresa, T3)) {
      Serial.print(F(" [G2 / MEGA_T3 = SOLAR PANEL]"));
    }
    else if (porovnajAdresyDS18B20(adresa, T4)) {
      Serial.print(F(" [H3 / MEGA_T4 = DNO BAZENA]"));
    }
    else if (porovnajAdresyDS18B20(adresa, MEGA_TBOX)) {
      Serial.print(F(" [MEGA_TBOX = ROZVADZAC]"));
    }
    else {
      Serial.print(" [NEPRIRADENY]");
    }

    Serial.println();
  }
}

// ==================================================
// INICIALIZACIA VSTUPOV
// ==================================================

void inicializaciaVstupov() {

  pinMode(MEGA_SONAR_TRIG_PIN, OUTPUT);
  pinMode(MEGA_SONAR_ECHO_PIN, INPUT);
  digitalWrite(MEGA_SONAR_TRIG_PIN, LOW);
  megaSonarPoslednyStartUs = micros() - MEGA_SONAR_INTERVAL_US;

  sensors.begin();
  sensors.setWaitForConversion(false);

  vypisAdresyDS18B20();

  // Boot pipeline: hodnoty su INVALID a prva konverzia bezi na pozadi.
  spustiMegaDsKonverziu();

  Serial.println("VSTUPY: inicializovane");
}
