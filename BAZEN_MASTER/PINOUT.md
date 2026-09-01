# BAZEN MASTER – pinout

Aktualizované: 2026-08-26

FYZICKY OVERENÉ znamená potvrdený fyzický test. POUŽITÉ V KÓDE nepotvrdzuje montáž. REZERVOVANÉ/NAVRHOVANÉ nikdy neznamená hotové zapojenie.

## Mega 2560

Použitá doska je kombinovaná Arduino Mega + WiFi Techfun IOT382: ATmega2560, samostatný ESP8266, CH340G a micro-USB. Podrobná referenčná tabuľka DIP režimov a úroveň ich overenia sú v `MASTER_KONCEPT.md`, sekcia **MEGA+WIFI – DIP PREPÍNAČE A UART REŽIMY**.

### FYZICKY OVERENÉ

| Pin | Funkcia | Výsledok |
|---:|---|---|
| D38 | MEGA HY-SRF05 TRIG | fyzicky funkčne overené; izolovaný test meral približne 19,3–19,8 cm |
| D39 | MEGA HY-SRF05 ECHO | fyzicky funkčne overené ako obyčajný `INPUT`, bez interného pull-upu |
| D20/D21 | AHT10 I²C | fyzicky namontované, funkčné a prevádzkovo overené meranie vonkajšej teploty a RH; zdieľaná I²C zbernica |
| D31 | H/L relé modul #2 | FYZICKY OVERENÉ RIADENIE; 5 V, spoločná systémová GND/DC−; `HIGH → aktívne/COM–NO`, `LOW → neaktívne/COM–NC`; strata napájania → COM–NC |
| D32 | MEGA_TOTAL_STOP | FYZICKY PRIPOJENÉ A COMMISSIONING TESTOM OVERENÉ; `HIGH → relé zopnuté/COM–NO`, `LOW → relé uvoľnené/COM–NC`; napájanie modulu z Mega power domain |
| D33 | W1209_FACKOVAC | FYZICKY PRIPOJENÉ A COMMISSIONING TESTOM OVERENÉ; `HIGH → relé zopnuté/COM–NO`, `LOW → relé uvoľnené/COM–NC`; napájanie modulu z Mega power domain |

Ostatné jednotlivé Mega piny neboli v dodaných poznámkach výslovne označené fyzickým testom.

### POUŽITÉ V KÓDE

