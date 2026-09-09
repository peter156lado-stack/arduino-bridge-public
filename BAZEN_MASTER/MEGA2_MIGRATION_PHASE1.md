# UNO → MEGA2 MIGRATION – PHASE 1

**Dátum prípravy:** 2026-09-09  
**Stav:** SOFTWARE PREPARED / PHYSICAL MIGRATION PENDING / NOT COMMISSIONED  
**Cieľová doska:** Arduino Mega 2560 / ATmega2560

## Rozsah a autorita

Mega1 zostáva jediný `MASTER / CONTROL` a vlastník SMART regulácie. Mega2
preberá iba dnešnú implementovanú rolu Una: `SUPERVISOR / VERIFY / SAFETY /
BASIC / BLACK BOX`. Mega2 nie je druhý SMART regulátor. Platí
`COMMAND != FEEDBACK != PROCESS EFFECT` a priorita
`STOP > BASIC > DEGRADED > SMART`.

Fáza 1 nemení Mega1, Uno ani ESP produkčný kód, fyzickú safety filozofiu,
časovania, protokolový formát ani výstupy. Nový Mega2 sketch zatiaľ nebol
nahraný a nebol fyzicky testovaný.

## Audit aktuálneho Uno zdroja

Aktuálny Uno V6 sketch reálne implementuje:

- štyri DS18B20 na spoločnej D2 zbernici s pevnými ROM, explicitným 12-bit
  nastavením, neblokujúcim START/READ a recovery;
- HY-SRF05 na D3/D4 bez `pulseIn()`;
- lokálny XKC na A2 a jediného zapisovateľa TOTAL STOP A0;
- agreement na D9 s 180 s stabilizáciou a okamžitým dropom;
- SD BLACK BOX na D10–D13, 60 s záznam, event log a recovery;
- V6 MASTER→REPLY linku s Mega1, soft clock synchronizovaný z Mega1 RTC;
- defaultne vypnutú cross-reset prípravu bez pridelených pinov.

Aktuálny Uno zdroj **neimplementuje** autonómne riadenie BASIC_R1–R4,
FIL_BLOCK, SOLAR_BLOCK ani fyzický cross-reset. Nepoužitá štruktúra
`BuduceBezpecnostneStavy` nebola do nového Mega2 projektu prenesená, pretože
nemá runtime funkciu ani piny. Tým nevzniká nová BASIC logika; uvedené funkcie
zostávajú TBD a vyžadujú osobitné schválenie.

## Aktuálny V6 protokol – zachovaný

| Vlastnosť | Hodnota |
|---|---|
| Fyzická vrstva | priame neinvertované 5 V TTL, spoločná GND |
| Ochrana | sériový 10 kΩ v každom dátovom smere |
| Spoločné +5 V | NIE |
| PC817 v UART ceste | NIE |
| Baud | 38400, 8N1 |
| Magic | `BA 5E` |
| Verzia | 6 |
| Mega1→supervisor typ | `0x02`, 38 B |
| Supervisor→Mega1 typ | `0x01`, 22 B |
| Integrita | CRC-8/ATM, polynóm `0x07` |
| Poradie | Mega1 MASTER rámec → validácia → supervisor REPLY |
| Sekvencia | 16-bit v oboch smeroch |
| Perioda Mega1 requestu | približne 1 s |
| Reply window | 500 ms |
| Link/stale timeout | 10 s |
| Agreement recovery | 180 s nepretržitej stability |

Mega2 používa rovnaké bajtové rozloženie aj reserved-bit validáciu. Názvy
`UNO_FRAME_SIZE`, `LINK_TYPE_UNO_TO_MEGA`, `UNO_*` a existujúce CSV hlavičky
zostávajú vo fáze 1 zámerne zachované ako wire/log kompatibilita. Starý rámec
sa neinterpretuje ako nový.

## 1:1 pin migration map

