// ==================================================
// 06_HMI - OVLADANIE A ZOBRAZENIE
// ==================================================

#include <LiquidCrystal_I2C.h>

const byte LCD_ADRESA = 0x27;
const byte LCD_STLPCE = 20;
const byte LCD_RIADKY = 4;
const unsigned long CAS_PREPINANIA_HMI = 5000UL;
const unsigned long CAS_PORUCHY_HMI = 30000UL;

LiquidCrystal_I2C lcd(LCD_ADRESA, LCD_STLPCE, LCD_RIADKY);

unsigned long casStartuHMI = 0;
unsigned long casPoslednejObrazovkyHMI = 0;
bool uvodHMI = true;
byte aktualnaObrazovkaHMI = 0;

extern bool manualFiltracia6h;
extern bool chrlicManualAktivny;
extern bool nastavenieTeplotyAktivne;
extern unsigned long casZaciatkuManualnejFiltracie;
extern unsigned long casZaciatkuManualnehoChrlica;

void vymazRiadokLCD(byte riadok) {

  lcd.setCursor(0, riadok);
  lcd.print("                    ");
}

void inicializaciaHMI() {

  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("BAZEN AUTOMATIKA");
  lcd.setCursor(0, 1);
  lcd.print("SYSTEM START");

  casStartuHMI = millis();
  casPoslednejObrazovkyHMI = casStartuHMI;
  uvodHMI = true;
  aktualnaObrazovkaHMI = 0;
}

void vykresliPrevadzkuLCD() {

  vymazRiadokLCD(0);
  lcd.setCursor(0, 0);
  lcd.print("T1:");
  lcd.print(t1, 1);
  lcd.print(" T4:");
  lcd.print(t4, 1);

  vymazRiadokLCD(1);
  lcd.setCursor(0, 1);
  lcd.print("T2:");
  lcd.print(t2, 1);
  lcd.print(" T3:");
  lcd.print(t3, 1);

  vymazRiadokLCD(2);
  lcd.setCursor(0, 2);

  if (!system_OK) {
    lcd.print("SYS:ERR");
  }
  else if (systemDegradovany) {
    lcd.print("SYS:DEG");
  }
  else {
    lcd.print("SYS:OK");
  }

  lcd.print(" SET:");
  lcd.print(MAX_BAZEN, 1);
  lcd.print("C");

  vymazRiadokLCD(3);
  lcd.setCursor(0, 3);
  lcd.print("SOL:");
  lcd.print(solarZapnuty ? "ON" : "OFF");
  lcd.print(" FILT:");
  lcd.print(filtraciaZapnuta ? "ON" : "OFF");
}

void vykresliNastavenieBazenaLCD() {

  vymazRiadokLCD(0);
  lcd.setCursor(0, 0);
  lcd.print("NASTAVENIE BAZENA");

  vymazRiadokLCD(1);

  vymazRiadokLCD(2);
  lcd.setCursor(0, 2);
  lcd.print("TEPLOTA: ");
  lcd.print(MAX_BAZEN, 1);
  lcd.print(" C");

  vymazRiadokLCD(3);
  lcd.setCursor(0, 3);
  lcd.print("-      +       SET");
}

void vykresliProstredieLCD() {

  vymazRiadokLCD(0);
  lcd.setCursor(0, 0);

  if (aht10Dostupny) {
    lcd.print("OUT:");
    lcd.print(teplotaVonku, 1);
    lcd.print("C RH:");
    lcd.print(vlhkostVonku, 0);
    lcd.print("%");
  }
  else {
    lcd.print("OUT: SENSOR ERR");
  }

  vymazRiadokLCD(1);
  lcd.setCursor(0, 1);
  lcd.print("BAZEN:");
  lcd.print(teplotaBazena, 1);
  lcd.print("C");

  vymazRiadokLCD(2);
  lcd.setCursor(0, 2);
  lcd.print("SOLAR T3:");
  lcd.print(t3, 1);
  lcd.print("C");

  vymazRiadokLCD(3);
  lcd.setCursor(0, 3);

  byte hodina;
  byte minuta;
  byte sekunda;
  byte den;
  byte mesiac;
  byte rok;

  if (nacitajRTC(hodina, minuta, sekunda, den, mesiac, rok)) {
    lcd.print("CAS ");
    vypisDvojciferneNaLCD(hodina);
    lcd.print(':');
    vypisDvojciferneNaLCD(minuta);
    lcd.print(' ');
    vypisDvojciferneNaLCD(den);
    lcd.print('.');
    vypisDvojciferneNaLCD(mesiac);
  }
  else {
    lcd.print("CAS RTC ERR");
  }
}

