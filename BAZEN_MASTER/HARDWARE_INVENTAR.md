# BAZEN MASTER – hardvérový inventár

Aktualizované: 2026-09-01

MÁME znamená fyzicky doručené. Použitie je uvedené iba podľa kódu alebo potvrdených poznámok. Neznáme údaje sú NEURČENÉ.

## Riadiace jednotky

| Položka | Kúpené | Použité | Kde použité | Plánovaná funkcia | Voľná zásoba | Stav |
|---|---:|---:|---|---|---:|---|
| Arduino Mega + WiFi, Techfun IOT382 | 1 | 1 | Mega + onboard ESP projekty | hlavný SMART kontrolér + samostatný ESP8266 pre web HMI | 0 | NAMONTOVANÉ; presné DIP režimy fyzicky NEOVERENÉ |
| KIT240 – Arduino UNO Elementary/Starter kit | 1 kit | 1 kit otvorený | Uno projekt + prototypovací materiál | zdroj Uno a pomocné prototypovanie | 0 celých kitov | MÁME |
| ESP8266 ESP-01S | 1 | 0 | SKLADOM | NEURČENÉ | 1 | SKLADOM / funkčné |

Techfun IOT382 je kombinovaná doska s ATmega2560 16 MHz, 256 kB Flash, 8 kB SRAM, 4 kB EEPROM, samostatným 32-bitovým ESP8266, CH340G, micro-USB a DIP smerovaním UART. Onboard ESP8266 je samostatný aplikačný procesor. Identita a parametre sú potvrdené produktovým podkladom Techfun; presná elektrická zhoda DIP 1–8 s referenčnou RobotDyn schémou a fyzická poloha UART selectoru na našej revízii zostávajú `NEOVERENÉ`.

### Obsah KIT240

KIT240 nebol nákup samostatnej dosky Uno. Obsah kitu sa eviduje samostatne:

| Súčasť KIT240 | Počet v kite | Použité | Kde použité | Plánovaná funkcia | Voľná zásoba | Stav |
|---|---:|---:|---|---|---:|---|
| Arduino UNO SMD klon | 1 | 1 | projekt Uno | bezpečnostná/BASIC jednotka | 0 | TESTOVANÉ |
| USB kábel | 1 | NEURČENÉ | prototypovací materiál | programovanie/napájanie pri vývoji | NEURČENÉ | MÁME |
| Nepájivé kontaktné pole 400 bodov | 1 | NEURČENÉ | prototypovací materiál | prototypovanie | NEURČENÉ | MÁME |
| Jack pre 9 V batériu | 1 | NEURČENÉ | prototypovací materiál | prototypovanie | NEURČENÉ | MÁME |
| M-M kábliky 20 cm | 20 ks | NEURČENÉ | prototypovací materiál | prototypovanie | NEURČENÉ | MÁME |
| Červené LED | 5 | NEURČENÉ | prototypovací materiál | prototypovanie | NEURČENÉ | MÁME |
| Zelené LED | 5 | NEURČENÉ | prototypovací materiál | prototypovanie | NEURČENÉ | MÁME |
| Žlté LED | 5 | NEURČENÉ | prototypovací materiál | prototypovanie | NEURČENÉ | MÁME |
| Fotorezistor | 2 | NEURČENÉ | prototypovací materiál | prototypovanie | NEURČENÉ | MÁME |
| Pinový pásik 2,54 mm | 40 pinov | NEURČENÉ | prototypovací materiál | prototypovanie | NEURČENÉ | MÁME |
| Tlačidlá + násady | NEURČENÉ | NEURČENÉ | prototypovací materiál | prototypovanie | NEURČENÉ | MÁME |
| Rezistory 220 Ω | NEURČENÉ | NEURČENÉ | prototypovací materiál | prototypovanie | NEURČENÉ | MÁME |
| Rezistory 1 kΩ | NEURČENÉ | NEURČENÉ | prototypovací materiál | prototypovanie | NEURČENÉ | MÁME |
| Rezistory 10 kΩ | NEURČENÉ | NEURČENÉ | prototypovací materiál | prototypovanie | NEURČENÉ | MÁME |
| Rezistory 100 kΩ | NEURČENÉ | NEURČENÉ | prototypovací materiál | prototypovanie | NEURČENÉ | MÁME |