| UNO_PIN | UNO_FUNCTION | DIRECTION | ACTIVE_LEVEL | PULLUP | VOLTAGE | SAFETY/DIAG | CURRENT_PHYSICAL_WIRE | MEGA2_PIN | MEGA2_REASON | 1_TO_1 | PHYSICAL_ACTION |
|---|---|---|---|---|---|---|---|---|---|---|---|
| D2 | 1-Wire UNO_T1/T2/T3/TBOX | I/O | open-drain bus | 1× externý 4,7 kΩ; horná vetva NEOVERENÁ | 5 V AVR logika; pull-up rail NEOVERENÝ | sensor/diag | spoločná štvorica DS18B20 | D2 | priama migrácia zachová mapu | YES | presunúť jediný D2 vodič aj existujúci jediný pull-up; nepridávať paralelný |
| D3 | sonar TRIG | OUT | krátky HIGH impulz | nie | 5 V | diag | HY-SRF05 TRIG | D3 | priama migrácia | YES | Uno D3 vodič → Mega2 shield D3 |
| D4 | sonar ECHO | IN | HIGH pulse width | nie; obyčajný INPUT | 5 V | diag | HY-SRF05 ECHO | D4 | priama migrácia | YES | Uno D4 vodič → Mega2 shield D4 |
| D7 | Mega1 TX → supervisor RX | IN | neinvertované UART | nie | 5 V TTL | comm/diag | Mega1 D16/TX2 cez 10 kΩ | D19/RX1 | Mega2 HW UART Serial1 | NO | vodič z Uno D7 presunúť na Mega2 D19; 10 kΩ ponechať |
| D8 | supervisor TX → Mega1 RX | OUT | neinvertované UART | nie | 5 V TTL | comm/diag | do Mega1 D17/RX2 cez 10 kΩ | D18/TX1 | Mega2 HW UART Serial1 | NO | vodič z Uno D8 presunúť na Mega2 D18; 10 kΩ ponechať |
| D9 | agreement H/L relé | OUT | HIGH = COM–NO, LOW = COM–NC | nie | 5 V | agreement/physical authority | H/L modul #1 | D9 | priama migrácia; boot LOW | YES | Uno D9 vodič → Mega2 shield D9; skontrolovať konektor/header |
| D10 | MicroSD CS | OUT | LOW select | nie | 5 V AVR signál; modul supply NEOVERENÉ | BLACK BOX | SD CS | D10 | CS môže zostať bez kolízie | YES | Uno D10 vodič → Mega2 shield D10 |
| D11 | MicroSD MOSI | OUT | SPI | nie | 5 V AVR signál; modul supply NEOVERENÉ | BLACK BOX | SD MOSI | D51/MOSI | Mega2560 HW SPI nie je na D11 | NO | Uno D11 vodič → Mega2 D51 |
| D12 | MicroSD MISO | IN | SPI | nie | 5 V AVR signál; modul supply NEOVERENÉ | BLACK BOX | SD MISO | D50/MISO | Mega2560 HW SPI nie je na D12 | NO | Uno D12 vodič → Mega2 D50 |
| D13 | MicroSD SCK | OUT | SPI clock | nie | 5 V AVR signál; modul supply NEOVERENÉ | BLACK BOX | SD SCK | D52/SCK | Mega2560 HW SPI nie je na D13 | NO | Uno D13 vodič → Mega2 D52 |
| A0 | supervisor TOTAL STOP | OUT | HIGH = trip/COM–NO; LOW = COM–NC | nie | 5 V | independent safety | vlastný energize-to-trip H/L modul | A0 | priama migrácia; jediný writer | YES | Uno A0 vodič → Mega2 shield A0 |
| A2 | supervisor XKC | IN_PULLUP | LOW = WATER; HIGH = LOW_WATER/open path | interný pull-up | 5 V logika za HY-M154/PC817 | independent safety | druhý samostatný XKC optočlenový kanál | A2 | priama migrácia | YES | Uno A2 vodič → Mega2 shield A2 |
| — | Mega2560 HW SS master | OUT | držať HIGH | nie | 5 V | platform/SPI | na Uno nemá osobitný vodič | D53 | nutné pre spoľahlivý Mega SPI master režim | NO | bez externého vodiča; neobsadzovať inou funkciou |

D5, D6 a A1 sú na dnešnom Uno nepoužité a nemajú migrovaný vodič. D7/D8 sa
po migrácii neužívajú; Mega2 linka je iba Serial1 D19/D18. D0/D1 zostávajú
pre USB/Serial servis a nesmú sa použiť na M1↔M2 linku.

### Súhrn mapy

- Implementovaných Uno funkcií mapovaných: **12**.
- Priame 1:1 migrácie: **7**.
- Nepriame migrácie: **5** — dva UART signály a tri SPI signály
  (MOSI, MISO, SCK); navyše D53 je nová interná platformová rezervácia bez
  vodiča.
- Mega1 zostáva `Serial2`: D16/TX2 a D17/RX2.
- Mega2 používa `Serial1`: D19/RX1 a D18/TX1.