| Pin | Smer | Funkcia | Poznámka |
|---:|---|---|---|
| D0/D1 | UART | USB Serial0 diagnostika | 115200 Bd |
| D2 | I/O | 1-Wire MEGA_T1–T4 + MEGA_TBOX | päť pevných ROM; MEGA_TBOX monitor-only |
| D14/D15 | UART | Serial3 Mega↔onboard ESP | 115200 baud |
| D16/D17 | UART | Serial2 Mega↔Uno – priame TTL | Mega D16/TX2 → sériový 10 kΩ → Uno D7/RX; Uno D8/TX → sériový 10 kΩ → Mega D17/RX2; 38400 Bd, MASTER→REPLY, V4 24 B/22 B; spoločná GND, bez PC817, bez prepojenia +5 V medzi doskami; Uno SoftwareSerial neinvertovaný |
| D20/D21 | I²C | DS3231, AHT10, LCD 20×4 | SDA/SCL; LCD používa 4-stranovú rotáciu 5 s, poruchová stránka pri aktívnom existujúcom probléme 30 s |
| D22 | OUT | MEGA_R9 | filtrácia, aktívne LOW |
| D23 | OUT | MEGA_R10 | solár/chrlič, aktívne LOW |
| D24–D29 | deklarované | MEGA_R11–R16 | bez implementovanej funkcie |
| D30 | — | voľné | pôvodný vodič k fyzickému reléovému kanálu 1 odstránený; bez nového pridelenia |
| D31 | OUT | MEGA_AGREEMENT – H/L relé modul #2 | boot/reset LOW; platná linka + čerstvé dáta + nekritický stav Una počas 180 s → HIGH; link/stale timeout 10 s alebo kritický stav → okamžite LOW a timer od nuly; nie finálna BASIC/safety logika |
| D32 | OUT | MEGA_TOTAL_STOP | produkčný boot/reset stav `LOW / COM–NC`; fyzicky potvrdené správne ovládanie Mega TOTAL STOP modulu; automatické fault podmienky zatiaľ NEIMPLEMENTOVANÉ |
| D33 | OUT | W1209_FACKOVAC (`W1209_SUPERVISION_RELAY_PIN`) | produkčný boot/reset stav `LOW / COM–NC`; fyzicky potvrdené správne ovládanie W1209 supervision modulu; autorita výhradne v budúcom explicitnom `SYSTEM_MODE == BASIC`; automatický power-cycle/fault stavový automat zatiaľ NEIMPLEMENTOVANÝ |
| D34–D37 | deklarované | MEGA_R5–R8 | časť Mega/SMART, bez implementovanej funkcie |
| D38 | OUT | MEGA HY-SRF05 TRIG | monitor-only, neblokujúci stavový automat |
| D39 | IN | MEGA HY-SRF05 ECHO | monitor-only, obyčajný `INPUT` bez interného pull-upu, bez `pulseIn()` |
| D40–D43 | — | voľná rezerva | nepridelené |
| D44 | OUT | aktuálny heartbeat | externý watchdog |
| D45 | IN_PULLUP | SET | tlačidlo |
| D46 | IN_PULLUP | PLUS | tlačidlo |
| D47 | IN_PULLUP | MÍNUS | tlačidlo |
| D48 | IN_PULLUP | FIL 6H | tlačidlo |
| D49 | IN_PULLUP | CHR 1H | tlačidlo |

### Fyzické vlastníctvo jednej 16-relé dosky

| Fyzický kanál | Dokumentačný názov | Riadiaca vrstva | Aktuálna väzba/pin |
|---:|---|---|---|
| 1 | BASIC_R1 | Uno/BASIC/watchdog – filtrácia | fyzicky vložené do FIL povoľovacej cesty; konkrétny Uno pin a detailný ovládací stupeň `NEURČENÉ`; Mega D30 zrušené |
| 2 | BASIC_R2 | Uno/BASIC/watchdog – solár | fyzicky vložené do SOLAR povoľovacej cesty; konkrétny Uno pin a detailný ovládací stupeň `NEURČENÉ`; Mega D31 zrušené |
| 3 | BASIC_R3 | Uno/BASIC/watchdog – W1209 12 V povoľovacia cesta | kontakt `COM–NC` fyzicky zapojený medzi 12 V buck a W1209 supervision modul; Uno/BASIC pin `NEURČENÉ`; pôvodná väzba kanála na Mega D32 zrušená, D32 je dnes MEGA_TOTAL_STOP |
| 4 | BASIC_R4 | Uno/BASIC/watchdog – 12 V | rezervované; fyzicky ďalej nezapojené; Uno pin `NEURČENÉ`; pôvodná väzba kanála na Mega D33 zrušená, D33 je dnes W1209_FACKOVAC |
| 5 | MEGA_R5 | Mega/SMART | D34, iba deklarované |
| 6 | MEGA_R6 | Mega/SMART | D35, iba deklarované |
| 7 | MEGA_R7 | Mega/SMART | D36, iba deklarované |
| 8 | MEGA_R8 | Mega/SMART | D37, iba deklarované |
| 9 | MEGA_R9 | Mega/SMART | D22, aktívne: filtrácia |
| 10 | MEGA_R10 | Mega/SMART | D23, aktívne: solár/chrlič |
| 11 | MEGA_R11 | Mega/SMART | D24, iba deklarované |
| 12 | MEGA_R12 | Mega/SMART | D25, iba deklarované |
| 13 | MEGA_R13 | Mega/SMART | D26, iba deklarované |
| 14 | MEGA_R14 | Mega/SMART | D27, iba deklarované |
| 15 | MEGA_R15 | Mega/SMART | D28, iba deklarované |
| 16 | MEGA_R16 | Mega/SMART | D29, iba deklarované |