Spotrebované a zostávajúce počty prototypovacích súčastí KIT240 ostávajú NEURČENÉ, kým nebudú fyzicky spočítané. Položky KIT240 sa nesčítavajú so samostatne kúpenými káblikmi, tlačidlami, 830-pinovým poľom ani sadou rezistorov.

## Snímače

| Položka | Kúpené | Použité | Kde použité | Plánovaná funkcia | Voľná zásoba | Stav |
|---|---:|---:|---|---|---:|---|
| DS18B20 vodotesný 1 m, Techfun | 3 | NEURČENÉ | spoločná zásoba | teplotné merania | NEURČENÉ | MÁME |
| DS18B20 vodotesný 1 m, drotik | 5 | NEURČENÉ | spoločná zásoba | teplotné merania | NEURČENÉ | MÁME |
| Súhrn vodotesných DS18B20 | 8 | 7 v aktuálnom kóde | MEGA_T1–T4, UNO_T1–T3 | nezávislé procesné teploty; UNO_T2 solárny výstup a UNO_T3 panel | 1 podľa aktuálneho kódu; presná fyzická zásoba NEURČENÁ | LOKÁLNA MIGRÁCIA UNO_T2/T3 IMPLEMENTOVANÁ A FYZICKY TESTOVANÁ `OK` |
| Holý DS18B20 klon | 3 | 2 | UNO_TBOX na Uno D2; MEGA_TBOX na Mega D2 | nezávislé monitorovanie vnútra rozvádzača a budúca vzájomná kontrola | 1 | 2× FYZICKY NAINŠTALOVANÝ/TESTOVANÝ, 1× REZERVA |
| HY-SRF05, prvý kus | 1 | 1 | Uno D3/D4 | hladina Uno | 0 | TESTOVANÉ |
| HY-SRF05, posledná objednávka | 2 | 1 | Mega D38/D39; druhý kus rezerva | monitorovanie hladiny Mega + rezerva | 1 | 1× NAMONTOVANÝ A FUNKČNE TESTOVANÝ / 1× SKLADOM |
| XKC-Y25-NPN 5–12 V | 1 | 1 | dve samostatné optočlenové read paths: Mega D30 a Uno A2 | aktuálne iba commissioning monitor LOW WATER; bez riadiacej/safety autority | 0 | FYZICKY ZAPOJENÉ; OUT ÚROVNE OVERENÉ / MCU COMMISSIONING ČAKÁ NA TEST |
| BH1750 | 1 | 0 | plánovaný lux vstup pre solárnu diagnostiku | pri strate oboch T3 iba podklad pre kontrolovaný skúšobný chod; tepelný zisk musí potvrdiť ustálené T2 | 1 | SKLADOM / PIN, ADRESA A PRAHY NEURČENÉ |
| AHT10 I²C | 1 | 1 | Mega, vonku v tieni, I²C D20/D21 | vonkajšia teplota a relatívna vlhkosť; diagnostická/informačná úloha | 0 | FYZICKY NAMONTOVANÉ / FUNKČNÉ / PREVÁDZKOVO OVERENÉ |
| Elektrický plavákový senzor hladiny | 1 | 0 | plánovaná fyzická cesta dopúšťania | nadradený hardvérový zákaz COAX ventilu pri maximálnej hladine | 1 | SKLADOM |
| Infračervený senzor plameňa | 1 | 0 | plánovaný ohrev bazéna | kontrola prítomnosti plameňa horáka pre ohrev cez výmenník/výmenníky | 1 | SKLADOM |

