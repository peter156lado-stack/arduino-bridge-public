// BINARNA DIAGNOSTICKA/KOMUNIKACNA LINKA MEGA <-> UNO
// Mega Serial2: D16/TX2, D17/RX2, 38400 Bd. Link zatial riadi iba prvu
// fyzicku agreement skusku; UART nie je finalny safety heartbeat.

const unsigned long UNO_LINK_BAUD = 38400UL;
const unsigned long UNO_LINK_FRAME_INTERVAL_MS = 1000UL;
const unsigned long UNO_REPLY_WAIT_MS = 500UL;
const unsigned long UNO_LINK_TIMEOUT_MS = 10000UL;
const unsigned long UNO_REMOTE_TIMEOUT_MS = 10000UL;
const unsigned long MEGA_SMART_STABILIZACIA_MS = 180000UL;
const byte LINK_MAGIC_1 = 0xBA;
const byte LINK_MAGIC_2 = 0x5E;
const byte LINK_PROTOCOL_VERSION = 4;
const byte LINK_TYPE_UNO_TO_MEGA = 0x01;
const byte LINK_TYPE_MEGA_TO_UNO = 0x02;
const byte UNO_FRAME_SIZE = 22;
const byte MEGA_FRAME_SIZE = 24;

struct UnoRemoteSnapshot {
  float t1, t2, t3, tbox, sonar;
  bool t1Ok, t2Ok, t3Ok, tboxOk, sonarOk;
  bool unoAgreementOn;
  byte sonarStav, unoStav;
  unsigned int sekvencia;
};

UnoRemoteSnapshot unoRemote = {
  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
  false, false, false, false, false,
  false, 0, 2, 0
};

byte unoLinkRxBuffer[UNO_FRAME_SIZE];
byte unoLinkTxBuffer[MEGA_FRAME_SIZE];
byte unoLinkRxDlzka = 0;
byte unoLinkTxPozicia = MEGA_FRAME_SIZE;
unsigned int megaLinkSekvencia = 0;
unsigned long unoLinkPosledneFrameTxMs = 0;
unsigned long unoLinkPosledneRxMs = 0;
unsigned long unoRemotePoslednyRamecMs = 0;
bool unoLinkMaRamec = false;
bool unoLinkStavOk = false;
bool unoLinkStavVypisany = false;
bool unoLinkCakaNaOdpoved = false;
unsigned long unoLinkCakaOdMs = 0;
bool megaAgreementOn = false;
bool megaSmartStabilizaciaBezi = false;
unsigned long megaSmartStabilnyOdMs = 0;
bool REMOTE_DATA_VALID = false;
bool unoAgreementOnRemote = false;
unsigned long unoRxFrameOk = 0;
unsigned long unoCrcFail = 0;
unsigned long unoFrameInvalid = 0;
unsigned long unoLinkTimeoutCount = 0;
unsigned long unoSeqGapCount = 0;
unsigned long unoTxRequestCount = 0;
unsigned long unoReplyOkCount = 0;
unsigned long unoReplyTimeoutCount = 0;
unsigned int unoPoslednaSekvencia = 0;
bool unoSekvenciaPlatna = false;
unsigned long unoLinkPoslednyStatsMs = 0;

bool T1_CONFLICT = false;
bool T2_CONFLICT = false;
bool MEGA_T1_SUSPECT = false;
bool UNO_T1_SUSPECT = false;
bool MEGA_T2_SUSPECT = false;
bool UNO_T2_SUSPECT = false;
const float KRIZOVA_DIAG_TOLERANCIA_C = 2.0f;
const unsigned long KRIZOVA_DIAG_STABILIZACIA_MS = 10UL * 60UL * 1000UL;
unsigned long solarStabilnyOdMs = 0;
bool solarBolZapnutyPreDiagnostiku = false;
bool KRIZOVA_DIAG_READY = false;

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

void zapisI16(byte *data, byte pozicia, int hodnota) {
  zapisU16(data, pozicia, (unsigned int)hodnota);
}

int citajI16(const byte *data, byte pozicia) {
  return (int)citajU16(data, pozicia);
}

void zapisU32(byte *data, byte pozicia, unsigned long hodnota) {
  data[pozicia] = (byte)hodnota;
  data[pozicia + 1] = (byte)(hodnota >> 8);
  data[pozicia + 2] = (byte)(hodnota >> 16);
  data[pozicia + 3] = (byte)(hodnota >> 24);
}

