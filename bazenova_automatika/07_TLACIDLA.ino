// ==================================================
// 07_TLACIDLA – MANUALNE POZIADAVKY
// ==================================================

#include <EEPROM.h>

const int EEPROM_ADRESA_MAX_BAZEN = 0;
const unsigned long OPAKOVANIE_START = 1000UL;

bool nastavenieTeplotyAktivne = false;
unsigned long casPoslednejAktivityNastavenia = 0;

bool poslednyNacitanyStavTlacidlaSet = HIGH;
bool stabilnyStavTlacidlaSet = HIGH;
unsigned long casZmenyTlacidlaSet = 0;

bool poslednyNacitanyStavTlacidlaPlus = HIGH;
bool stabilnyStavTlacidlaPlus = HIGH;
unsigned long casZmenyTlacidlaPlus = 0;
unsigned long casStlaceniaTlacidlaPlus = 0;
unsigned long casPoslednehoOpakovaniaPlus = 0;

bool poslednyNacitanyStavTlacidlaMinus = HIGH;
bool stabilnyStavTlacidlaMinus = HIGH;
unsigned long casZmenyTlacidlaMinus = 0;
unsigned long casStlaceniaTlacidlaMinus = 0;
unsigned long casPoslednehoOpakovaniaMinus = 0;

bool manualFiltracia6h = false;
bool chrlicManualAktivny = false;

unsigned long casZaciatkuManualnejFiltracie = 0;
unsigned long casZaciatkuManualnehoChrlica = 0;

bool poslednyNacitanyStavTlacidlaFiltracie = HIGH;
bool stabilnyStavTlacidlaFiltracie = HIGH;
unsigned long casZmenyTlacidlaFiltracie = 0;

bool poslednyNacitanyStavTlacidlaChrlica = HIGH;
bool stabilnyStavTlacidlaChrlica = HIGH;
unsigned long casZmenyTlacidlaChrlica = 0;

void nacitajMaxBazenZEEPROM() {

  float ulozenaHodnota;
  EEPROM.get(EEPROM_ADRESA_MAX_BAZEN, ulozenaHodnota);

  if (isnan(ulozenaHodnota) || ulozenaHodnota < MAX_BAZEN_MIN ||
      ulozenaHodnota > MAX_BAZEN_MAX) {
    MAX_BAZEN = MAX_BAZEN_PREDVOLENA;
    Serial.println("MAX BAZEN EEPROM: NEPLATNA, POUZITA HODNOTA 32.0 C");
  }
  else {
    MAX_BAZEN = round(ulozenaHodnota * 10.0) / 10.0;
    Serial.print("MAX BAZEN EEPROM: ");
    Serial.print(MAX_BAZEN, 1);
    Serial.println(" C");
  }
}

void zmenMaxBazen(int zmenaDesatin) {

  int hodnotaDesatiny = (int)round(MAX_BAZEN * 10.0) + zmenaDesatin;
  int minimumDesatiny = (int)round(MAX_BAZEN_MIN * 10.0);
  int maximumDesatiny = (int)round(MAX_BAZEN_MAX * 10.0);

  if (hodnotaDesatiny < minimumDesatiny) {
    hodnotaDesatiny = minimumDesatiny;
  }
  else if (hodnotaDesatiny > maximumDesatiny) {
    hodnotaDesatiny = maximumDesatiny;
  }

  MAX_BAZEN = hodnotaDesatiny / 10.0;
  casPoslednejAktivityNastavenia = millis();
  vykresliNastavenieBazenaLCD();
}

unsigned long intervalOpakovaniaTlacidla(unsigned long dlzkaDrzania) {

  const unsigned long POCIATOCNY_INTERVAL = 350UL;
  const unsigned long NAJKRATSI_INTERVAL = 75UL;
  const unsigned long CAS_ZRYCHLOVANIA = 5000UL;

  if (dlzkaDrzania <= OPAKOVANIE_START) return POCIATOCNY_INTERVAL;

  unsigned long casPoSpusteni = dlzkaDrzania - OPAKOVANIE_START;
  if (casPoSpusteni >= CAS_ZRYCHLOVANIA) return NAJKRATSI_INTERVAL;

  return POCIATOCNY_INTERVAL -
         (casPoSpusteni * (POCIATOCNY_INTERVAL - NAJKRATSI_INTERVAL) /
          CAS_ZRYCHLOVANIA);
}

void otvorNastavenieTeploty() {

  nastavenieTeplotyAktivne = true;
  casPoslednejAktivityNastavenia = millis();
  vykresliNastavenieBazenaLCD();
}