UNO_T1 je bazénový senzor s ROM `28 70 A6 08 91 25 06 55`. Nový dlhokáblový UNO_T2 pre vodu na výstupe zo soláru má fyzicky potvrdenú ROM `28 F8 23 5B 00 00 00 37`. Pôvodný krátkokáblový senzor s ROM `28 29 81 5E 00 00 00 F5` je UNO_T3 pre solárny panel. UNO_TBOX má ROM `28 16 2E 09 00 03 24 29`. Commissioning test našiel na spoločnej D2 zbernici všetky štyri senzory a potvrdil ich meranie `OK`; lokálny Uno kód ich samostatne meria, validuje, zotavuje, vypisuje a loguje. Startup ROM scanner bol po úspešnom teste odstránený a pri fyzickom V4 boote 22. 8. 2026 sa nespustil. Prenos UNO_T3 v 22 B V4 rámci do Mega a SMART fallback MEGA_T3→čerstvý UNO_T3 boli fyzicky overené 22. 8. 2026. Spoločná UNO 1-Wire zbernica používa jeden fyzicky osadený pull-up 4,7 kΩ; ďalší paralelný pull-up sa nepridáva a napätie jeho hornej strany zostáva `NEOVERENÉ`. MEGA_TBOX s ROM `28 B1 31 05 00 03 24 F1` je fyzicky nainštalovaný ako piaty DS18B20 na Mega D2 a je implementovaný iba na meranie a diagnostiku. Presné priradenie ôsmich kúpených káblových kusov k ROM adresám nie je evidované. AHT10 je fyzicky namontovaný, funkčný a overený v prevádzke; poskytuje Mega reálne údaje vonkajšej teploty a relatívnej vlhkosti, zatiaľ iba informačne/diagnosticky. Prvý HY-SRF05 je fyzicky testovaný na Uno D3/D4 a funguje; commissioning test nameral približne 11,6–11,7 cm a stav `OK`. Mega HY-SRF05 je fyzicky a funkčne overený na D38/D39; izolovaný test nameral približne 19,3–19,8 cm. Oba sonary zostávajú výhradne monitorovacie. Jeden XKC-Y25-NPN je fyzicky privedený dvoma optočlenovými cestami na Mega D30 a Uno A2. Meranie senzora potvrdilo voda/LED ON približne 0 V a sucho/LED OFF približne +5 V; oba MCU vstupy používajú `INPUT_PULLUP` a za optočlenom interpretujú `LOW = WATER`, `HIGH = LOW_WATER / DRY / otvorená cesta`. Aktuálna implementácia je iba commissioning monitor bez safety autority a čaká na fyzické overenie oboch MCU read paths.

## Relé, izolácia a napájanie