int teplotaNaStotiny(float hodnota) {
  return (int)(hodnota * 100.0f + (hodnota >= 0.0f ? 0.5f : -0.5f));
}

float stotinyNaTeplotu(int hodnota) {
  return (float)hodnota / 100.0f;
}

void prijmiUnoRamec() {
  if (unoLinkRxBuffer[0] != LINK_MAGIC_1 || unoLinkRxBuffer[1] != LINK_MAGIC_2 ||
      unoLinkRxBuffer[2] != LINK_PROTOCOL_VERSION || unoLinkRxBuffer[3] != LINK_TYPE_UNO_TO_MEGA ||
      unoLinkRxBuffer[4] != UNO_FRAME_SIZE) {
    unoFrameInvalid++;
    return;
  }
  if (linkCrc8(unoLinkRxBuffer, UNO_FRAME_SIZE - 1) != unoLinkRxBuffer[UNO_FRAME_SIZE - 1]) {
    unoCrcFail++;
    return;
  }

  const byte validity = unoLinkRxBuffer[7];
  UnoRemoteSnapshot novy;
  novy.sekvencia = citajU16(unoLinkRxBuffer, 5);
  novy.t1Ok = validity & 0x01;
  novy.t2Ok = validity & 0x02;
  novy.tboxOk = validity & 0x04;
  novy.sonarOk = validity & 0x08;
  novy.t3Ok = validity & 0x10;
  novy.unoStav = unoLinkRxBuffer[8];
  novy.sonarStav = unoLinkRxBuffer[9];
  if (novy.unoStav > 2 || novy.sonarStav > 2) {
    unoFrameInvalid++;
    return;
  }
  novy.t1 = stotinyNaTeplotu(citajI16(unoLinkRxBuffer, 10));
  novy.t2 = stotinyNaTeplotu(citajI16(unoLinkRxBuffer, 12));
  novy.tbox = stotinyNaTeplotu(citajI16(unoLinkRxBuffer, 14));
  novy.sonar = (float)citajU16(unoLinkRxBuffer, 16) / 10.0f;
  novy.t3 = stotinyNaTeplotu(citajI16(unoLinkRxBuffer, 18));
  const byte remoteFlags = unoLinkRxBuffer[20];
  if ((remoteFlags & 0xFE) != 0) {
    unoFrameInvalid++;
    return;
  }
  novy.unoAgreementOn = (remoteFlags & 0x01) != 0;
  if (unoSekvenciaPlatna && novy.sekvencia != (unsigned int)(unoPoslednaSekvencia + 1U)) unoSeqGapCount++;
  unoPoslednaSekvencia = novy.sekvencia;
  unoSekvenciaPlatna = true;
  unoRxFrameOk++;
  unoReplyOkCount++;
  unoRemote = novy;
  unoRemotePoslednyRamecMs = millis();
  unoLinkPosledneRxMs = unoRemotePoslednyRamecMs;
  unoLinkMaRamec = true;
  unoLinkCakaNaOdpoved = false;
}

void prijmiUnoByte(byte hodnota) {
  if (unoLinkRxDlzka == 0 && hodnota != LINK_MAGIC_1) return;
  if (unoLinkRxDlzka == 1 && hodnota != LINK_MAGIC_2) {
    unoLinkRxDlzka = hodnota == LINK_MAGIC_1 ? 1 : 0;
    return;
  }
  unoLinkRxBuffer[unoLinkRxDlzka++] = hodnota;
  if (unoLinkRxDlzka == UNO_FRAME_SIZE) {
    prijmiUnoRamec();
    unoLinkRxDlzka = 0;
  }
}