Fyzicky potvrdené: prepojenia Mega → kanály 1–4 boli odstránené a Mega ich už neovláda. BASIC_R1 a BASIC_R2 sú fyzicky vložené do motorových FIL/SOLAR ciest cez BASIC/watchdog architektúru. BASIC_R3 je kontaktom `COM–NC` fyzicky zapojený v 12 V napájacej ceste W1209; BASIC_R4 fyzicky ďalej nepokračuje a zostáva rezervovaný. Konkrétne Uno/BASIC piny a autonómna logika zostávajú neurčené. Kanály 5–16 zostávajú Mega/SMART; ich známy residual backfeed pri mŕtvej Mega je akceptovaný podľa MASTER_KONCEPT.md.

### REZERVOVANÉ/NAVRHOVANÉ

| Funkcia | Pin | Stav |
|---|---|---|
| XKC-Y25 vstup Mega | NEURČENÉ | PLÁNOVANÉ |
| Lux senzor (skladovaný BH1750 je kandidát) | NEURČENÉ | PLÁNOVANÉ; pin/zbernica, adresa a prahy nepridelené |
| SMART/BASIC H/L relé Mega | D31 | FYZICKY PRIPOJENÉ A RIADENÉ HLAVNÝM PROGRAMOM; `OUTPUT LOW` pri boote, HIGH až po 180 s stability, timeout 10 s → okamžite LOW; finálna BASIC/safety vrstva a použitie kontaktov ešte NEIMPLEMENTOVANÉ |
| FIL_BLOCK relé Mega | NEURČENÉ | PLÁNOVANÉ |
| SOLAR_BLOCK relé Mega | NEURČENÉ | PLÁNOVANÉ |
| heartbeat Uno→Mega | NEURČENÉ | PLÁNOVANÉ |
| MicroSD | NEURČENÉ | PLÁNOVANÉ |

### UART cesty kombinovanej dosky

| Cesta | Mega UART | Piny ATmega2560 | Stav |
|---|---|---|---|
| USB/CH340 ↔ Mega | Serial0 | D0/RX0, D1/TX0 | POUŽITÉ V KÓDE; presná DIP poloha na našej IOT382 fyzicky `NEOVERENÁ` |
| Mega ↔ onboard ESP8266 | Serial3 | D14/TX3, D15/RX3 | POUŽITÉ V KÓDE pri 115200 Bd; presná DIP poloha a UART selector fyzicky `NEOVERENÉ` |
| Súčasne USB Mega + Mega ↔ ESP | Serial0 + Serial3 | D0/D1 + D14/D15 | REFERENČNE podporované pri DIP 1–4 ON a 5–8 OFF; na našom kuse `NEOVERENÉ` |

Referenčná rodina dosiek uvádza pre internú linku selector RXD3/TXD3. Nejde o nové pridelenie pinov: aktuálny Mega projekt už používa Serial3. Presné označenie/poloha selectoru a elektrická identita revízie Techfun IOT382 sa musia potvrdiť fyzicky.

### Pevné ROM priradenie Mega 1-Wire D2

| Názov | ROM adresa | Funkcia/stav |
|---|---|---|
| MEGA_T1 | `28 3A CF 78 91 25 06 A5` | bazén |
| MEGA_T2 | `28 E6 74 05 91 25 06 16` | solárny výstup |
| MEGA_T3 | `28 07 79 7E 91 25 06 D8` | solárny panel |
| MEGA_T4 | `28 FE FA 7D 91 25 06 31` | dno bazéna |
| MEGA_TBOX | `28 B1 31 05 00 03 24 F1` | vnútorná teplota rozvádzača; FYZICKY NAINŠTALOVANÉ, monitor-only |