| Položka | Kúpené | Použité | Kde použité | Plánovaná funkcia | Voľná zásoba | Stav |
|---|---:|---:|---|---|---:|---|
| 16-relé modul, opticky oddelený, 5 V | 1 | 1 modul; R1/R2 v motorových cestách, R3 v 12 V W1209 ceste, R9/R10 aktívne v Mega kóde | R1 a R2 za WAGO rozdelením TOTAL STOP; R3 COM–NC medzi W1209 buck 12 V a supervision modulom; R4 rezerva; MEGA_R9/D22 filtrácia; MEGA_R10/D23 solár/chrlič | 1=BASIC_R1 filtrácia, 2=BASIC_R2 solár, 3=BASIC_R3 fyzicky v 12 V W1209 povoľovacej ceste, 4=BASIC_R4 rezerva; 5–16 Mega/SMART | R4 rezervované; neimplementované SMART funkcie podľa pinoutu | NAMONTOVANÉ / R1–R3 FYZICKY POUŽITÉ / R4 REZERVA / R5–R16 MEGA/SMART / RESIDUAL BACKFEED KNOWN–ACCEPTED |
| H/L relé 1 kanál 5 V – spoločná evidencia oboch objednávok | 5 | 5 | Uno agreement D9; Mega watchdog/povolenie riadené agreement logikou D31; Uno TOTAL STOP A0; Mega TOTAL STOP D32; W1209 fackovač D33 | agreement/watchdog povolenie, dve nezávislé TOTAL STOP autority a power-cycle 12 V W1209 | 0 | VŠETKÝCH 5 FYZICKY PRIPOJENÝCH/PRIDELENÝCH; `HIGH→COM–NO`, `LOW/strata napájania→COM–NC` FYZICKY OVERENÉ; automatické TOTAL STOP/W1209 fault podmienky NEIMPLEMENTOVANÉ |
| PC817 izolačný modul HY-M154, 4 kanály | 2 | 2 aktívne kanály pre XKC; presné rozdelenie medzi fyzické moduly NEOVERENÉ | dve samostatné XKC read paths na Mega D30 a Uno A2; PC817 nie je v aktuálnej UART dátovej ceste | galvanicky oddelené commissioning čítanie spoločného XKC oboma MCU | 6 kanálov bez aktuálne pridelenej funkcie; fyzická skladová/alokačná kontrola NEOVERENÁ | MÁME / XKC FYZICKY ZAPOJENÉ / MCU COMMISSIONING ČAKÁ NA TEST / UART POUŽITIE HISTORICKÉ |
| BC547 NPN | 5 | 0 | budúci hardvérový reset Mega↔Uno | low-side/open-collector-like pull-down RESET; nie galvanické oddelenie | 5 | SKLADOM / PINY, REZISTORY A PINOUT KUSU TBD |
| TOTAL STOP kontaktná cesta | 2 moduly zahrnuté v evidencii 5× H/L | 2 | Mega D32 z Mega power domain; Uno A0 z Uno power domain; sériovo COM–NC pred WAGO rozdelením do BASIC_R1/R2 | energize-to-trip spoločné odpojenie oboch motorových vetiev | 0 | MOTOROVÁ CESTA, PINY A COMMISSIONING OVLÁDANIE FYZICKY OVERENÉ / AUTOMATICKÉ FAULT PODMIENKY NEIMPLEMENTOVANÉ |
| DC/DC buck pre W1209 12 V | 1 fyzicky potvrdený; pôvod/model NEURČENÝ | 1 | `24 V → buck 12 V → BASIC_R3 COM–NC → W1209 fackovač COM–NC → W1209` | 12 V napájanie nezávislého regulátora W1209 | 0 | FYZICKY ZAPOJENÉ |
| W1209 kontaktné cesty | 1 supervision modul zahrnutý v evidencii 5× H/L; W1209 fyzicky potvrdený | 1 | W1209 fackovač D33 napájaný z Mega power domain; 12 V supervision cesta fyzicky hotová; 230 V `BASIC_R1 NC → COM1 → K1 → COM2 → K2`, za K2 nezapojené | power-cycle W1209 a dvojpodmienkové BASIC povolenie ohrevu | 0 | D33 COMMISSIONING PASS / 12 V CESTA FYZICKY ZAPOJENÁ / AUTOMATICKÁ RESET LOGIKA NEIMPLEMENTOVANÁ / FINAL K2 OUTPUT TBD |
| Siemens SITOP PSU200M 6EP1334-3BA10 | 1 | 1 | hlavné napájanie zostavy | hlavný zdroj systému | 0 | FYZICKY NAMONTOVANÉ |
| LM2596HVS | 1 | 1 | samostatná vetva watchdogu | napájanie watchdogu | 0 | FYZICKY NAMONTOVANÉ |
| DC/DC CC/CV buck do cca 300 W #1 | 1 | 1 | napájacia zostava | 7,5 V pre Arduino Mega | 0 | FYZICKY NAMONTOVANÉ |
| DC/DC CC/CV buck do cca 300 W #2 | 1 | 1 | napájacia zostava | 5 V pre 16R relé/BASIC vetvu; nenapája Arduino Uno | 0 | FYZICKY NAMONTOVANÉ |
| Samostatný LM2596 pre Arduino Uno | 1 fyzicky potvrdený; pôvod nákupnej dávky NEURČENÝ | 1 | samostatná napájacia vetva Una | 7,5 V napájanie Arduino Uno | 0 | FYZICKY NAMONTOVANÉ / NASTAVENÉ NA 7,5 V |
| DC/DC vetva ESP-01S | 1 | 0 aktívnych | mimo aktívnej Uno zostavy | ďalšie použitie NEURČENÉ | 1 | FYZICKY TESTOVANÁ / aktuálne NEAKTÍVNA |
| Historický nákup LM2596 | 2 | priradenie ku konkrétnym fyzickým kusom NEURČENÉ | presný vzťah k vetve Una, W1209 a bývalej ESP vetve nie je v tomto zázname potvrdený | evidenčný pôvod kusov; funkčné vetvy sú vedené samostatnými položkami | NEURČENÉ | NÁKUP POTVRDENÝ / AKTUÁLNE PRIRADENIE A SKLADOVÁ ZÁSOBA NEURČENÉ |
| DC/DC CC/CV pre nabíjanie | NEURČENÉ | 0 | medzi SITOP 24 V a nabíjacím dorazom | 12,2 V, max. 12 A pre plánovanú 3S vetvu | NEURČENÉ | PLÁNOVANÉ |
| Programovacia doska s relé | NEURČENÉ | 0 | hardvérový doraz nabíjania | fyzické obmedzenie nabíjacej cesty | NEURČENÉ | PLÁNOVANÉ |
| 3S BMS | NEURČENÉ | 0 | plánovaná batériová zostava | ochrana 3S batérie; model NEURČENÉ | NEURČENÉ | PLÁNOVANÉ |
| 3S batéria približne 500 Wh | NEURČENÉ | 0 | vložená do hlavnej napájacej cesty | UPS bez prepínania SITOP ↔ batéria | NEURČENÉ | PLÁNOVANÉ |
| Aktívny balancér 5 A | NEURČENÉ | 0 | plánovaná 3S zostava | aktívne balansovanie 3S batérie | NEURČENÉ | PLÁNOVANÉ |
| Step-up na 24 V | NEURČENÉ | 0 | budúca výstupná vetva batérie | 24 V výstup; model NEURČENÉ | NEURČENÉ | PLÁNOVANÉ |
| Budúca 3,3 V vetva | NEURČENÉ | 0 | budúca výstupná vetva batérie | napájanie 3,3 V; konkrétny menič NEURČENÉ | NEURČENÉ | PLÁNOVANÉ |