void pripravMegaRamec() {
  megaLinkSekvencia++;
  unoLinkTxBuffer[0] = LINK_MAGIC_1;
  unoLinkTxBuffer[1] = LINK_MAGIC_2;
  unoLinkTxBuffer[2] = LINK_PROTOCOL_VERSION;
  unoLinkTxBuffer[3] = LINK_TYPE_MEGA_TO_UNO;
  unoLinkTxBuffer[4] = MEGA_FRAME_SIZE;
  zapisU16(unoLinkTxBuffer, 5, megaLinkSekvencia);
  byte validity = 0;
  if (POOL_TEMP_VALID) validity |= 0x01;
  if (T2_EFFECTIVE_VALID) validity |= 0x02;
  byte h, m, s, d, mo, y;
  const bool rtcOk = nacitajRTC(h, m, s, d, mo, y);
  if (rtcOk) validity |= 0x04;
  unoLinkTxBuffer[7] = validity;
  unoLinkTxBuffer[8] = (byte)zdrojTeplotyBazena;
  unoLinkTxBuffer[9] = (byte)zdrojT2;
  byte diagnostika = 0;
  if (T1_CONFLICT) diagnostika |= 0x01;
  if (T2_CONFLICT) diagnostika |= 0x02;
  if (MEGA_T1_SUSPECT) diagnostika |= 0x04;
  if (UNO_T1_SUSPECT) diagnostika |= 0x08;
  if (MEGA_T2_SUSPECT) diagnostika |= 0x10;
  if (UNO_T2_SUSPECT) diagnostika |= 0x20;
  unoLinkTxBuffer[10] = diagnostika;
  // Kompatibilny Mega health stav 0/1/2 zostava docasne agreement vstupom Una.
  // Nie je druhou autoritou SYSTEM_MODE.
  unoLinkTxBuffer[11] = system_OK ? (systemDegradovany ? 1 : 0) : 2;
  zapisI16(unoLinkTxBuffer, 12, POOL_TEMP_VALID ? teplotaNaStotiny(teplotaBazena) : 0);
  zapisI16(unoLinkTxBuffer, 14, T2_EFFECTIVE_VALID ? teplotaNaStotiny(teplotaSolarVystup) : 0);
  const unsigned long sekundyDna = rtcOk ? (unsigned long)h * 3600UL + (unsigned long)m * 60UL + s : 0UL;
  zapisU32(unoLinkTxBuffer, 16, sekundyDna);
  unoLinkTxBuffer[20] = rtcOk ? y : 0;
  unoLinkTxBuffer[21] = rtcOk ? mo : 0;
  unoLinkTxBuffer[22] = rtcOk ? d : 0;
  unoLinkTxBuffer[23] = linkCrc8(unoLinkTxBuffer, MEGA_FRAME_SIZE - 1);
  unoLinkTxPozicia = 0;
}

void aktualizujKrizovuDiagnostiku() {
  const unsigned long teraz = millis();
  REMOTE_DATA_VALID = unoRemotePoslednyRamecMs != 0 && teraz - unoRemotePoslednyRamecMs < UNO_REMOTE_TIMEOUT_MS;
  // Vzdialene agreement je pravdive iba spolu s cerstvym validnym ramcom.
  unoAgreementOnRemote = REMOTE_DATA_VALID && unoRemote.unoAgreementOn;
  T1_CONFLICT = REMOTE_DATA_VALID && T1_OK && unoRemote.t1Ok && fabs(t1 - unoRemote.t1) > KRIZOVA_DIAG_TOLERANCIA_C;
  T2_CONFLICT = REMOTE_DATA_VALID && T2_OK && unoRemote.t2Ok && fabs(t2 - unoRemote.t2) > KRIZOVA_DIAG_TOLERANCIA_C;
  if (solarZapnuty && !solarBolZapnutyPreDiagnostiku) solarStabilnyOdMs = teraz;
  solarBolZapnutyPreDiagnostiku = solarZapnuty;
  KRIZOVA_DIAG_READY = solarZapnuty && teraz - solarStabilnyOdMs >= KRIZOVA_DIAG_STABILIZACIA_MS;
  MEGA_T1_SUSPECT = UNO_T1_SUSPECT = MEGA_T2_SUSPECT = UNO_T2_SUSPECT = false;
  if (!KRIZOVA_DIAG_READY || !REMOTE_DATA_VALID) return;
  if (T1_OK && unoRemote.t1Ok && T2_OK && unoRemote.t2Ok && fabs(t2 - unoRemote.t2) <= KRIZOVA_DIAG_TOLERANCIA_C) {
    const float refT2 = (t2 + unoRemote.t2) * 0.5f;
    const bool megaBlizko = fabs(t1 - refT2) <= KRIZOVA_DIAG_TOLERANCIA_C;
    const bool unoBlizko = fabs(unoRemote.t1 - refT2) <= KRIZOVA_DIAG_TOLERANCIA_C;
    if (megaBlizko && !unoBlizko) UNO_T1_SUSPECT = true;
    if (!megaBlizko && unoBlizko) MEGA_T1_SUSPECT = true;
  }
  if (T1_OK && unoRemote.t1Ok && T2_OK && unoRemote.t2Ok && fabs(t1 - unoRemote.t1) <= KRIZOVA_DIAG_TOLERANCIA_C) {
    const float refT1 = (t1 + unoRemote.t1) * 0.5f;
    const bool megaBlizko = fabs(t2 - refT1) <= KRIZOVA_DIAG_TOLERANCIA_C;
    const bool unoBlizko = fabs(unoRemote.t2 - refT1) <= KRIZOVA_DIAG_TOLERANCIA_C;
    if (megaBlizko && !unoBlizko) UNO_T2_SUSPECT = true;
    if (!megaBlizko && unoBlizko) MEGA_T2_SUSPECT = true;
  }
}