### Servisná mapa fyzických odbočiek Mega 1-Wire D2

| Odbočka | Senzor | Stav |
|---|---|---|
| H1 | MEGA_T1 | fyzická servisná identifikácia |
| H2 | MEGA_T2 | fyzická servisná identifikácia |
| G2 | MEGA_T3 | FYZICKY ZNOVU POTVRDENÉ 22. 8. 2026: po odpojení iba MEGA_T3 prešla na `-127 °C`, MEGA_T1/T2/T4 zostali platné |
| H3 | MEGA_T4 | fyzická servisná identifikácia |

Všetky štyri procesné senzory sú na spoločnej Mega 1-Wire zbernici D2. Označenie fyzickej odbočky MEGA_TBOX nie je potvrdené a preto sa tu neuvádza.

MEGA_TBOX má samostatnú platnosť a diagnostiku. Nie je súčasťou regulácie, alarmu ani safety rozhodovania.

## Arduino Uno

### FYZICKY OVERENÉ

Napájanie Una je fyzicky vedené zo samostatného LM2596 nastaveného na 7,5 V. Uno nie je napájané z veľkého CC/CV bucku #2; ten poskytuje 5 V pre 16R relé/BASIC vetvu. Uno LM2596 je samostatný od watchdog vetvy LM2596HVS aj od nového 12 V LM2596 pre W1209.

| Pin | Funkcia | Výsledok |
|---:|---|---|
| D2 | 1-Wire UNO_T1/UNO_T2/UNO_T3/UNO_TBOX | všetky štyri pevné ROM aj merania fyzicky potvrdené `OK`; jeden spoločný fyzicky osadený pull-up 4,7 kΩ; napätie hornej strany pull-upu NEOVERENÉ |
| D3 | HY-SRF05 TRIG | testované |
| D4 | HY-SRF05 ECHO | testované |
| D7 | Mega D16/TX2 → sériový 10 kΩ → Uno RX | priame TTL, neinvertovaný SoftwareSerial, 38400 Bd; V4 MASTER rámec 24 B; spoločná GND; bez PC817 a bez spoločného +5 V |
| D8 | Uno TX → sériový 10 kΩ → Mega D17/RX2 | priame TTL, neinvertovaný SoftwareSerial, 38400 Bd; V4 REPLY rámec 22 B s UNO_T3; spoločná GND; bez PC817 a bez spoločného +5 V |
| D9 | H/L relé modul #1 | FYZICKY OVERENÉ RIADENIE; 5 V, spoločná systémová GND/DC−; `HIGH → aktívne/COM–NO`, `LOW → neaktívne/COM–NC`; strata napájania → COM–NC |
| A0 | UNO_TOTAL_STOP | FYZICKY PRIPOJENÉ A COMMISSIONING TESTOM OVERENÉ; `HIGH → relé zopnuté/COM–NO`, `LOW → relé uvoľnené/COM–NC`; napájanie modulu z Uno power domain |
| D10 | MicroSD CS | testované, `UNO_LOG.CSV` |
| D11 | MicroSD MOSI | testované |
| D12 | MicroSD MISO | testované |
| D13 | MicroSD SCK | testované |

Fyzická/cieľová identita ROM po checkpointe:

- UNO_T1: 28 70 A6 08 91 25 06 55
- UNO_T2, solárny výstup vody: 28 F8 23 5B 00 00 00 37 – nový dlhokáblový DS18B20
- UNO_T3, solárny panel: 28 29 81 5E 00 00 00 F5 – pôvodný krátkokáblový senzor doteraz vedený v kóde ako UNO_T2
- UNO_TBOX: 28 16 2E 09 00 03 24 29