Fyzické kanály 1–4 jedinej 16R dosky patria výhradne BASIC_R1–R4; pôvodná väzba týchto kanálov na Mega D30–D33 bola odstránená zo zdrojového kódu aj fyzickej kabeláže. D30 je nanovo pridelený XKC commissioning vstupu, D31 fyzickej watchdog/povoľovacej vetve riadenej agreement logikou, D32/D33 samostatným modulom MEGA_TOTAL_STOP/W1209_FACKOVAC; nejde o obnovenie väzby na 16R kanály 1–4. BASIC_R1 a BASIC_R2 sú fyzicky v motorových cestách za spoločným TOTAL STOP/WAGO rozdelením. BASIC_R3 fyzicky pokračuje cez `COM–NC` v 12 V napájacej ceste W1209; BASIC_R4 zostáva rezervou. Konkrétne Uno/BASIC riadiace piny nie sú potvrdené. Kanály 5–16 zostávajú Mega/SMART; aktívne sú `MEGA_R9/D22 = filtrácia` a `MEGA_R10/D23 = solár/chrlič`. Pri mŕtvej Mega bol fyzicky potvrdený residual backfeed z napájanej 16R dosky približne 1,58 V a reálne zopnutie R5–R16; Mega pritom funkčne nežije a účinok SMART kanálov je blokovaný nadradenou agreement/BASIC povoľovacou cestou. Stav je podľa MASTER `KNOWN / PHYSICALLY CONFIRMED / FUNCTIONALLY ISOLATED / ACCEPTED`. D31 je pripojený na Mega watchdog/povoľovací modul a D9 na Uno agreement modul; polarita oboch kusov je fyzicky overená ako `HIGH → COM–NO`, `LOW/strata napájania → COM–NC`.