void vypisDvojciferneNaLCD(byte hodnota) {

  if (hodnota < 10) {
    lcd.print('0');
  }

  lcd.print(hodnota);
}

void vypisManualnyCasNaLCD(const char *nazov, bool aktivny,
                            unsigned long casZaciatku, unsigned long trvanie,
                            byte riadok) {

  vymazRiadokLCD(riadok);
  lcd.setCursor(0, riadok);
  lcd.print(nazov);
  lcd.print(aktivny ? " ON" : " OFF");

  if (aktivny) {
    unsigned long uplynulo = millis() - casZaciatku;
    unsigned long zostava = uplynulo >= trvanie ? 0UL : trvanie - uplynulo;
    unsigned long zostavaMinut = zostava / 60000UL;

    lcd.print("  ");
    lcd.print(zostavaMinut / 60UL);
    lcd.print(':');
    vypisDvojciferneNaLCD(zostavaMinut % 60UL);
  }
}

void vykresliManualneRezimyLCD() {

  vymazRiadokLCD(0);
  lcd.setCursor(0, 0);
  lcd.print("MANUALNE REZIMY");

  vypisManualnyCasNaLCD("FIL 6H:", manualFiltracia6h,
                         casZaciatkuManualnejFiltracie,
                         FILTRACIA_MANUAL_6H_CAS, 1);
  vypisManualnyCasNaLCD("CHR 1H:", chrlicManualAktivny,
                         casZaciatkuManualnehoChrlica,
                         CHRLIC_MANUAL_1H_CAS, 2);

  bool r10FyzickyZapnute = SOLAR_CONTROL_VALID && (solarZapnuty || chrlicManualAktivny);

  vymazRiadokLCD(3);
  lcd.setCursor(0, 3);
  lcd.print("R9:");
  lcd.print(filtraciaZapnuta ? "ON" : "OFF");
  lcd.print("     R10:");
  lcd.print(r10FyzickyZapnute ? "ON" : "OFF");
}

void vypisProblemNaLCD(byte kod) {
  switch (kod) {
    case MEGA_PROBLEM_T1: lcd.print(F("T1 CHYBA")); break;
    case MEGA_PROBLEM_T2: lcd.print(F("T2 CHYBA")); break;
    case MEGA_PROBLEM_T3: lcd.print(F("T3 CHYBA")); break;
    case MEGA_PROBLEM_T4_ADRESA: lcd.print(F("T4 ADR NENAST")); break;
    case MEGA_PROBLEM_T4: lcd.print(F("T4 CHYBA")); break;
    case MEGA_PROBLEM_TBOX: lcd.print(F("MEGA_TBOX CHYBA")); break;
    case MEGA_PROBLEM_AHT10: lcd.print(F("AHT10 CHYBA")); break;
    case MEGA_PROBLEM_RTC: lcd.print(F("RTC CHYBA")); break;
    case MEGA_PROBLEM_SONAR_TIMEOUT: lcd.print(F("SONAR TIMEOUT")); break;
    case MEGA_PROBLEM_SONAR_CHYBA: lcd.print(F("SONAR CHYBA")); break;
    case MEGA_PROBLEM_POOL_INVALID: lcd.print(F("POOL INVALID")); break;
    case MEGA_PROBLEM_T2_INVALID: lcd.print(F("T2 EFEKT INVALID")); break;
    case MEGA_PROBLEM_SOLAR_INVALID: lcd.print(F("SOLAR INVALID")); break;
    case MEGA_PROBLEM_FIL_INVALID: lcd.print(F("FIL INVALID")); break;
    case MEGA_PROBLEM_TBOX_DIAG_INVALID: lcd.print(F("TBOX DIAG INVALID")); break;
    case MEGA_PROBLEM_REMOTE_STALE: lcd.print(F("REMOTE STALE")); break;
    case MEGA_PROBLEM_UNO_LINK: lcd.print(F("UNO LINK CHYBA")); break;
    case MEGA_PROBLEM_AGREEMENT: lcd.print(F("MEGA AGR OFF")); break;
    case MEGA_PROBLEM_T1_CONFLICT: lcd.print(F("T1 CONFLICT")); break;
    case MEGA_PROBLEM_T2_CONFLICT: lcd.print(F("T2 CONFLICT")); break;
    case MEGA_PROBLEM_MEGA_T1_SUSPECT: lcd.print(F("MEGA T1 SUSPECT")); break;
    case MEGA_PROBLEM_UNO_T1_SUSPECT: lcd.print(F("UNO T1 SUSPECT")); break;
    case MEGA_PROBLEM_MEGA_T2_SUSPECT: lcd.print(F("MEGA T2 SUSPECT")); break;
    case MEGA_PROBLEM_UNO_T2_SUSPECT: lcd.print(F("UNO T2 SUSPECT")); break;
    default: lcd.print(F("ZIADNA AKTIVNA")); break;
  }
}

