// ==================================================
// MEGA <-> UNO CROSS-RESET - PREPARED / DISABLED
// ==================================================
// Samostatny pulzny heartbeat generovany iba z hlavneho loopu.
// Nezavisi od UART, D31/D9 agreement, SystemMode, XKC ani TOTAL STOP.
// Piny su zamerne nepridelene. Defaultny build cely blok odstrani.

#if CROSS_RESET_ENABLED

#if !defined(CROSS_RESET_HEARTBEAT_OUT_PIN) || \
    !defined(CROSS_RESET_HEARTBEAT_IN_PIN) || \
    !defined(CROSS_RESET_PEER_RESET_OUT_PIN)
#error "Assign cross-reset heartbeat/reset pins before enabling."
#endif

const unsigned long HEARTBEAT_TOGGLE_MS = 500UL;
const unsigned long HEARTBEAT_TIMEOUT_MS = 5000UL;
const unsigned long STARTUP_GRACE_MS = 15000UL;
const unsigned long RESET_PULSE_MS = 200UL;
const unsigned long POST_RESET_GRACE_MS = 15000UL;
const byte CROSS_RESET_REQUIRED_EDGES = 2;

// Pripravene diagnosticke stavy. Nikde sa periodicky nevypisuju.
bool PEER_HEARTBEAT_SEEN = false;
bool PEER_HEARTBEAT_FRESH = false;
bool CROSS_RESET_ARMED = false;
bool CROSS_RESET_PULSE = false;
bool CROSS_RESET_ATTEMPTED = false;
bool CROSS_RESET_RECOVERY = false;

bool crossResetHeartbeatOutLevel = false;
bool crossResetPeerLevel = false;
bool crossResetPeerLevelInicializovany = false;
bool crossResetPostGraceAktivna = false;
byte crossResetObservedEdges = 0;
byte crossResetRecoveryEdges = 0;
unsigned long crossResetBootOdMs = 0;
unsigned long crossResetHeartbeatToggleOdMs = 0;
unsigned long crossResetLastHeartbeatEdgeMs = 0;
unsigned long crossResetPulseOdMs = 0;
unsigned long crossResetPostGraceOdMs = 0;

void inicializujCrossReset() {
  // BC547 reset vystup musi byt LOW este pred prechodom na OUTPUT.
  digitalWrite(CROSS_RESET_PEER_RESET_OUT_PIN, LOW);
  pinMode(CROSS_RESET_PEER_RESET_OUT_PIN, OUTPUT);
  digitalWrite(CROSS_RESET_PEER_RESET_OUT_PIN, LOW);

  // Heartbeat zacina deterministicky LOW a meni sa iba z main loopu.
  digitalWrite(CROSS_RESET_HEARTBEAT_OUT_PIN, LOW);
  pinMode(CROSS_RESET_HEARTBEAT_OUT_PIN, OUTPUT);
  digitalWrite(CROSS_RESET_HEARTBEAT_OUT_PIN, LOW);
  pinMode(CROSS_RESET_HEARTBEAT_IN_PIN, INPUT_PULLUP);

  const unsigned long teraz = millis();
  crossResetBootOdMs = teraz;
  crossResetHeartbeatToggleOdMs = teraz;
  crossResetPeerLevel = digitalRead(CROSS_RESET_HEARTBEAT_IN_PIN);
  crossResetPeerLevelInicializovany = true;
}

void aktualizujCrossResetHeartbeat(unsigned long teraz) {
  if (teraz - crossResetHeartbeatToggleOdMs < HEARTBEAT_TOGGLE_MS) return;

  crossResetHeartbeatToggleOdMs = teraz;
  crossResetHeartbeatOutLevel = !crossResetHeartbeatOutLevel;
  digitalWrite(CROSS_RESET_HEARTBEAT_OUT_PIN,
               crossResetHeartbeatOutLevel ? HIGH : LOW);
}

void zaznamenajCrossResetPeerEdge(unsigned long teraz) {
  const bool novaUroven = digitalRead(CROSS_RESET_HEARTBEAT_IN_PIN);

  if (!crossResetPeerLevelInicializovany) {
    crossResetPeerLevel = novaUroven;
    crossResetPeerLevelInicializovany = true;
    return;
  }

  if (novaUroven == crossResetPeerLevel) return;

  crossResetPeerLevel = novaUroven;
  PEER_HEARTBEAT_SEEN = true;
  crossResetLastHeartbeatEdgeMs = teraz;

  if (CROSS_RESET_ATTEMPTED && !CROSS_RESET_PULSE) {
    if (crossResetRecoveryEdges < CROSS_RESET_REQUIRED_EDGES)
      crossResetRecoveryEdges++;

    if (crossResetRecoveryEdges >= CROSS_RESET_REQUIRED_EDGES) {
      CROSS_RESET_ATTEMPTED = false;
      CROSS_RESET_RECOVERY = true;
      crossResetObservedEdges = CROSS_RESET_REQUIRED_EDGES;
    }
    return;
  }

  if (!CROSS_RESET_ATTEMPTED &&
      crossResetObservedEdges < CROSS_RESET_REQUIRED_EDGES)
    crossResetObservedEdges++;
}

void spustiCrossResetPulse(unsigned long teraz) {
  // Maximalne jeden pokus pre suvislu stratu heartbeat.
  CROSS_RESET_ARMED = false;
  CROSS_RESET_PULSE = true;
  CROSS_RESET_ATTEMPTED = true;
  CROSS_RESET_RECOVERY = false;
  crossResetRecoveryEdges = 0;
  crossResetPulseOdMs = teraz;
  digitalWrite(CROSS_RESET_PEER_RESET_OUT_PIN, HIGH);
}

void aktualizujCrossResetPulse(unsigned long teraz) {
  if (!CROSS_RESET_PULSE ||
      teraz - crossResetPulseOdMs < RESET_PULSE_MS) return;

  digitalWrite(CROSS_RESET_PEER_RESET_OUT_PIN, LOW);
  CROSS_RESET_PULSE = false;
  crossResetPostGraceAktivna = true;
  crossResetPostGraceOdMs = teraz;
  crossResetRecoveryEdges = 0;
}

void aktualizujCrossReset() {
  const unsigned long teraz = millis();

  aktualizujCrossResetHeartbeat(teraz);
  zaznamenajCrossResetPeerEdge(teraz);
  aktualizujCrossResetPulse(teraz);

  PEER_HEARTBEAT_FRESH = PEER_HEARTBEAT_SEEN &&
                        teraz - crossResetLastHeartbeatEdgeMs <
                            HEARTBEAT_TIMEOUT_MS;

  if (crossResetPostGraceAktivna &&
      teraz - crossResetPostGraceOdMs >= POST_RESET_GRACE_MS)
    crossResetPostGraceAktivna = false;

  const bool startupGraceHotova =
      teraz - crossResetBootOdMs >= STARTUP_GRACE_MS;

  if (!CROSS_RESET_ARMED &&
      !CROSS_RESET_ATTEMPTED &&
      !CROSS_RESET_PULSE &&
      !crossResetPostGraceAktivna &&
      startupGraceHotova &&
      PEER_HEARTBEAT_FRESH &&
      crossResetObservedEdges >= CROSS_RESET_REQUIRED_EDGES) {
    CROSS_RESET_ARMED = true;
  }

  if (CROSS_RESET_ARMED &&
      PEER_HEARTBEAT_SEEN &&
      teraz - crossResetLastHeartbeatEdgeMs >= HEARTBEAT_TIMEOUT_MS) {
    spustiCrossResetPulse(teraz);
  }
}

#endif  // CROSS_RESET_ENABLED