Dva TOTAL STOP moduly sú samostatné od agreement relé a od kanálov 16R dosky. Sú nezávisle napájané z Mega/Uno power domains a ich kontakty sú fyzicky zapojené sériovo `COM–NC` pred WAGO rozdelením do BASIC_R1/R2. Mega modul je fyzicky ovládaný z D32, Uno modul z A0; commissioning test potvrdil správne relé aj návrat LOW/COM–NC. W1209 fackovač je fyzicky ovládaný z Mega D33. Všetkých päť jednokanálových H/L modulov je funkčne pridelených a voľná zásoba je 0; automatické fault podmienky zatiaľ nie sú implementované.

Dva veľké CC/CV buck meniče do približne 300 W, samostatný 7,5 V LM2596 pre Uno a watchdog LM2596HVS sú odlišné fyzické vetvy. Veľký buck #2 napája iba 5 V vetvu 16R relé/BASIC. Samostatný nový LM2596 pre W1209 tvorí ďalšiu oddelenú 12 V vetvu. Pôvod jednotlivých malých LM2596 v nákupných dávkach nie je všade jednoznačne potvrdený, preto sa ich funkčné vetvy nesmú zlučovať iba podľa typu modulu. Presný model DC/DC použitý v samostatnej overenej vetve ESP-01S nie je v tomto zázname potvrdený.

Všetky komponenty 3S UPS vrstvy sú PLÁNOVANÉ. Batéria približne 500 Wh ešte nie je fyzicky postavená. Nie sú potvrdené typy článkov, paralelné skupiny, model BMS, model 24 V step-up ani poistky.

## HMI, čas a záznam

| Položka | Kúpené | Použité | Kde použité | Plánovaná funkcia | Voľná zásoba | Stav |
|---|---:|---:|---|---|---:|---|
| LCD 20×4 + I²C | 1 | 1 v kóde | Mega, adresa 0x27 | lokálne HMI | 0 | NEURČENÉ |
| RTC DS3231 + AT24C32 | 1 | 1 v kóde | Mega, adresa 0x68 | čas/harmonogram | 0 | NEURČENÉ |
| MicroSD modul, veľká doska | 2 | 1 | Uno D10–D13 | UNO diagnostické logovanie do `UNO_LOG.CSV`; druhý kus plánovaný pre Mega | 1 | 1× TESTOVANÉ, 1× MÁME / NEOTESTOVANÉ |
| MicroSD karta 4 GB FAT32 | 1 | 1 | Uno MicroSD modul | UNO diagnostické logovanie | 0 | TESTOVANÉ |
| MicroSD karta 1 GB | 1 | 0 | pripravená pre bazénový logger | záznam do budúceho MicroSD modulu | 1 | MÁME / NEOTESTOVANÉ |
| Tlačidlo s násadou | 10 | 5 v kóde | Mega D45–D49 | SET, +, −, FIL 6H, CHR 1H | 5 podľa kódu | NEURČENÉ |

## Dopúšťanie vody

| Položka | Kúpené | Použité | Kde použité | Plánovaná funkcia | Voľná zásoba | Stav |
|---|---:|---:|---|---|---:|---|
| Elektrický plavákový bezpečnostný kontakt | 1 | 0 | séria v ovládacej ceste dopúšťania | fyzicky odobrať povolenie relé/COAX ventilu pri maximálnej hladine | 1 | SKLADOM |
| Relé dopúšťania | NEURČENÉ | 0 | plánovaná cesta dopúšťania | spínanie COAX ventilu podriadené elektrickému plaváku | NEURČENÉ | PLÁNOVANÉ |
| COAX ventil | NEURČENÉ | 0 | prívod vody do bazéna | elektricky ovládané dopúšťanie | NEURČENÉ | PLÁNOVANÉ |
| Mechanický plavákový uzáver | NEURČENÉ | 0 | za COAX ventilom na prívode | posledná čisto mechanická ochrana proti pretečeniu | NEURČENÉ | PLÁNOVANÉ |
| T_FILL – snímač teploty dopúšťacej vody | NEURČENÉ | 0 | plánovaný prívod vody pred zmiešaním s bazénom | diagnostické meranie teploty prítoku, tepelný dopad dopustenia a korelácia s ventilom/budúcim flow/hladinou | NEURČENÉ | PLÁNOVANÉ / TYP, KUS, ROM, MCU VLASTNÍK A PIN NEURČENÉ |