Lokálna softvérová migrácia je implementovaná a fyzicky potvrdená: meranie, validity, recovery, kompaktný Serial DIAG a SD rozlišujú UNO_T2 a UNO_T3. Commissioning scanner našiel všetky štyri pevné ROM a po úspešnom teste bol zo sketchu odstránený; pri fyzickom V4 boote 22. 8. 2026 sa už nespustil. Prenos UNO_T3 v 22 B Uno→Mega V4 rámci a nezmenený 24 B Mega→Uno rámec sú fyzicky overené.

### POUŽITÉ V KÓDE

| Pin | Smer | Funkcia | Poznámka |
|---:|---|---|---|
| D0/D1 | UART | USB hardvérový Serial diagnostika | 115200 Bd |
| D2 | I/O | UNO_T1/UNO_T2/UNO_T3/UNO_TBOX | štyri pevné ROM fyzicky potvrdené `OK`; asynchrónne každých 5 300 ms; samostatné validity/chyby; OneWire recovery 5 000 ms zahŕňa všetky štyri senzory |
| D3 | OUT | Uno HY-SRF05 TRIG | neblokujúci stavový automat |
| D4 | IN | Uno HY-SRF05 ECHO | bez pulseIn |
| D5 | — | voľné | bývalá ESP-01S linka fyzicky odstránená |
| D6 | — | voľné | bývalá ESP-01S linka fyzicky odstránená |
| D7 | IN | Uno RX z Mega D16/TX2 cez sériový 10 kΩ | jediná SoftwareSerial linka, neinvertovaná logika, 38400 Bd; binárny výsledkový rámec V4 24 B |
| D8 | OUT | Uno TX cez sériový 10 kΩ do Mega D17/RX2 | neinvertovaný TTL UART 38400 Bd; binárny lokálny rámec V4 22 B s UNO_T3 |
| D9 | OUT | UNO_AGREEMENT – H/L relé modul #1 | boot/reset LOW; platná linka + čerstvé dáta + nekritický stav Mega počas 180 s → HIGH; link/stale timeout 10 s alebo kritický stav → okamžite LOW a timer od nuly; Mega DEGRADED samo neblokuje; nie finálna BASIC/safety logika |
| D10 | OUT | Uno MicroSD CS | SPI, `UNO_LOG.CSV` každých 60 s |
| D11 | OUT | Uno MicroSD MOSI | SPI |
| D12 | IN | Uno MicroSD MISO | SPI |
| D13 | OUT | Uno MicroSD SCK | SPI |
| A0 | OUT | UNO_TOTAL_STOP | produkčný boot/reset stav `LOW / COM–NC`; automatický commissioning impulz odstránený; fyzicky potvrdené správne ovládanie Uno TOTAL STOP modulu; `HIGH` je rezervované iba pre budúci vedomý TOTAL STOP zásah, automatické fault podmienky zatiaľ NEIMPLEMENTOVANÉ |

### REZERVOVANÉ/NAVRHOVANÉ

Všetky piny ostávajú NEURČENÉ:

