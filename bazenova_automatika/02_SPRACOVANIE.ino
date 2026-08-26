// ==================================================
// 02_SPRACOVANIE – SPRACOVANIE NAMERANÝCH HODNÔT
// ==================================================

extern bool T1_OK;
extern bool T3_OK;
extern bool T4_OK;
extern float teplotaBazena;

void spracovanie() {

  // ------------------------------------------------
  // VÝPOČET ROZDIELOV TEPLOTY
  // ------------------------------------------------

  // ------------------------------------------------
  // VÝPIS
  // ------------------------------------------------

  if (megaDiagnostickyVypis) {
  Serial.print("Rozdiel T3-bazen: ");

  if (T3_OK && (T1_OK || T4_OK)) {

    float rozdiel_panel_bazen = t3 - teplotaBazena;
    Serial.print(rozdiel_panel_bazen, 2);
    Serial.println(" C");
  }
  else {

    Serial.println("N/A");
  }
  }

}