Elektrický plavák nie je obyčajný informačný vstup. Arduino môže žiadať dopúšťanie, ale plavák musí byť fyzicky v povoľovacej ceste a nesmie byť softvérovo obíditeľný.

T_FILL je iba budúca meracia/diagnostická vrstva. Bez potvrdeného reálneho dopúšťania alebo prietoku môže merať stojacu vodu v potrubí; nenahrádza elektrický ani mechanický plavák a nie je safety dôkazom.

## Prototypovanie

| Položka | Kúpené | Použité | Kde použité | Plánovaná funkcia | Voľná zásoba | Stav |
|---|---:|---:|---|---|---:|---|
| Kábliky M-F 20 cm, 40 ks | 2 balenia | NEURČENÉ | NEURČENÉ | prototypovanie | NEURČENÉ | MÁME |
| Kábliky M-M 20 cm, 40 ks | 2 balenia | NEURČENÉ | NEURČENÉ | prototypovanie | NEURČENÉ | MÁME |
| Kábliky F-F 20 cm, 40 ks | 2 balenia | NEURČENÉ | NEURČENÉ | prototypovanie | NEURČENÉ | MÁME |
| DuPont M-F 40 cm, 40 ks | 1 balenie | NEURČENÉ | NEURČENÉ | prototypovanie | NEURČENÉ | MÁME |
| Nepájivé pole 830 pinov | 1 | NEURČENÉ | NEURČENÉ | prototypovanie | NEURČENÉ | MÁME |
| Sada rezistorov 600 ks | 1 sada | NEURČENÉ | NEURČENÉ | pomocné obvody | NEURČENÉ | MÁME |

Jeden MicroSD modul s kartou 4 GB FAT32 je fyzicky zapojený na Uno a otestovaný. UNO zapisuje `UNO_LOG.CSV` v intervale 60 s a zmeny lokálnych/komunikačných stavov do `UNO_EVT.CSV`; chyba SD nesmie ovplyvniť meranie ani ostatné funkcie a obnova SD sa skúša automaticky raz za 60 s. Druhý MicroSD modul a 1 GB microSD karta zatiaľ nie sú fyzicky otestované v module. RTC DS3231 ostáva iba na Mega. Aktuálna diagnostická linka Mega Serial2 D16/D17 ↔ Uno D7/D8 je priame neinvertované TTL prepojenie cez 10 kΩ v každom smere, so spoločnou GND, bez spoločného +5 V a bez PC817 v dátovej ceste. Používa 38400 Bd, MASTER→REPLY a V5 protokol 24/22 B; V5 commissioning bity ešte čakajú na fyzický test XKC ciest. Linka nie je safety prvok.

ESP-01S bol fyzicky odstránený z Uno zostavy. Modul zostáva funkčný a je uložený SKLADOM; budúce použitie je `NEURČENÉ`. UNO sketch už neobsahuje ESP/Wi-Fi podporu a D5/D6 sú voľné. Samostatná fyzicky overená 3,30 V vetva už nie je aktívnou súčasťou Uno zostavy; ďalšie použitie je `NEURČENÉ`.

Plánované logovanie: Mega bude zapisovať hlavný prevádzkový záznam v základnom intervale 1 minúta. Uno už má funkčný 60-sekundový diagnostický záznam a dlhodobo má zapisovať najmä eventové/diagnostické údaje, nie plnú duplicitnú minútovú kópiu.

## Posledná Techfun objednávka – FYZICKY DORUČENÁ

- 2× MicroSD modul, veľká doska;
- 1× XKC-Y25-NPN 5–12 V;
- 2× H/L relé 5 V;
- 2× HY-SRF05;
- 3× holý DS18B20 klon.
