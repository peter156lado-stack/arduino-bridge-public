// ==================================================
// TEST_RELE - DOCASNY HARDVEROVY TEST R9
// ==================================================
//
// Test sa spusti iba manualnym zavolanim funkcie
// testReleR9() alebo prikazom TEST cez Serial Monitor.
// R10 (pin 23) sa tu nepouziva.

String testPrikaz;
unsigned long casTestuR9 = 0;
bool testR9Aktivny = false;
bool testR9Povoleny = false;

void testReleR9() {

  if (!testR9Povoleny) {
    return;
  }

  // Relé modul je aktivny v logickej nule.
  testR9Aktivny = true;
  casTestuR9 = millis();
  aktualizujNapajanieFiltracie(false);
}

void aktualizujTestReleR9() {

  if (testR9Povoleny && testR9Aktivny && millis() - casTestuR9 >= 1000) {

    testR9Aktivny = false;
    aktualizujNapajanieFiltracie(false);
    testR9Povoleny = false;

    Serial.println("TEST R9 FILTRACIA: HOTOVO");
  }
}

// Arduino jadro tuto funkciu automaticky vola po loop().
// Prikaz TEST (s odosielanim Newline alebo Both NL & CR) spusti test R9.
void serialEvent() {

  while (Serial.available()) {

    char znak = (char)Serial.read();

    if (znak == '\n' || znak == '\r') {

      if (testPrikaz == "TEST") {
        Serial.println("TEST R9 FILTRACIA: START");
        testR9Povoleny = true;
        testReleR9();
      }

      testPrikaz = "";
    }
    else {

      testPrikaz += znak;
    }
  }
}