| Funkcia | Pin | Stav |
|---|---|---|
| XKC-Y25 vstup Uno | NEURČENÉ | iba dátová kostra |
| RESET/ACK | NEURČENÉ | iba dátová kostra |
| heartbeat Mega→Uno | NEURČENÉ | iba dátová kostra |
| heartbeat Uno→Mega | NEURČENÉ | iba dátová kostra |
| SMART/BASIC H/L relé Uno | D9 | FYZICKY PRIPOJENÉ A RIADENÉ HLAVNÝM PROGRAMOM; `OUTPUT LOW` pri boote, HIGH až po 180 s stability, timeout 10 s → okamžite LOW; finálna BASIC/safety vrstva a použitie kontaktov ešte NEIMPLEMENTOVANÉ |
| FIL_BLOCK relé Uno | NEURČENÉ | iba dátová kostra |
| SOLAR_BLOCK relé Uno | NEURČENÉ | iba dátová kostra |
| BASIC_R1 filtrácia | NEURČENÉ | fyzický reléový kanál a FIL povoľovacia cesta zapojené; konkrétny Uno pin a autonómne riadenie neurčené |
| BASIC_R2 solár | NEURČENÉ | fyzický reléový kanál a SOLAR povoľovacia cesta zapojené; konkrétny Uno pin a autonómne riadenie neurčené |
| BASIC_R3 / W1209 12 V povoľovacia cesta | NEURČENÉ | fyzický kanál 3 na 16R doske; kontakt COM–NC fyzicky zapojený medzi 12 V buck a W1209 supervision modul; budúci Uno/BASIC riadiaci pin TBD |
| BASIC_R4 12 V | NEURČENÉ | rezervované; fyzicky ďalej nezapojené |
| UNO_T2 – solárny výstup vody | D2 1-Wire | ROM `28 F8 23 5B 00 00 00 37`; IMPLEMENTOVANÉ/FYZICKY TESTOVANÉ `OK` |
| UNO_T3 – solárny panel/náhrada MEGA_T3 | D2 1-Wire | ROM `28 29 81 5E 00 00 00 F5`; lokálne aj prenos V4 do Mega FYZICKY OVERENÉ 22. 8. 2026 |

ESP-01S bol fyzicky odstránený z Uno zostavy; je funkčný a uložený SKLADOM, budúce použitie `NEURČENÉ`. D5/D6 sú voľné. Bývalá samostatná 3,30 V ESP vetva už nie je aktívnou súčasťou Uno zostavy a jej ďalšie použitie je `NEURČENÉ`. Mega↔Uno D7/D8 je jediná SoftwareSerial linka Una a môže trvalo prijímať bez prepínania posluchu. Roly UNO_T1 bazén, UNO_T2 solárny výstup vody a UNO_T3 solárny panel sú lokálne implementované a fyzicky potvrdené; prenos UNO_T3 cez V4 bol fyzicky overený 22. 8. 2026.

UNO 1-Wire zbernica na D2 používa presne jeden spoločný fyzicky osadený pull-up rezistor 4,7 kΩ pre UNO_T1, UNO_T2, UNO_T3 a UNO_TBOX. V tomto zapojení boli všetky štyri DS18B20 úspešne fyzicky testované. Pri presune do rozvádzača sa existujúci rezistor zachová a nesmie sa pridať ďalší paralelný pull-up. Napätie, na ktoré je horná strana 4,7 kΩ pripojená, nebolo potvrdené a zostáva `NEOVERENÉ`.

## Onboard ESP8266

### FYZICKY OVERENÉ

Samostatné GPIO neboli výslovne potvrdené fyzickým testom.

### POUŽITÉ V KÓDE

| Rozhranie | Funkcia | Poznámka |
|---|---|---|
| UART Serial | ESP↔Mega Serial3 | 115200 baud |
| Wi-Fi STA/AP | web HMI a konfigurácia | AP pri zlyhaní STA |

ESP priamo neovláda relé.

### REZERVOVANÉ/NAVRHOVANÉ

Žiadne ďalšie GPIO onboard ESP nie sú rezervované.

## Systémové signály bez pinov