void inicializaciaUnoLinkTest() {
  Serial2.begin(UNO_LINK_BAUD);
  unoLinkPosledneFrameTxMs = millis() - UNO_LINK_FRAME_INTERVAL_MS;
  Serial.println("UNO LINK: Serial2 D16/D17 @ 38400, BINARY V4 CRC8");
}

void nastavMegaAgreement(bool povolit) {
  if (megaAgreementOn == povolit) return;
  megaAgreementOn = povolit;
  digitalWrite(MEGA_HL_RELAY_2_PIN, megaAgreementOn ? HIGH : LOW);
  Serial.println(megaAgreementOn ? F("RECOVERY: MEGA_AGREEMENT=ON")
                                 : F("EVENT: MEGA_AGREEMENT=OFF"));
}

void aktualizujMegaAgreementStabilizaciu(bool podmienkyOk, unsigned long teraz) {
  if (!podmienkyOk) {
    if (megaSmartStabilizaciaBezi) {
      megaSmartStabilizaciaBezi = false;
      Serial.println(F("EVENT: MEGA_SMART_STABLE=BLOCKED"));
    }
    nastavMegaAgreement(false);
    return;
  }

  if (!megaSmartStabilizaciaBezi) {
    megaSmartStabilizaciaBezi = true;
    megaSmartStabilnyOdMs = teraz;
    Serial.println(F("RECOVERY: MEGA_SMART_STABLE=START"));
  }

  if (teraz - megaSmartStabilnyOdMs >= MEGA_SMART_STABILIZACIA_MS)
    nastavMegaAgreement(true);
}

bool megaSmartPodmienkyOk() {
  const unsigned long teraz = millis();
  const bool remoteDataCerstve = unoRemotePoslednyRamecMs != 0 &&
                                 teraz - unoRemotePoslednyRamecMs < UNO_REMOTE_TIMEOUT_MS;
  // UNO_SENSOR_DEGRADED (1) nie je dovod zrusit agreement. Hodnota 2 je
  // rezervovana pre kriticku poruchu supervisor/safety vrstvy Una.
  return unoLinkStavOk && remoteDataCerstve && unoRemote.unoStav != 2;
}

void vypisMegaSmartStable() {
  Serial.print(F("SMART_STABLE="));
  if (!megaSmartPodmienkyOk() || !megaSmartStabilizaciaBezi) {
    Serial.println(F("BLOCKED"));
    return;
  }
  if (megaAgreementOn) {
    Serial.println(F("READY"));
    return;
  }
  unsigned long sekundy = (millis() - megaSmartStabilnyOdMs) / 1000UL;
  if (sekundy > 180UL) sekundy = 180UL;
  Serial.print(sekundy);
  Serial.println(F("/180s"));
}