void vykresliPoruchyLCD() {
  vymazRiadokLCD(0);
  lcd.setCursor(0, 0);
  lcd.print(F("PORUCHY / DIAG"));

  vymazRiadokLCD(1);
  lcd.setCursor(0, 1);
  const byte pocet = pocetMegaProblemov();
  if (pocet == 0) {
    lcd.print(F("ZIADNA AKTIVNA"));
  }
  else {
    vypisProblemNaLCD(megaProblemPodlaPoradia(0));
    if (pocet > 1) {
      lcd.print(F(" +"));
      lcd.print(pocet - 1);
    }
  }

  vymazRiadokLCD(2);
  lcd.setCursor(0, 2);
  lcd.print(F("LINK:"));
  lcd.print(unoLinkStavOk ? F("OK") : F("ERR"));
  lcd.print(F(" AGR:"));
  lcd.print(megaAgreementOn ? F("ON") : F("OFF"));

  vymazRiadokLCD(3);
  lcd.setCursor(0, 3);
  lcd.print(F("SYSTEM: "));
  if (!system_OK) lcd.print(F("HAVARIA"));
  else if (systemDegradovany) lcd.print(F("DEGRADED"));
  else lcd.print(F("OK"));
}

void vykresliAktualnuObrazovkuLCD() {
  if (aktualnaObrazovkaHMI == 0) vykresliPrevadzkuLCD();
  else if (aktualnaObrazovkaHMI == 1) vykresliProstredieLCD();
  else if (aktualnaObrazovkaHMI == 2) vykresliManualneRezimyLCD();
  else vykresliPoruchyLCD();
}

void aktualizujRotaciuHMI() {
  if (nastavenieTeplotyAktivne) return;

  const unsigned long teraz = millis();
  if (uvodHMI) {
    if (teraz - casStartuHMI < 2000UL) return;
    uvodHMI = false;
    aktualnaObrazovkaHMI = 0;
    casPoslednejObrazovkyHMI = teraz;
    lcd.clear();
    vykresliAktualnuObrazovkuLCD();
    return;
  }

  const unsigned long trvanie =
    aktualnaObrazovkaHMI == 3 && megaMaAktivnyProblem()
      ? CAS_PORUCHY_HMI : CAS_PREPINANIA_HMI;
  if (teraz - casPoslednejObrazovkyHMI < trvanie) return;

  aktualnaObrazovkaHMI = (aktualnaObrazovkaHMI + 1) % 4;
  casPoslednejObrazovkyHMI = teraz;
  vykresliAktualnuObrazovkuLCD();
}

void HMI() {

  if (nastavenieTeplotyAktivne) {
    vykresliNastavenieBazenaLCD();
    return;
  }

  if (uvodHMI) return;
  vykresliAktualnuObrazovkuLCD();
}