void ulozMaxBazenDoEEPROM() {

  EEPROM.put(EEPROM_ADRESA_MAX_BAZEN, MAX_BAZEN);
}

void ulozAUkonciNastavenieTeploty() {

  ulozMaxBazenDoEEPROM();
  nastavenieTeplotyAktivne = false;
  casPoslednejObrazovkyHMI = millis();

  Serial.print("MAX BAZEN EEPROM ULOZENA: ");
  Serial.print(MAX_BAZEN, 1);
  Serial.println(" C");

  HMI();
}

bool aktualizujDebounceTlacidla(int pin, bool &poslednyNacitanyStav,
                                bool &stabilnyStav, unsigned long &casZmeny,
                                unsigned long teraz) {

  bool nacitanyStav = digitalRead(pin);

  if (nacitanyStav != poslednyNacitanyStav) {
    poslednyNacitanyStav = nacitanyStav;
    casZmeny = teraz;
  }

  if (teraz - casZmeny >= TLACIDLO_DEBOUNCE_CAS &&
      nacitanyStav != stabilnyStav) {
    stabilnyStav = nacitanyStav;
    return true;
  }

  return false;
}

void obsluzTlacidloZmenyTeploty(bool stavSaZmenil, bool stabilnyStav,
                                unsigned long &casStlacenia,
                                unsigned long &casPoslednehoOpakovania,
                                int zmenaDesatin, unsigned long teraz) {

  if (stavSaZmenil) {
    if (stabilnyStav == LOW) {
      casStlacenia = teraz;
      casPoslednehoOpakovania = teraz;

      if (nastavenieTeplotyAktivne) {
        zmenMaxBazen(zmenaDesatin);
      }
    }
    else if (nastavenieTeplotyAktivne) {
      // Desatsekundova necinnost sa zacina pocitat az od uvolnenia.
      casPoslednejAktivityNastavenia = teraz;
    }
  }

  if (!nastavenieTeplotyAktivne || stabilnyStav != LOW) {
    return;
  }

  unsigned long dlzkaDrzania = teraz - casStlacenia;
  unsigned long interval = intervalOpakovaniaTlacidla(dlzkaDrzania);

  if (dlzkaDrzania >= OPAKOVANIE_START &&
      teraz - casPoslednehoOpakovania >= interval) {
    casPoslednehoOpakovania = teraz;
    zmenMaxBazen(zmenaDesatin);
  }
}

void aktualizujTlacidlaNastavenia() {

  unsigned long teraz = millis();

  if (aktualizujDebounceTlacidla(TLACIDLO_SET_PIN,
      poslednyNacitanyStavTlacidlaSet, stabilnyStavTlacidlaSet,
      casZmenyTlacidlaSet, teraz) && stabilnyStavTlacidlaSet == LOW) {

    if (nastavenieTeplotyAktivne) {
      casPoslednejAktivityNastavenia = teraz;
    }
    else {
      otvorNastavenieTeploty();
    }
  }

  bool zmenaStavuPlus = aktualizujDebounceTlacidla(TLACIDLO_PLUS_PIN,
      poslednyNacitanyStavTlacidlaPlus, stabilnyStavTlacidlaPlus,
      casZmenyTlacidlaPlus, teraz);

  bool zmenaStavuMinus = aktualizujDebounceTlacidla(TLACIDLO_MINUS_PIN,
      poslednyNacitanyStavTlacidlaMinus, stabilnyStavTlacidlaMinus,
      casZmenyTlacidlaMinus, teraz);

  obsluzTlacidloZmenyTeploty(zmenaStavuPlus, stabilnyStavTlacidlaPlus,
      casStlaceniaTlacidlaPlus, casPoslednehoOpakovaniaPlus, 1, teraz);

  obsluzTlacidloZmenyTeploty(zmenaStavuMinus, stabilnyStavTlacidlaMinus,
      casStlaceniaTlacidlaMinus, casPoslednehoOpakovaniaMinus, -1, teraz);

  bool plusJeUvolnene = stabilnyStavTlacidlaPlus == HIGH &&
                        poslednyNacitanyStavTlacidlaPlus == HIGH;
  bool minusJeUvolnene = stabilnyStavTlacidlaMinus == HIGH &&
                         poslednyNacitanyStavTlacidlaMinus == HIGH;

  if (nastavenieTeplotyAktivne && plusJeUvolnene && minusJeUvolnene &&
      teraz - casPoslednejAktivityNastavenia >= NASTAVENIE_TEPLOTY_TIMEOUT) {
    ulozAUkonciNastavenieTeploty();
  }
}