| Signál | Zdroj/cieľ | Elektrická logika |
|---|---|---|
| XKC LOW WATER | jeden XKC → pevný MODE → dve optočlenové cesty → Mega a Uno | jeden spoločný sensing element; úrovne, polarita, vstupný prúd, GND jumpery a MCU piny sa musia overiť |
| SMART agreement Mega | Mega → jeho povoľovacie relé | aktívne držanie povoľuje SMART |
| SMART agreement Uno | Uno → jeho povoľovacie relé | aktívne držanie povoľuje SMART |
| FIL_BLOCK Mega/Uno | každý MCU → vlastné sériové relé | neaktívna cievka povoľuje, aktívna blokuje |
| SOLAR_BLOCK Mega/Uno | každý MCU → vlastné sériové relé | neaktívna cievka povoľuje, aktívna blokuje |
| MEGA_TOTAL_STOP | Mega D32 → samostatný H/L 5 V energize-to-trip modul | FYZICKY PRIPOJENÉ/COMMISSIONING PASS; napájanie z Mega power domain; `LOW → COM–NC`, `HIGH → COM–NO`. Kontakt COM–NC je fyzicky zapojený v spoločnej sériovej motorovej ceste pred WAGO rozdelením do BASIC_R1/R2; automatické fault podmienky neimplementované. |
| UNO_TOTAL_STOP | Uno A0 → samostatný H/L 5 V energize-to-trip modul | FYZICKY PRIPOJENÉ/COMMISSIONING PASS; napájanie z Uno power domain; `LOW → COM–NC`, `HIGH → COM–NO`. Kontakt COM–NC je fyzicky zapojený v spoločnej sériovej motorovej ceste pred WAGO rozdelením do BASIC_R1/R2; automatické fault podmienky neimplementované. |
| TOTAL STOP spoločná kontaktná cesta | prívod → 2× TOTAL STOP COM–NC → WAGO → BASIC_R1/BASIC_R2 | FYZICKY ZAPOJENÉ; D32 a A0 commissioning test potvrdil ovládanie správnych reléových modulov; automatická TOTAL STOP fault logika zatiaľ neimplementovaná |
| XKC LOW WATER optická cesta Mega | XKC → HY-M154/PC817 CH1 → Mega | cieľový koncept; pin, polarita, vstupná topológia a odstránenie GND jumpera TBD do fyzického overenia |
| XKC LOW WATER optická cesta Uno | XKC → HY-M154/PC817 CH2 → Uno | cieľový koncept; pin, polarita, vstupná topológia a odstránenie GND jumpera TBD do fyzického overenia |
| RESET Mega/Uno cez BC547 | zdravá doska → BC547 → RESET druhej dosky | piny, rezistory, polarita a impulz TBD; BC547 low-side/open-collector-like, nie galvanické oddelenie |
| HISTORICKÉ UART galvanické oddelenie | Mega D16/TX2 → PC817 → Uno D7/RX; Uno D8/TX → PC817 → Mega D17/RX2 | historický stav 9600 Bd/inverse; PC817 už nie sú v dátovej ceste. Aktuálny stav je priame TTL 38400 Bd cez 10 kΩ v každom smere, spoločná GND a bez spoločného +5 V. |
| W1209_FACKOVAC / W1209_RESET_SUPERVISION | Mega D33 → samostatný H/L 5 V modul v 12 V napájaní W1209 | FYZICKY PRIPOJENÉ/COMMISSIONING PASS; napájanie z Mega power domain; `LOW → COM–NC`, `HIGH → COM–NO`; kontaktná cesta `24 V → buck 12 V → BASIC_R3 COM–NC → fackovač COM–NC → W1209` fyzicky zapojená. Automatické fault podmienky, power-cycle čas a recovery zostávajú NEIMPLEMENTOVANÉ/TBD; mŕtvy Mega ponechá NC napájanie. |
| W1209 230 V kontaktná cesta | BASIC_R1 NC → COM1 → K1 → COM2 → K2 | vstup/K1→COM2/K2 FYZICKY ZAPOJENÉ; finálny výstup za K2 NEZAPOJENÝ, cieľ neurčený |
| T_FILL / DOPÚŠŤANIE_T | budúci snímač teploty prívodnej vody pred zmiešaním s bazénom | PLÁNOVANÉ; typ senzora, fyzický kus, ROM, MCU vlastník, zbernica a pin NEURČENÉ; iba diagnostika, nie safety |

Pred montážou treba potvrdiť presné NO/NC svorky, ovládacie stupne, úrovne XKC a spoločnú zem alebo izoláciu.