## OneWire časovanie po prechode na HW UART

### A) Stále potrebné

- pevné ROM adresy a rolové validity;
- explicitné 12-bit nastavenie pri boote aj po recovery;
- neblokujúci START/READ s minimálne 800 ms konverzným oknom;
- recovery pri chybe a rollover-safe `millis()` rozdiely;
- posledný platný snapshot namiesto falošného priebežného výsledku.

### B) Historicky zvolené najmä kvôli SoftwareSerial

- presná hodnota 5 300 ms bola zvolená tak, aby sa nefázovala v pomere 5:1
  voči približne sekundovému requestu;
- obmedzenie príjmu na jeden 38 B rámec a posielanie reply po bajtoch bolo
  konzervatívne pre Uno SoftwareSerial.

### C) Ponechané do fyzického commissioningu Mega2

Vo fáze 1 sa 5 300 ms interval, 5 000 ms recovery, 800 ms konverzné okno aj
MASTER→REPLY obsluha nemenia. HW UART odstráni SoftwareSerial bit-sampling a
TX/RX kolíziu, ale až fyzický test pri trvalej DS chybe môže potvrdiť výslednú
rezervu. Po commissioningu možno osobitne zvážiť zjednodušenie časovania;
nejde o súčasť tejto migrácie.

## Safety invariants

- A2 sa inicializuje `INPUT_PULLUP`; HIGH/open path znamená LOW WATER.
- 5 s súvislého LOW WATER nastaví lokálny trip.
- A0 je energize-to-trip, boot/default LOW, a má jediného zapisovateľa.
- 10 s súvislého WATER zruší trip.
- D9 je boot/reset LOW a prejde HIGH až po 180 s platnej stability.
- UART, vzdialený XKC a agreement nie sú podmienkou lokálneho XKC tripu.
- Cross-reset zostáva compile-time vypnutý, bez GPIO a bez runtime účinku.
- Mega2 nepridáva SMART reguláciu ani command endpoint.

## Konflikty a aktuálne hranice zdrojov

- Staršie vety v MASTER/PINOUT uvádzajú V6 alebo automatický XKC test ako
  čakajúci, ale neskoršie fyzické záznamy potvrdzujú V6 linku a XKC TOTAL STOP
  commissioning. Migrácia vychádza z novšieho fyzicky potvrdeného stavu.
- HARDWARE_INVENTAR pred touto prípravou neevidoval druhú fyzickú Mega2560.
  Konkrétny kus, jeho napájanie a sensor shield sú preto NEOVERENÉ.
- BASIC_R1–R4 a cross-reset nemajú schválené piny. Nejde o prekážku portu
  dnešného implementovaného runtime, ale tieto budúce funkcie nie sú pripravené
  na fyzické zapojenie.

## Otvorené TBD pred fyzickou migráciou

1. Potvrdiť konkrétny kus Mega2, jeho napájaciu vetvu a mechanický sensor shield.
2. Overiť horné napätie existujúceho 4,7 kΩ OneWire pull-upu.
3. Overiť napájanie/level kompatibilitu konkrétneho SD modulu s Mega2560.
4. Overiť, že D18/D19 a D50–D53 nie sú na konkrétnom shielde interne obsadené.
5. Fyzicky potvrdiť boot LOW na D9/A0, A2 polaritu, 5 s/10 s XKC a 180 s agreement.
6. Zopakovať persistent DS fault test a sledovať V6 CRC/INV/GAP/timeout na HW UART.
7. Zmerať SD/worst-loop/SRAM stack watermark; väčšia SRAM sama nenahrádza test.
8. BASIC_R1–R4 a cross-reset riešiť iba v samostatne schválenej fáze.

## Commissioning checklist

1. Vypnúť a bezpečne odpojiť Uno aj Mega2; neprepájať medzi doskami +5 V.
2. Presunúť vždy jeden kábel z Una priamo na určený pin Mega2 sensor shieldu.
3. Pred ďalším vodičom overiť číslo pinu, smer, polaritu a cieľ na oboch koncoch.
4. Pri D7/D8 použiť mapu D19/RX1 a D18/TX1; oba 10 kΩ odpory ponechať.
5. Pri SD presunúť D11/D12/D13 na D51/D50/D52; CS ponechať D10.
6. Po kompletnej kontrole kabeláže nasadiť shield na Mega2.
7. Až potom vykonať samostatne schválený upload a fyzický test.