void aktualizujUnoLink() {
  const unsigned long teraz = millis();
  byte prijate = 0;
  while (Serial2.available() > 0 && prijate < 24) {
    prijmiUnoByte((byte)Serial2.read());
    prijate++;
  }
  if (unoLinkCakaNaOdpoved && teraz - unoLinkCakaOdMs >= UNO_REPLY_WAIT_MS) {
    unoLinkCakaNaOdpoved = false;
    unoReplyTimeoutCount++;
  }

  if (unoLinkTxPozicia < MEGA_FRAME_SIZE && Serial2.availableForWrite() > 0) {
    Serial2.write(unoLinkTxBuffer[unoLinkTxPozicia++]);
    if (unoLinkTxPozicia >= MEGA_FRAME_SIZE) {
      unoTxRequestCount++;
      unoLinkCakaNaOdpoved = true;
      unoLinkCakaOdMs = millis();
    }
  }
  else if (!unoLinkCakaNaOdpoved && unoLinkTxPozicia >= MEGA_FRAME_SIZE &&
           teraz - unoLinkPosledneFrameTxMs >= UNO_LINK_FRAME_INTERVAL_MS) {
    unoLinkPosledneFrameTxMs = teraz;
    pripravMegaRamec();
  }
  // Parser mohol prave nastavit posledny RX cez novsie millis(). Timeout preto
  // vyhodnot az z casu odobrateho po spracovani prijmu.
  const unsigned long linkTerazMs = millis();
  const bool linkTerazOk = unoLinkMaRamec && linkTerazMs - unoLinkPosledneRxMs < UNO_LINK_TIMEOUT_MS;
  if (unoLinkStavOk && !linkTerazOk) unoLinkTimeoutCount++;
  if (!unoLinkStavVypisany || linkTerazOk != unoLinkStavOk) {
    unoLinkStavOk = linkTerazOk;
    unoLinkStavVypisany = true;
    Serial.println(unoLinkStavOk ? F("RECOVERY: UNO_LINK_OK") : F("EVENT: UNO_LINK_CHYBA"));
  }
  const bool remoteDataCerstve = unoRemotePoslednyRamecMs != 0 &&
                                 linkTerazMs - unoRemotePoslednyRamecMs < UNO_REMOTE_TIMEOUT_MS;
  const bool smartPodmienkyOk = linkTerazOk && remoteDataCerstve && unoRemote.unoStav != 2;
  aktualizujMegaAgreementStabilizaciu(smartPodmienkyOk, linkTerazMs);
  if (teraz - unoLinkPoslednyStatsMs >= 10000UL) {
    unoLinkPoslednyStatsMs = teraz;
    Serial.print("UNO LINK STATS: RX_FRAME_OK=");
    Serial.print(unoRxFrameOk);
    Serial.print(" CRC_FAIL=");
    Serial.print(unoCrcFail);
    Serial.print(" FRAME_INVALID=");
    Serial.print(unoFrameInvalid);
    Serial.print(" LINK_TIMEOUT_COUNT=");
    Serial.print(unoLinkTimeoutCount);
    Serial.print(" SEQ_GAP_COUNT=");
    Serial.println(unoSeqGapCount);
    Serial.print(F("TX_REQUEST_COUNT="));
    Serial.print(unoTxRequestCount);
    Serial.print(F(" REPLY_OK_COUNT="));
    Serial.print(unoReplyOkCount);
    Serial.print(F(" REPLY_TIMEOUT_COUNT="));
    Serial.println(unoReplyTimeoutCount);
  }
  aktualizujKrizovuDiagnostiku();
}

unsigned long vekUnoRemoteDatMs() { return REMOTE_DATA_VALID ? millis() - unoRemotePoslednyRamecMs : 0UL; }
bool unoRemoteT1Platna() { return REMOTE_DATA_VALID && unoRemote.t1Ok; }
bool unoRemoteT2Platna() { return REMOTE_DATA_VALID && unoRemote.t2Ok; }
bool unoRemoteT3Platna() { return REMOTE_DATA_VALID && unoRemote.t3Ok; }
bool unoRemoteTboxPlatna() { return REMOTE_DATA_VALID && unoRemote.tboxOk; }
float unoRemoteT1Hodnota() { return unoRemote.t1; }
float unoRemoteT2Hodnota() { return unoRemote.t2; }
float unoRemoteT3Hodnota() { return unoRemote.t3; }
float unoRemoteTboxHodnota() { return unoRemote.tbox; }
float unoRemoteSonarHodnota() { return unoRemote.sonar; }
bool unoRemoteSonarPlatny() { return REMOTE_DATA_VALID && unoRemote.sonarOk; }

const char *textZdroja(byte zdroj) {
  switch (zdroj) {
    case ZDROJ_LOKALNY_PRIMARNY: return "PRIMARY";
    case ZDROJ_LOKALNY_FALLBACK: return "LOCAL_FALLBACK";
    case ZDROJ_VZDIALENY_FALLBACK: return "REMOTE_FALLBACK";
    default: return "INVALID";
  }
}