void prepnIManualnuFiltraciu() {

  if (manualFiltracia6h) {
    manualFiltracia6h = false;
    Serial.println("FIL MANUAL 6H: OFF");
  }
  else {
    manualFiltracia6h = true;
    casZaciatkuManualnejFiltracie = millis();
    Serial.println("FIL MANUAL 6H: ON");
  }
}

void prepnIManualnyChrlic() {

  if (chrlicManualAktivny) {
    chrlicManualAktivny = false;
    Serial.println("CHRLIC MANUAL 1H: OFF");
  }
  else {
    chrlicManualAktivny = true;
    casZaciatkuManualnehoChrlica = millis();
    Serial.println("CHRLIC MANUAL 1H: ON");
  }
}

void inicializaciaTlacidiel() {

  nacitajMaxBazenZEEPROM();

  pinMode(TLACIDLO_SET_PIN, INPUT_PULLUP);
  pinMode(TLACIDLO_PLUS_PIN, INPUT_PULLUP);
  pinMode(TLACIDLO_MINUS_PIN, INPUT_PULLUP);

  pinMode(TLACIDLO_FILTRACIA_6H_PIN, INPUT_PULLUP);
  pinMode(TLACIDLO_CHRLIC_1H_PIN, INPUT_PULLUP);

  poslednyNacitanyStavTlacidlaFiltracie = digitalRead(TLACIDLO_FILTRACIA_6H_PIN);
  stabilnyStavTlacidlaFiltracie = poslednyNacitanyStavTlacidlaFiltracie;

  poslednyNacitanyStavTlacidlaChrlica = digitalRead(TLACIDLO_CHRLIC_1H_PIN);
  stabilnyStavTlacidlaChrlica = poslednyNacitanyStavTlacidlaChrlica;

  poslednyNacitanyStavTlacidlaSet = digitalRead(TLACIDLO_SET_PIN);
  stabilnyStavTlacidlaSet = poslednyNacitanyStavTlacidlaSet;

  poslednyNacitanyStavTlacidlaPlus = digitalRead(TLACIDLO_PLUS_PIN);
  stabilnyStavTlacidlaPlus = poslednyNacitanyStavTlacidlaPlus;

  poslednyNacitanyStavTlacidlaMinus = digitalRead(TLACIDLO_MINUS_PIN);
  stabilnyStavTlacidlaMinus = poslednyNacitanyStavTlacidlaMinus;
}

void aktualizujTlacidla() {

  unsigned long teraz = millis();

  aktualizujTlacidlaNastavenia();

  bool nacitanyStavFiltracie = digitalRead(TLACIDLO_FILTRACIA_6H_PIN);
  if (nacitanyStavFiltracie != poslednyNacitanyStavTlacidlaFiltracie) {
    poslednyNacitanyStavTlacidlaFiltracie = nacitanyStavFiltracie;
    casZmenyTlacidlaFiltracie = teraz;
  }

  if (teraz - casZmenyTlacidlaFiltracie >= TLACIDLO_DEBOUNCE_CAS &&
      nacitanyStavFiltracie != stabilnyStavTlacidlaFiltracie) {

    stabilnyStavTlacidlaFiltracie = nacitanyStavFiltracie;
    if (stabilnyStavTlacidlaFiltracie == LOW) {
      prepnIManualnuFiltraciu();
    }
  }

  bool nacitanyStavChrlica = digitalRead(TLACIDLO_CHRLIC_1H_PIN);
  if (nacitanyStavChrlica != poslednyNacitanyStavTlacidlaChrlica) {
    poslednyNacitanyStavTlacidlaChrlica = nacitanyStavChrlica;
    casZmenyTlacidlaChrlica = teraz;
  }

  if (teraz - casZmenyTlacidlaChrlica >= TLACIDLO_DEBOUNCE_CAS &&
      nacitanyStavChrlica != stabilnyStavTlacidlaChrlica) {

    stabilnyStavTlacidlaChrlica = nacitanyStavChrlica;
    if (stabilnyStavTlacidlaChrlica == LOW) {
      prepnIManualnyChrlic();
    }
  }

  if (manualFiltracia6h &&
      teraz - casZaciatkuManualnejFiltracie >= FILTRACIA_MANUAL_6H_CAS) {

    manualFiltracia6h = false;
    Serial.println("FIL MANUAL 6H: CAS VYPRSAL");
  }

  if (chrlicManualAktivny &&
      teraz - casZaciatkuManualnehoChrlica >= CHRLIC_MANUAL_1H_CAS) {

    chrlicManualAktivny = false;
    Serial.println("CHRLIC MANUAL 1H: CAS VYPRSAL");
  }
}
