# BAZEN MASTER – koncepcia

Aktualizované: 2026-08-22

## Zdroje pravdy a názvoslovie

Dokument vychádza z projektov bazenova_automatika (Mega), bazenova_automatika uno (Uno), bazen_wifi (onboard ESP8266), potvrdených fyzických testov a nákupných zoznamov.

Otvorené auditné nálezy a stav ich riešenia: AUDIT_BACKLOG.md

V dokumentácii používame:

- MEGA_T1 až MEGA_T4 pre procesné senzory Mega;
- UNO_T1, UNO_T2, UNO_T3 a UNO_TBOX pre aktuálne senzory Uno;
- MEGA_TBOX pre fyzicky nainštalovaný DS18B20 vnútornej teploty rozvádzača na Mega;
- BASIC_R1 až BASIC_R4 pre fyzické kanály 1–4 jedinej existujúcej 16-relé dosky: BASIC filtráciu, solár, 24 V a 12 V;
- MEGA_R5 až MEGA_R16 pre fyzické kanály 5–16 tej istej dosky, ktoré zostávajú reléovou časťou Mega/SMART.

Pôvodná väzba Mega `R1`–`R4` na D30–D33 už neplatí a bola zo zdrojového kódu aj fyzickej kabeláže odstránená. BASIC_R1/R2 sú fyzicky vložené do motorových FIL/SOLAR ciest cez BASIC/watchdog architektúru. BASIC_R3 je fyzicky zapojený kontaktom `COM–NC` v 12 V napájacej ceste W1209; BASIC_R4 zostáva rezervovaný a fyzicky ďalej nezapojený. Konkrétne Uno/BASIC riadiace piny a autonómna logika zostávajú neurčené.

## NÁVRHOVÝ KONTEXT A FILOZOFIA PROJEKTU – ZÁVÄZNÉ PRAVIDLO

Projekt nevychádza iba z hobby Arduino návrhov a katalógových údajov výrobcov. Pri rozhodovaní sa zohľadňuje praktická skúsenosť autora s údržbou, diagnostikou, opravami a návrhom zlepšení technických systémov z viacerých oblastí:

- priemyselná údržba strojov a zariadení;
- generálne opravy a návrhy technických zlepšení;
- výrobná technológia a dlhodobá diagnostika reálnych porúch;
- serverová a PC technika, diagnostika a skladanie systémov;
- energetika a technológia vodných elektrární;
- batériové systémy, Li-ion a LiFePO4 zostavy vrátane napájania a ochrán;
- mechanika, pneumatika, hydraulika a súvisiace technické oblasti.

Katalógový údaj alebo datasheet opisuje očakávanú normálnu funkciu zariadenia, nie absolútnu záruku jeho reálneho správania pri poruche. Návrh preto počíta aj so stavmi ako zamrznutá hodnota, neúplné zlyhanie elektroniky, občasná komunikačná chyba, rušenie, zaseknutý výstup, chybný senzor alebo nevhodná kombinácia viacerých porúch. Aj stav označený výrobcom za nepravdepodobný sa môže v dlhodobej reálnej prevádzke vyskytnúť.

Zároveň sa nesmie komplikovať alebo prestavovať funkčný a fyzicky overený základ iba preto, že existuje teoreticky dokonalejšie riešenie. Záväzný postup je:

1. určiť fyzickú príčinu alebo aspoň triedu poruchy;
2. zistiť jej reálny dopad na prevádzku a bezpečnosť;
3. ak možno chybu bezpečne izolovať malým zásahom, uprednostniť tento zásah;
4. zachovať funkčné a overené vrstvy systému;
5. veľkú prestavbu vykonať iba vtedy, keď malá izolácia nestačí alebo by vytvárala nové riziko.

Praktickým príkladom je kolízia periodickej OneWire komunikácie so SoftwareSerial RX na Uno. Namiesto prestavby celej komunikácie bol interval merania UNO DS18B20 zmenený z presných 5 000 ms na 5 300 ms. Tým sa odstránil stabilný fázový pomer voči približne sekundovému V3 requestu a obmedzilo sa zhlukovanie chýb. Fyzikálna príčina zostáva známa a dokumentovaná; prevádzkovo overené malé riešenie samo osebe nevytvára dôvod na prestavbu funkčného systému.

Nadradené pravidlo projektu je:

> **Neopravovať fabriku, keď stačí odstrániť konkrétny problém.**

Bezpečnosť má vždy prednosť. Technická elegancia sama osebe však nie je dôvodom na zbytočné zvýšenie zložitosti, pridanie ďalších miest poruchy alebo nahradenie funkčnej a overenej vrstvy. Budúci návrh musí odlišovať nevyhnutné bezpečnostné opatrenie od úpravy vykonanej iba pre teoretickú dokonalosť.

## SCHVÁLENÉ – TVRDÉ ARCHITEKTONICKÉ PRAVIDLO

Táto sekcia je nadradená neskorším návrhom jednotlivých funkcií. Nesmie sa meniť ani obchádzať bez výslovného schválenia. Ak budúca požiadavka, optimalizácia alebo implementácia začne týmto pravidlám odporovať, zmena sa nesmie vykonať automaticky; konflikt sa musí najprv pomenovať a predložiť na rozhodnutie.

### Pevné role procesorov

- **Mega = MASTER / CONTROL.** Patrí naň normálna SMART regulácia, komplexná diagnostika, porovnávanie Mega↔Uno senzorov, `CONFLICT/SUSPECT`, výber primárnych a fallback hodnôt, fyzikálne kontroly T1/T2/T3, 10-minútová diagnostika ustáleného solárneho okruhu, budúca lux logika, RTC, harmonogramy a ďalšie výpočtovo alebo pamäťovo náročné funkcie, ktoré nie sú nevyhnutné pre nezávislú bezpečnosť Una.
- **Uno = SUPERVISOR / SAFETY / BASIC / BLACK BOX.** Nie je druhý SMART regulátor. Patrí naň nezávislá sada fyzických senzorov, ich základná lokálna validácia, dohľad nad Mega, budúci watchdog a reset Mega, kritické kontroly, LOW WATER/chod nasucho, jednoduchá autonómna BASIC logika, budúce ovládanie BASIC_R1–R4 podľa osobitne schválenej logiky a SD/event logger ako čierna skrinka.
- Uno musí vedieť zachytiť posledný relevantný stav pred poruchou Mega. Pri smrti Mega však jeho kritické lokálne SAFETY/BASIC funkcie nesmú závisieť od diagnostiky Mega ani od UART.

### Redundancia a hierarchia režimov

Redundancia fyzických senzorov neznamená redundanciu komplexných výpočtov. Mega a Uno môžu mať nezávislé T1, T2, budúce T3, TBOX, sonar a ďalšie schválené vstupy. Uno ich používa na lokálne `OK/CHYBA`, nezávislý dohľad, safety, BASIC, prenos údajov Mega a záznam udalostí. Komplexné rozdiely, konflikty, vierohodnosť, fallback a súvislosti T1/T2/T3 vyhodnocuje prednostne Mega. Ak Mega žije, Uno môže prijať už hotový výsledok, ktorý skutočne potrebuje.

Schválená hierarchia je **SMART → DEGRADED → BASIC → STOP**:

- **SMART:** Mega riadi bazén, Uno vykonáva nezávislý dohľad a budúca hardvérová dohoda povoľuje SMART. Mega sama ani Uno samo netvoria plnohodnotný SMART. UART sám nikdy nie je safety povolenie SMART.
- **DEGRADED:** zvládnuteľná nekritická porucha, izolovaný výpadok alebo schválený fallback. Nemusí znamenať BASIC. Stav môže byť lokálny pre konkrétnu funkciu; porucha soláru napríklad nesmie bez dôvodu odstaviť filtráciu.
- **BASIC:** nastupuje pri strate dôvery v SMART vrstvu, napríklad pri zamrznutí procesora, opakovanom resete alebo odmietnutí SMART budúcim watchdogom/HW dohodou. Nie je druhý SMART; obsahuje iba jednoduché, robustné, vopred schválené autonómne funkcie. Uno ho musí vedieť vykonávať bez Mega a bez UART. Konkrétna logika BASIC_R1–R4 sa schváli osobitne.
- **STOP:** nadradená reakcia na kritický fyzický stav, napríklad LOW WATER, chod nasucho alebo potvrdený únik. Vypína funkcie, ktoré by mohli spôsobiť škodu alebo poruchu zhoršiť; nemusí vypnúť celý rozvádzač. Safety blokácia má prednosť pred SMART aj BASIC.

Základné prechody sú: zvládnuteľná porucha `SMART → DEGRADED`, strata dôvery v SMART vrstvu `SMART/DEGRADED → BASIC` a kritická fyzická porucha `SMART/DEGRADED/BASIC → príslušný STOP`. Návrat po chybe nemusí byť okamžitý; recovery, stabilizačné časy a ACK sa schvália samostatne.

### Základný autoritatívny runtime `SYSTEM_MODE` – IMPLEMENTOVANÉ / VYBRANÉ PRECHODY FYZICKY POTVRDENÉ

Mega obsahuje jedinú autoritatívnu premennú `SystemMode systemMode` so stavmi `MODE_SMART`, `MODE_DEGRADED`, `MODE_BASIC` a `MODE_STOP`. Resolver sa vykonáva na jednom mieste po aktuálnom `bezpecnost()` a pred reguláciou/výstupmi; týmto checkpointom režim iba pravdivo sumarizuje existujúci stav a nemení reguláciu ani fyzické relé.

Pevná priorita resolvera je `STOP > BASIC > DEGRADED > SMART`:

1. `MODE_STOP`: má prvú prioritu. Aktuálne implementovanou explicitnou STOP podmienkou je lokálny `MEGA_XKC_TRIP` po 5 000 ms súvislého LOW WATER na Mega D30. Bežná senzorová chyba, `system_OK=false`, `SOLAR_CONTROL_VALID=false`, vzdialený XKC stav, `XKC_CONFLICT` ani kompatibilný health stav `2` sa samy priamo nemapujú na STOP.
2. `MODE_BASIC`: nastane, ak nie sú súčasne potvrdené `megaAgreementOn`, čerstvý `unoAgreementOnRemote`, `unoLinkStavOk` a `REMOTE_DATA_VALID`. Boot, reset, strata linky a celý 180 s stabilizačný interval preto zostávajú BASIC.
3. `MODE_DEGRADED`: vyhodnotí sa iba pri zachovanej SMART autorite oboch agreement a používa dnešný `systemDegradovany`. Jeho aktuálne zdroje sú neprimárny pool zdroj, lokálna chyba T2, lokálna chyba T3, chyba MEGA_TBOX a remote stale; remote stale však vďaka vyššej priorite resolvera vytvorí BASIC. Fyzicky potvrdený príklad je `MEGA_T3_CHYBA + čerstvý UNO_T3_REMOTE_FALLBACK + oba agreement ON → MODE_DEGRADED`.
4. `MODE_SMART`: obe agreement sú potvrdené, linka a remote údaje sú čerstvé a `systemDegradovany=false`.

Uno neposudzuje druhú SMART analytiku. Vo V5 Uno→Mega rámci používa bajt 20 ako flagy: bit 0 znamená `UNO_AGREEMENT_ON`, bit 1 `UNO_XKC_LOW_WATER` a bity 2–7 musia byť nulové. Mega považuje vzdialený agreement za ON iba z čerstvého validného rámca. Veľkosti 22/24 B, CRC-8/ATM, sekvencie, MASTER→REPLY, 500 ms reply okno, 10 s timeouty a 180 s stabilizácia zostávajú nezmenené; aktuálna fyzická linka používa 38400 Bd. Prenášaný vzdialený XKC flag zostáva iba diagnostický a nevstupuje do agreement ani `SYSTEM_MODE`; do `MODE_STOP` vstupuje výhradne lokálne potvrdený Mega XKC trip.

`megaStav 0/1/2` a `aktualnyUnoStav() 0/1/2` zostávajú kompatibilnou health/agreement telemetriou, nie druhou autoritou `SYSTEM_MODE`. `megaStav` nepočíta chybné senzory; klasifikuje dôveryhodnosť Mega ako CONTROL vrstvy. Mega posiela `0`, ak je control vrstva plne v poriadku (`FIL_CONTROL_VALID=true` a `systemDegradovany=false`); `1`, ak Mega zostáva dôveryhodná, ale niektorá funkcia používa fallback, je degradovaná alebo bezpečne izolovaná; `2` iba pri explicitnej implementovanej strate dôveryhodnosti control vrstvy. V dnešnom rozsahu je jedinou takou podmienkou `FIL_CONTROL_VALID=false`. Neplatný pool, T2 alebo T3 a z toho odvodený `SOLAR_CONTROL_VALID=false` zostávajú stavom 1, pretože existujúca výstupná logika izoluje solár OFF a filtrácia zostáva platná. Uno povoľuje stabilizáciu pri `megaStav` 0 aj 1 a odmieta ju pri 2. Serial Mega zobrazuje `SYSTEM_MODE=... REASON=...` v 10 s diagnostike a okamžitý event iba pri zmene režimu.

### Nezávislá klasifikácia MODE / FAULT PRIORITY / BASIC PERMISSION

**SCHVÁLENÉ TVRDÉ ARCHITEKTONICKÉ PRAVIDLO – IMPLEMENTÁCIA ČAKÁ NA FYZICKÉ TOTAL STOP RIADENIE A KONKRÉTNU KLASIFIKÁCIU PORÚCH.** Prevádzkový režim systému, priorita poruchy a povolenie BASIC sú tri nezávislé veličiny:

- `SYSTEM MODE`: `SMART / DEGRADED / BASIC / STOP`;
- `FAULT PRIORITY`: `INFO / WARNING / FAULT / CRITICAL`;
- `BASIC PERMISSION`: `BASIC_ALLOWED=YES / BASIC_ALLOWED=NO`.

`DEGRADED` preto automaticky neznamená nízku prioritu ani povolený BASIC. Ak je fyzická porucha bezpečná iba preto, že ju živý SMART aktívne izoluje alebo nepoužíva, ale BASIC nevie rovnakú izoláciu garantovane zachovať, platí súčasne:

    MODE = DEGRADED
    FAULT_PRIORITY = CRITICAL
    BASIC_ALLOWED = NO

Poškodená funkcia alebo vetva môže počas funkčného SMART zostať izolovaná a systém môže pokračovať v `DEGRADED`. Ak však počas tejto poruchy zanikne dôveryhodný SMART, normálny BASIC fallback je zakázaný a musí nasledovať fyzický TOTAL STOP motorovej vetvy. Tento stav sa označuje aj ako `BASIC_CRITICAL` alebo servisne `CRITICAL – BASIC INHIBITED`.

Prechodové pravidlá:

- `DEGRADED + BASIC_ALLOWED=YES + strata SMART → BASIC` podľa existujúcej prechodovej architektúry;
- `DEGRADED + BASIC_ALLOWED=NO + strata SMART → TOTAL STOP`, nie BASIC;
- samotné `DEGRADED` bez kritickej inhibície BASIC nevyvoláva TOTAL STOP;
- samotná strata procesora naďalej znamená BASIC iba vtedy, ak neexistuje aktívna porucha s `BASIC_ALLOWED=NO` ani iná kritická fyzická STOP podmienka.

HMI a BLACK BOX musia pri aktívnom `BASIC_ALLOWED=NO` jasne rozlíšiť, že systém ešte pracuje v `DEGRADED`, poškodená funkcia je momentálne izolovaná SMARTom, BASIC fallback je zakázaný a strata SMART vyvolá TOTAL STOP. Nestačí zobraziť iba všeobecné `DEGRADED`.

UART nesmie byť jedinou safety pamäťou ani jediným dôkazom `BASIC_ALLOWED=NO`. Každá doska smie aktivovať vlastný TOTAL STOP iba z poruchy, pre ktorú má podľa osobitne schválenej implementácie dostatočný lokálny alebo nezávislý fyzický dôkaz. Prenos klasifikácie cez V4 môže slúžiť na synchronizáciu, HMI a BLACK BOX, nie ako jediná safety autorita.

Toto pravidlo nemení vlastníctvo W1209 supervision: patrí Mega a zostáva samostatnou funkciou. Uno týmto nezískava oprávnenie resetovať ani odpájať W1209.

**Aktuálny implementačný stav:** TOTAL STOP relé sú osadené, samostatne napájané a ich sériová kontaktná cesta `COM–NC` pred WAGO rozdelením do BASIC_R1/R2 je fyzicky zapojená. Mega D32 a Uno A0 sú pripojené a priamym commissioning testom overené. XKC Safety V1 je prvá konkrétna automatická TOTAL STOP podmienka: každá doska používa iba vlastný lokálny XKC vstup, po 5 000 ms LOW WATER aktivuje vlastné relé a po 10 000 ms WATER ho uvoľní. Dňa 2026-09-02 obe lokálne vetvy pri fyzicky odpojenom UART samostatne zopli svoje TOTAL STOP relé, po WATER a 10 s recovery ich uvoľnili a prerušený recovery interval správne začal odznova; výsledok je `PHYSICAL PASS / COMMISSIONED`. Otvorené zostáva samostatné riziko resetu MCU počas trvalého LOW WATER. Model `FAULT_PRIORITY/BASIC_ALLOWED` ani iné automatické TOTAL STOP dôvody zatiaľ nie sú zapojené do produkčného rozhodovania.

#### OPEN / REQUIRED BEFORE IMPLEMENTATION – zachovanie BASIC inhibície po smrti detegujúcej dosky

Súčasný koncept TOTAL STOP používa `energize-to-trip` relé s pokojovou cestou `COM–NC`. Táto vlastnosť je vhodná pre aktívnu reakciu živej autority, ale sama negarantuje trvalé zachovanie `BASIC_ALLOWED=NO` po strate napájania alebo smrti dosky, ktorá poruchu zistila. Kritický scenár je:

1. Mega zistí poruchu `SMART-DEGRADED / BASIC-CRITICAL`;
2. Mega aktivuje vlastný TOTAL STOP a preruší motorovú vetvu;
3. Mega následne stratí napájanie, zamrzne alebo zomrie;
4. jej energize-to-trip relé odpadne späť na `COM–NC`;
5. bez ďalšej nezávislej ochrany by sa motorová cesta mohla znovu spriechodniť práve v stave, v ktorom je BASIC zakázaný.

Preto sa `BASIC_ALLOWED=NO → TOTAL STOP` nesmie implementovať ako ochrana závislá iba od držania relé jedinou doskou, ktorá poruchu detegovala. Pred implementáciou každej konkrétnej `BASIC_ALLOWED=NO` poruchy musí byť schválená a fyzicky overená najmenej jedna z týchto rovnocenných ciest:

- Uno má nezávislý fyzický dôkaz tej istej poruchy a vlastnou autoritou drží svoj TOTAL STOP;
- existuje hardvérový latch alebo iný perzistentný blok, ktorý zachová bezpečný zákaz aj po smrti detegujúcej dosky;
- fyzická architektúra COAX ventilov alebo konkrétnej hydraulickej vetvy poskytuje inú nezávislú ochranu s rovnocenným výsledkom.

UART oznámenie, RAM príznak alebo posledný stav od Mega nie sú samy osebe postačujúce riešenie. Pre každú budúcu `BASIC_ALLOWED=NO` poruchu sa musí pri fyzickom návrhu osobitne zdokumentovať: zdroj nezávislého dôkazu, autorita reakcie, správanie pri strate oboch napájacích stavov, spôsob zachovania blokácie a podmienky bezpečného resetu/ACK. Konkrétny latch, piny, zapojenie a zoznam porúch zostávajú `OPEN / TBD` a nesmú sa domyslieť pri implementácii.

### Reakcia na poruchu procesorov a RESET – SCHVÁLENÉ / TVRDÁ ARCHITEKTÚRA

Táto časť je nadradená všetkým starším návrhom resetu, watchdogu a reakcie na stratu procesora. **Porucha, strata komunikácie, reset alebo trvalá nedostupnosť jedného procesora sama osebe nie je dôvod na STOP.** Mega a Uno/SUPERVISOR sa majú navzájom sledovať. Ak jeden procesor prestane byť dôveryhodný alebo prestane komunikovať, funkčná strana zruší svoje povolenie SMART a hardvérová dohoda musí viesť systém do BASIC.

Platí záväzné rozlíšenie:

- `PROCESSOR FAILURE ≠ STOP`;
- `PROCESSOR FAILURE → BASIC`;
- `RESET FAILURE / RESET_LOCKOUT → BASIC`;
- `CRITICAL PHYSICAL SAFETY CONDITION → STOP` pre dotknutú technológiu.

Funkčný procesor môže v budúcnosti vykonať obmedzený počet pokusov o reset nedôveryhodného procesora. Automatický reset nesmie prebiehať donekonečna. Po každom pokuse musí byť dostatok času na boot, inicializáciu, self-test, obnovenie komunikácie a synchronizáciu. Po vyčerpaní povolených pokusov vznikne `RESET_LOCKOUT`, ktorý zakáže ďalšie automatické resetovanie v danom poruchovom incidente. `RESET_LOCKOUT` neznamená STOP: ak nezávislé podmienky BASIC zostávajú bezpečné, systém pokračuje v BASIC aj s trvalo nedostupným druhým procesorom.

Pracovný návrh je najviac **3 automatické resetovacie pokusy v jednom poruchovom incidente**. Hodnota 3, rozostupy, boot timeout, dĺžka self-testu, podmienky nového incidentu a spôsob zrušenia `RESET_LOCKOUT` sú stále **NÁVRH – TREBA FYZICKY OVERIŤ**. Zatiaľ nie sú implementované ani pinovo priradené.

#### RESET GRACE – servisné čakacie okno

**SCHVÁLENÉ PRAVIDLO, NEIMPLEMENTOVANÉ.** Po zistení nedostupnosti, resetu, výpadku komunikácie alebo straty dôvery v druhú dosku sa SMART agreement zruší okamžite a systém prejde podľa existujúcej architektúry do BASIC. Hardvérový automatický RESET druhej dosky sa však nesmie vykonať okamžite. Najprv musí prebehnúť `RESET_GRACE`, ktorého presná dĺžka zostáva `TBD`.

Grace okno chráni normálny servisný zásah: pripojenie k PC, otvorenie programovania, upload firmvéru a USB/autoreset môžu prirodzene vyvolať reset procesora a dočasnú stratu UART. Druhá doska nesmie tento servisný reset prekryť vlastným hardvérovým resetom a rušiť alebo znemožniť upload.

- ak sa doska počas `RESET_GRACE` sama korektne vráti, fyzický RESET sa nevydá;
- samovoľný návrat bez vydaného fyzického RESET impulzu nespotrebuje automatický resetovací pokus;
- až po uplynutí grace okna, ak doska zostáva nedôveryhodná, môže zdravá strana vydať fyzický RESET;
- pracovný návrh zostáva najviac 3 automatické resetovacie pokusy na incident;
- po vyčerpaní pokusov platí `RESET_LOCKOUT → ďalšie automatické resety zakázané → BASIC`; `RESET_LOCKOUT ≠ STOP`.

Budúci hardvérový reset má používať BC547 ako tranzistorový low-side/open-collector-like pull-down vstupu RESET. BC547 nie je galvanické oddelenie. Konkrétne piny MCU, rezistory, zapojenie a čas impulzu zostávajú `TBD`; pinout konkrétneho fyzického BC547 sa musí pred montážou overiť.

`RESET_GRACE` ani `RESET_LOCKOUT` nesmú odložiť LOW WATER, dry-run, kritický leak alebo inú schválenú kritickú fyzickú reakciu. Po resete zostáva povinná sekvencia `BOOT → SELF_TEST → LINK/SYNC OK → STABLE → AGREEMENT → SMART` vrátane existujúcej 180-sekundovej nepretržitej stabilizácie.

Návrat do SMART po resete nesmie vyvolať prvý prijatý UART rámec. Obnovený procesor musí prejsť minimálne sekvenciou:

`BOOT → SELF_TEST → LINK/SYNC OK → STABLE → AGREEMENT → SMART povolené`

Obe strany môžu znovu povoliť SMART až po úspešnom dokončení tejto sekvencie a budúcej hardvérovej dohody. Konkrétne stabilizačné časy, self-test podmienky a ACK/recovery pravidlá zostávajú **NÁVRH – TREBA FYZICKY OVERIŤ**.

Kritické fyzické ochrany sú nadradené SMART, DEGRADED, BASIC, resetu aj `RESET_LOCKOUT`. LOW WATER a ďalšie neskôr schválené kritické vstupy majú Mega a Uno sledovať priamo a nezávisle. Každá doska má samostatné právo vyvolať príslušnú fyzickú bezpečnostnú reakciu bez súhlasu druhej. Reset procesora nesmie zrušiť aktívnu kritickú príčinu ani obísť fyzickú blokáciu.

STOP je posledný stupeň pre stav, pri ktorom už nie je bezpečné pokračovať ani v BASIC. Patrí sem napríklad potvrdený LOW WATER/chod nasucho, relevantný potvrdený únik alebo miznutie vody a iná neskôr výslovne schválená kritická fyzická safety podmienka. STOP nie je všeobecná reakcia na obyčajnú stratu procesora alebo UART.

Ak sa v historickom návrhu nachádza všeobecné tvrdenie `strata procesora → STOP`, okamžitý návrat do SMART po prvom rámci alebo neobmedzené automatické resetovanie, je od tohto rozhodnutia **NEAKTUÁLNE** a nesmie sa použiť ako podklad budúcej implementácie.

### SMART ↔ BASIC – prechodová a anti-flap ochrana – SCHVÁLENÉ / TVRDÁ ARCHITEKTÚRA

Táto vrstva zachováva hierarchiu **SMART → DEGRADED → BASIC → STOP** a rozlišuje okamžitý hardvérový fallback od riadeného prechodu živého systému. Konkrétne piny, dead-time, minimálne OFF časy a časovanie jednotlivých výstupov zatiaľ nie sú určené ani implementované.

#### SMART AGREEMENT – 2× ÁNO

SMART režim smie existovať iba vtedy, keď ho nezávisle povoľujú obe riadiace dosky:

`SMART = MEGA_AGREEMENT AND UNO_AGREEMENT`

Záväzná logika je:

- 2× `ÁNO` → SMART môže byť povolený, ak sú splnené aj ostatné schválené podmienky;
- 1× `NIE` → SMART nesmie pokračovať → prechod alebo hardvérové sprístupnenie BASIC;
- kritická fyzická safety podmienka → STOP podľa definície príslušnej ochrany.

Každá doska musí mať možnosť nezávisle zrušiť vlastný SMART agreement. Na zrušenie SMART nie je potrebná spolupráca druhej dosky. Toto pravidlo musí pokrývať minimálne fyzickú stratu napájania procesora, reset/reboot, zamrznutie, stratu požadovaného heartbeat/dohľadu, potvrdenú stratu alebo nedôveryhodnosť druhej dosky a vedomé rozhodnutie diagnostiky opustiť SMART.

Pri fyzickej strate napájania vlastné fail-safe agreement relé odpadne samo. Ak jedna doska zamrzne a jej výstup zostane v poslednom stave, zdravá druhá doska musí vedieť zrušiť SMART odobratím svojho agreement bez spolupráce zamrznutej dosky.

Nadradený princíp:

- na SMART sú potrebné dve nezávislé `ÁNO`;
- na BASIC stačí jedno `NIE`;
- na kritickú safety reakciu stačí jedna oprávnená kritická ochrana.

Strata jedného agreement sama osebe neznamená STOP. Znamená stratu SMART a prechod alebo sprístupnenie BASIC podľa schválenej hardvérovej či riadenej prechodovej logiky.

Návrat do SMART je možný až po obnovení oboch agreement a po dokončení sekvencie `BOOT → SELF_TEST → LINK/SYNC OK → STABLE → AGREEMENT`, vrátane schválenej anti-flap stabilizácie. Samotné obnovenie UART, heartbeat alebo jedného agreement nestačí.

Každá staršia formulácia, podľa ktorej môže SMART povoliť iba jedna doska, zrušenie SMART vyžaduje súhlas oboch alebo jedno agreement postačuje na návrat do SMART, je **HISTORICKÁ / NEAKTUÁLNA**.

#### Hardvérová strata procesora

Ak Mega alebo Uno/SUPERVISOR reálne zomrie, zamrzne, stratí napájanie alebo prestane držať svoje hardvérové povolenie, príslušné agreement/enable relé fyzicky odpadne, SMART sa hardvérovo zruší a mechanický fallback sprístupní BASIC. Na tento prechod sa nesmie čakať na softvérový dead-time, diagnostiku, UART ani potvrdenie druhého procesora. Bezpečný hardvérový fallback má prednosť pred komfortným prepnutím.

Hardvérové sprístupnenie BASIC stále samo nezapína BASIC_R1–R4. Ich konkrétne autonómne podmienky a bezpečnostné blokácie zostávajú samostatnou vrstvou.

#### Riadený prechod SMART → BASIC

Ak sú oba procesory živé a systém opúšťa SMART na základe diagnostiky, senzorov alebo iného schváleného dôvodu, prechod musí chrániť motory, relé a stykače:

`SMART výstupy OFF → fyzické odpadnutie/dobeh → dead-time → povolenie BASIC`

SMART a BASIC nesmú byť úmyselne súčasne aktívnymi riadiacimi zdrojmi rovnakej technológie. Dead-time a minimálne OFF časy budú určené osobitne podľa konkrétneho motora, čerpadla, relé alebo stykača a zostávajú **NÁVRH – TREBA FYZICKY OVERIŤ**.

#### Kontrolovaný návrat BASIC → SMART

Návrat do SMART je vždy riadený. Obnovenie procesora, komunikácie alebo prvý platný UART rámec ho nesmú vyvolať okamžite. Po recovery alebo resete musí systém zostať v BASIC počas nepretržitého stabilizačného obdobia.

Základná hodnota stabilizácie je schválená a v prvej agreement vrstve implementovaná ako presne **180 sekúnd nepretržite stabilného stavu**. Aktuálna implementácia vyžaduje platnú V5 linku, čerstvé vzdialené dáta a neprítomnosť kritického stavu druhej dosky; reset prirodzene začína nový interval od nuly. Rozšírenie o budúci plný self-test, RESET_LOCKOUT, LOW WATER, motorové prechody a ďalšie safety podmienky zatiaľ implementované nie je. Cieľová architektúra počas celého intervalu vyžaduje najmenej:

- obe dosky sú funkčné;
- komunikácia je stabilná;
- synchronizácia je dokončená;
- self-test a diagnostika sú v prípustnom stave;
- nevznikol nový reset ani nový dôvod na BASIC;
- obe strany sú pripravené udeliť SMART agreement.

Každá nová relevantná chyba počas stabilizačného intervalu vynuluje čas stabilizácie. Nový interval začne až po opätovnom dosiahnutí stabilného stavu. Toto pravidlo zabraňuje cyklu `SMART → BASIC → SMART → BASIC`, zbytočnému cvakaniu relé/stykačov a opakovanému štartovaniu motorov.

Po úspešnom stabilizačnom intervale sa návrat vykoná postupnosťou:

`BASIC výstupy OFF → dobeh/minimálny OFF čas → dead-time → SMART agreement → SMART`

#### Väzba na RESET a RESET_LOCKOUT

Úspešný reset jednej dosky neznamená automatický návrat do SMART. Resetovaný procesor musí dokončiť reinicializáciu, self-test, synchronizáciu a celý požadovaný stabilizačný interval. Pri vyčerpaní povoleného počtu resetov platí `RESET_LOCKOUT → ďalšie automatické resety zakázané → BASIC`, pokiaľ je BASIC bezpečný. `RESET_LOCKOUT ≠ STOP`.

#### Nadradenosť kritickej safety vrstvy

LOW WATER/chod nasucho a ďalšie neskôr schválené kritické fyzické ochrany sú nadradené celej prechodovej, anti-flap, resetovej a recovery logike. Sledujú ich nezávisle obe dosky a každá má samostatné právo vyvolať príslušnú bezpečnostnú reakciu bez súhlasu druhej.

Pri kritickej fyzickej podmienke sa nesmie čakať na približne 3-minútovú stabilizáciu, SMART/BASIC dead-time ani recovery procesora, ak by čakanie oneskorilo požadovanú ochranu. Kritická podmienka vyvolá nadradenú reakciu až STOP podľa definície konkrétnej ochrany.

Záväzné rozlíšenie:

- fyzický odpad procesora → hardvér okamžite zruší SMART → BASIC;
- živý systém sa rozhodne opustiť SMART → riadený SMART→BASIC prechod;
- BASIC→SMART → vždy synchronizácia, stabilizácia, agreement a ochranný prechod výstupov;
- kritická fyzická safety chyba → nadradená reakcia až STOP podľa príslušnej ochrany.

Každá staršia formulácia pripúšťajúca softvérové oneskorenie pri fyzickom odpadnutí procesora, okamžitý BASIC→SMART návrat alebo úmyselné súčasné riadenie rovnakej technológie zo SMART aj BASIC je **HISTORICKÁ / NEAKTUÁLNA**.

## TOTAL STOP – samostatná hardvérová motorová ochrana

**STATUS 26. 8. 2026: FYZICKY ZAPOJENÁ SÉRIOVÁ `COM–NC` MOTOROVÁ CESTA / MEGA D32 A UNO A0 COMMISSIONING PASS / AUTOMATICKÉ FAULT PODMIENKY NEIMPLEMENTOVANÉ.** TOTAL STOP je samostatná kritická hardvérová vrstva umiestnená pred spoločnou BASIC motorovou povoľovacou vetvou. Neodpája napájanie Mega, Uno, SD, HMI ani diagnostiky; fyzicky prerušuje iba spoločný prívod oboch motorových vetiev, aby procesory a BLACK BOX zostali živé a mohli zaznamenať udalosť.

Mega a Uno majú mať každý vlastný TOTAL STOP výstup a vlastné relé. Dva TOTAL STOP kontakty sú v motorovej povoľovacej ceste zapojené sériovo cez `COM–NC`:

- normálny stav: obe relé sú neaktivované a oba kontakty `COM–NC` vedú;
- kritická udalosť: oprávnená doska aktivuje svoje relé, kontakt prejde z NC na NO a fyzicky preruší motorovú vetvu;
- TOTAL STOP môže vyhlásiť jediná doska bez agreement a bez súhlasu druhej;
- aktivácia ktoréhokoľvek jedného relé alebo oboch relé preruší motorovú vetvu;
- zapojenie je zámerne **energize-to-trip**.

Smrť, strata napájania, reset alebo freeze Mega či Una sama osebe nie je TOTAL STOP. Pri neaktívnom alebo mŕtvom riadení zostane príslušné TOTAL STOP relé v pokojovom `COM–NC`. Porucha procesora sa rieši stratou SMART a BASIC fallbackom podľa tvrdej architektúry `PROCESSOR FAILURE → BASIC`. TOTAL STOP je aktívna reakcia na potvrdený kritický fyzický stav, nie reakcia na obyčajnú stratu procesora.

**FYZICKY POTVRDENÉ 26. 8. 2026:** dva samostatné jednokanálové H/L 5 V TOTAL STOP moduly sú fyzicky osadené a ich kontakty sú zapojené sériovo `COM–NC`. Mega autorita je fyzicky pripojená na D32 a napájaná z Mega power domain; Uno autorita je fyzicky pripojená na A0 a napájaná z Uno power domain. Commissioning test potvrdil, že oba piny ovládajú správne relé: `LOW → uvoľnené/COM–NC`, `HIGH → zopnuté/COM–NO`. Napájacie vetvy sa neparalelizujú. Automatické TOTAL STOP fault podmienky zatiaľ nie sú implementované. Existujúce vlastníctvo jedinej 16R dosky sa nemení.

Fyzicky zapojená spoločná motorová/povoľovacia cesta je:

    prívod spoločnej motorovej/povoľovacej vetvy
      → TOTAL_STOP_1 COM–NC
      → TOTAL_STOP_2 COM–NC
      → WAGO rozdelenie
          ├─ BASIC_R1 na 16R doske
          └─ BASIC_R2 na 16R doske

Za BASIC relé sú fyzicky potvrdené tieto kontaktné reťazce:

    BASIC_R1 COM–NO → MEGA_R9 COM → filtrácia
    BASIC_R2 COM–NO → MEGA_R10 COM → solár

Aktivácia ktoréhokoľvek jedného TOTAL STOP relé preto rozpojí spoločný prívod ešte pred WAGO rozdelením a zablokuje obe motorové vetvy. Softvérové mapovanie je už zosúladené s fyzickou realitou: `MEGA_R9 / D22 = filtrácia` a `MEGA_R10 / D23 = solár/chrlič`.

NO kontakt sa nepoužíva ako normálna motorová cesta. Oba moduly zostávajú `energize-to-trip`: pokojový stav vedie cez COM–NC, aktívny STOP energizuje cievku a odpojí NC cestu.

**SCHVÁLENÉ A FYZICKY ZAPOJENÉ PRAVIDLO NAPÁJACEJ NEZÁVISLOSTI:** Mega TOTAL STOP relé a Uno TOTAL STOP relé nie sú napájané zo spoločnej 5 V vetvy ani vzájomne neparalelizujú napájacie vetvy cez VCC modulov. Každý modul má vlastný prívod zo svojej procesorovej napájacej domény. Fyzické MCU signály D32/A0 aj aktivačný commissioning test sú potvrdené; automatická safety rozhodovacia logika zatiaľ nie je implementovaná.

### DRY RUN / NO WATER

Dôveryhodný a potvrdený signál neprítomnosti vody má právo vyvolať TOTAL STOP z jedinej dosky. Nečaká sa na zhodu oboch procesorov, agreement ani zhodu oboch sonarov. Pre XKC Safety V1 je zdrojom jeden spoločný XKC vedený dvoma samostatnými optočlenovými cestami; každá doska rozhoduje iba zo svojho vstupu po 5 000 ms súvislého LOW WATER. Ďalšie nezávislé fyzické dôkazy musia byť posúdené samostatne.

### Duálne sonary – budúci diagnostický model hladiny

MEGA_SONAR a UNO_SONAR sú rovnocenné; žiadny nemá automaticky nadradené slovo. Porovnáva sa kalibrovaná výsledná hladina po odpočítaní individuálneho montážneho offsetu, nie surová vzdialenosť od snímača:

- `Δ ≤ 2 cm`: `OK / ZHODA`;
- `Δ > 2 cm` až `Δ ≤ 4 cm`: `SUSPECT`;
- `Δ > 4 cm` počas troch po sebe idúcich platných porovnaní: `SENSOR_CONFLICT → DEGRADED`;
- jednorazové platné `Δ` približne nad 7–8 cm: okamžitý `SENSOR_CONFLICT → DEGRADED`;
- pri konflikte sa na kontrolu označia oba sonary; systém automaticky neurčí víťaza;
- návrat z `SENSOR_CONFLICT` je povolený až po piatich po sebe idúcich platných porovnaniach s `Δ ≤ 2 cm`;
- timeout, nemožná hodnota alebo hodnota mimo fyzikálnych mantinelov sa do série zhody ani konfliktu nezapočítava;
- samotný konflikt sonarov nie je dôvod na TOTAL STOP.

Tento model je budúci a nemení aktuálny stav: kým nebude hotová finálna vyrovnávacia nádoba, geometria, offsety a fyzická kalibrácia, oba sonary zostávajú iba monitorovacie a nerozhodujú o hladine, LOW WATER, úniku ani dopúšťaní.

### Výpadok jednej dosky a únik vody

Ak pred výpadkom existovala čerstvá potvrdená zhoda sonarov, preživšia doska môže za sprísnených podmienok použiť vlastný sonar. Vyžaduje sa posledná čerstvá zhoda, aktuálny platný lokálny údaj, splnené fyzikálne mantinely, časové potvrdenie alebo trend a podľa situácie ďalší nezávislý dôkaz. Za týchto podmienok môže preživšia doska pri potvrdenom kritickom úniku vyhlásiť TOTAL STOP sama. Presná platnosť poslednej zhody, leak threshold, trendové okno a požadovaná kombinácia dôkazov zostávajú `TBD – TREBA URČIŤ Z REÁLNYCH DÁT KOMORY`.

Ak možno únik bezpečne zvládnuť ventilmi alebo izoláciou konkrétnej vetvy, TOTAL STOP nemusí nasledovať. Ak je kritický únik potvrdený a ventilová logika ho nedokáže bezpečne zvládnuť, TOTAL STOP odreže motorovú vetvu. Rozhodovanie má podľa dostupného hardvéru používať viac platných nezávislých signálov; presná kombinácia zostáva `TBD`.

### Povinný fyzický test TOTAL STOP

Pred uvedením vrstvy do prevádzky sa musí potvrdiť:

1. oba TOTAL STOP kontakty v NC → motorová vetva priechodná;
2. aktivovaný iba Mega TOTAL STOP → motorová vetva prerušená;
3. aktivovaný iba Uno TOTAL STOP → motorová vetva prerušená;
4. aktivované oba TOTAL STOP → motorová vetva prerušená;
5. vo všetkých štyroch prípadoch zostávajú Mega, Uno, SD/HMI a diagnostika živé.

Otvorené body: latch a manuálny ACK po TOTAL STOP; automatické fault podmienky a ich nezávislé fyzické dôkazy; fyzické mantinely; platnosť poslednej sonarovej zhody; leak threshold a časové potvrdenie podľa budúcich reálnych dát. Piny TOTAL STOP sú už potvrdené ako Mega D32 a Uno A0.

## W1209 – dohľad nad nezávislým BASIC regulátorom

**STATUS: ROZSAH AUTORITY SCHVÁLENÝ; AUTOMATICKÝ ZÁSAH ZATIAĽ NEIMPLEMENTOVANÝ.** W1209 zostáva nezávislý BASIC teplotný regulátor s pevným setpointom 32,0 °C. Mega ani Uno ho v normálnej SMART prevádzke neriadia. Smrť supervisora nesmie sama odpojiť W1209 a pri strate Mega alebo Una má BASIC zostať funkčný.

Prevádzkovo bolo pozorované, že W1209 môže zamrznúť na starej teplote na minúty až približne hodinu. Power-cycle jeho 12 V vetvy ho dokáže okamžite obnoviť. Kritické riziko vzniká, ak zamrzne na nižšej hodnote a ďalej povoľuje ohrev nad nastavených 32,0 °C; reálne bolo pozorované obnovenie až približne pri 33,2 °C.

Záväzné diagnostické pravidlo je:

> **W1209 sa neposudzuje podľa toho, čo tvrdí jeho displej, ale podľa výsledku: reálna teplota bazéna + potvrdené reálne dodávanie tepla.**

### Schválený finálny rozsah autority W1209 supervision

**W1209_FACKOVAC je dohľad živej Mega nad autonómnym BASIC režimom. Autoritu na reset alebo dočasné odpojenie W1209 má iba pri `SYSTEM_MODE == BASIC`. V `SMART`, `DEGRADED` ani `STOP` túto autoritu nepoužíva.**

- `SMART`: D33 zostáva `LOW / COM–NC`; W1209 supervision nesmie zasahovať.
- `DEGRADED`: D33 zostáva `LOW / COM–NC`; W1209 supervision nesmie zasahovať.
- `BASIC`: ak Mega stále funkčne beží, môže byť iba supervisorom autonómnej BASIC vrstvy. Normálnu filtráciu ani solárnu vetvu v tomto režime neriadi. Filtrácia pokračuje podľa vlastného BASIC časovača/logiky a W1209 podľa vlastných K1/K2 nastavení.
- `STOP`: W1209 supervision nevykonáva vlastné resetovacie ani recovery rozhodovanie. Správanie už rozpojenej W1209 vetvy pri prechode do STOP zostáva `TBD` a nesmie sa domyslieť bez schválenia.

Autorita W1209 supervision sa nesmie odvodzovať zo SMART výstupov ani príkazov `solarZapnuty`, `MEGA_R10/D23` alebo `chrlicManualAktivny`, pretože počas BASIC tieto signály nie sú riadiacou autoritou BASIC vetvy.

Preferovaný teplotný dôkaz používa súčasne platné, čerstvé a vzájomne zosúladené páry `MEGA_T1 + UNO_T1` pre `T_POOL` a `MEGA_T2 + UNO_T2` pre `T_SOLAR_OUT`. Budúci model musí evidovať najmenej logické stavy `T1_PAIR_CONFIRMED`, `T2_PAIR_CONFIRMED`, `last_T1_pair_confirmed_time` a `last_T2_pair_confirmed_time`.

Ak po dlhšie stabilnej potvrdenej zhode vypadne jedna doska alebo jedna senzorová cesta, zostávajúci T1 alebo T2 môže byť na obmedzený čas použitý ako dôveryhodný fallback. Ak pred výpadkom potvrdená zhoda neexistovala, bola už zastaraná alebo bol pár v konflikte, samostatný senzor sa nesmie automaticky povýšiť na dôveryhodný fallback pre W1209 zásah. Toto pravidlo platí rovnako pre T1 aj T2. Presná tolerancia T1, tolerancia T2, minimálny čas predchádzajúcej zhody a maximálny čas fallbacku zostávajú `TBD` a zatiaľ sa neimplementujú.

Budúce potvrdenie reálneho BASIC kúrenia má najprv vyhodnocovať dôveryhodné `T_POOL`, dôveryhodné `T_SOLAR_OUT`, rozdiel `T_SOLAR_OUT − T_POOL`, trend v čase, stabilitu hodnôt, validity a históriu zhody redundantných párov. Až reálne dáta rozhodnú, či táto kombinácia sama postačuje. Flow senzor alebo iný fyzický dôkaz zostáva možný budúci doplnok `TBD`, nie povinná vopred určená podmienka.

Aktuálny Mega kód už obsahuje základný autoritatívny `SYSTEM_MODE`, ale stále nemá schválený algoritmus teplotného/trendového dôkazu BASIC kúrenia. W1209 tento nový režim zatiaľ nijako nepoužíva: produkčný D33 po boote iba bezpečne inicializuje `LOW / COM–NC` a automaticky ho neaktivuje. Budúca jediná režimová brána W1209 bude `systemMode == MODE_BASIC`, ale automatický neblokujúci W1209 stavový automat možno aktivovať až po samostatnom schválení dôkazového modelu a časovaní.

Vysoká teplota bazéna sama osebe nie je dôkaz poruchy W1209. Pri vonkajšej teplote približne 40–44 °C a silnom slnku bolo pozorované pasívne prehriatie bazéna nad 34 °C aj bez aktívneho solárneho ohrevu. Ak `T_POOL` prekročí limit, ale prietok alebo iný nezávislý dôkaz nepotvrdzuje aktívne dodávanie tepla BASIC/W1209 vetvou, W1209 sa neresetuje ani neodpája. Stav sa eviduje iba ako `POOL_OVER_TEMP_PASSIVE` / prirodzené prehriatie.

Budúce potvrdenie aktívneho BASIC ohrevu vychádza prednostne z dôveryhodných redundantných alebo časovo obmedzených potvrdených fallback hodnôt T1/T2, rozdielu `T_SOLAR_OUT − T_POOL`, trendu a stability. Flow senzor môže byť neskôr doplnený, ak reálne dáta ukážu, že teplotný model nestačí. SMART príkazy a SMART reléové výstupy nie sú dôkazom autority ani chodu BASIC vetvy. Presná kombinácia, validity, časové okná a prahy sú `TBD` do reálneho zapojenia a testov.

### Navrhovaná ochranná postupnosť W1209

1. `T_POOL ≤ 32,5 °C`: bez zásahu do W1209.
2. `T_POOL > 32,5 °C` a zároveň potvrdené aktívne dodávanie tepla W1209/solárnou vetvou: vykonať jeden power-cycle samostatnej 12 V vetvy W1209 a po jeho boote znovu vyhodnotiť reálny výsledok.
3. Ak sa po power-cycle aktívne dodávanie tepla zastaví: W1209 sa považuje za obnovený/OK a ďalší zásah nie je potrebný.
4. Ak teplota po resete pokračuje približne k 33,0 °C alebo vyššie a aktívne dodávanie tepla zostáva potvrdené: `W1209_FAULT / TEMP_LIMIT_PROTECTION`; odpojiť 12 V napájaciu vetvu W1209, aby už nemohol povoľovať ohrev.
5. W1209 možno znovu povoliť až po prirodzenom ochladení bazéna, orientačne pod približne 31,5–31,8 °C.
6. Pri `T_POOL` približne 33–34+ °C bez potvrdeného aktívneho ohrevu: žiadny reset ani odpojenie W1209; iba `POOL_OVER_TEMP_PASSIVE`.

Hranice 32,5 °C, približne 33,0 °C a recovery 31,5–31,8 °C sú pracovný návrh ochrany a musia sa fyzicky overiť. Rovnako `TBD` zostávajú čas power-cycle, boot delay, počet povolených zásahov, potvrdenie aktívneho ohrevu a finálne recovery pravidlá.

**FYZICKY ZAPOJENÁ 12 V SUPERVISION CESTA 25. 8. 2026:**

    24 V
      → DC/DC buck 12 V
      → BASIC_R3 (fyzický kanál 3 jedinej 16R dosky), COM–NC
      → samostatný jednokanálový H/L 5 V modul „W1209 fackovač“, COM–NC
      → +12 V W1209

Pokojový stav oboch NC kontaktov napája W1209. Samostatný supervision modul je fyzicky pripojený na Mega D33 a napájaný z Mega power domain. Commissioning test potvrdil správny modul a polaritu `LOW → uvoľnené/COM–NC`, `HIGH → zopnuté/COM–NO`. Produkčný Mega kód po odstránení commissioning cyklu inicializuje D33 ešte pred/pri nastavení `OUTPUT` bezpečne na `LOW` a automaticky ho neprepína. Budúce zopnutie v autorizovanom BASIC režime má iba rozpojiť 12 V napájanie W1209 a vykonať power-cycle alebo ochranné odstavenie. Automatická fault logika, čas power-cycle, boot delay, správanie pri prechode do STOP a finálne recovery podmienky zostávajú neimplementované/TBD; mŕtvy Mega nesmie W1209 zbytočne odstaviť.

**FYZICKY ČIASTOČNE ZAPOJENÁ 230 V KONTAKTNÁ CESTA W1209:**

    BASIC_R1 NC
      → COM1 W1209
      → K1 (maximálna teplota bazéna)
      → COM2 W1209
      → K2 (solárna teplotná podmienka)
      → finálny výstup NEZAPOJENÝ

Vstup z `BASIC_R1 NC` do `COM1` a prepoj `K1 → COM2 → K2` sú fyzicky hotové. Za K2 zatiaľ nie je pripojený finálny vývod a jeho budúci cieľ sa nesmie domýšľať. Stav: **`W1209 INPUT/K1→COM2/K2 PHYSICALLY WIRED / FINAL K2 OUTPUT NOT YET CONNECTED`**.

Ochrana W1209 nie je TOTAL STOP. Izoluje alebo resetuje konkrétny BASIC regulátor/ohrevnú vetvu. TOTAL STOP zostáva oddelená nadradená hardvérová motorová ochrana pre potvrdený dry-run, kritický únik a ďalšie osobitne schválené fyzické stavy.

### Pamäť, BLACK BOX a komunikácia

- Voľná SRAM Una je bezpečnostná rezerva pre watchdog, reset Mega, LOW WATER, BASIC, kritické vstupy, komunikáciu a SD čiernu skrinku. Nesmie sa automaticky zapĺňať duplicitnou diagnostikou. Preferujú sa jednoduché stavy, malé pevné buffre, minimum duplicitných údajov a žiadne `String`.
- SD na Uno je **BLACK BOX**, nie druhý analytický procesor. Má prednostne uchovať lokálne senzory, stav linky, posledný relevantný výsledok Mega, posledný známy režim, watchdog/reset udalosti, LOW WATER, kritické blokácie a prechody režimov. Detailnú analýzu vykoná následne Mega, PC alebo človek.
- Mega↔Uno UART prenáša merania Una do Mega, potrebné hotové výsledky Mega do Una, dostupnosť procesorov a nekritické údaje ako čas. Uno nemá držať kompletnú kópiu diagnostického modelu Mega. UART nie je safety heartbeat ani jediný prvok povoľujúci SMART; budúci fyzický watchdog/HW agreement je samostatná vrstva.
- Čím závažnejší režim, tým jednoduchšie rozhodovanie: SMART môže byť komplexný, DEGRADED používa schválené fallbacky, BASIC jednoduchú autonómnu logiku a STOP čo najpriamejšie nezávislé podmienky.

### Povinná kontrola každej väčšej zmeny

Pred implementáciou sa musí najprv položiť otázka: **Je funkcia na Uno nevyhnutná pre SUPERVISOR, SAFETY, BASIC alebo BLACK BOX?** Ak nie, patrí prednostne na Mega. Následne sa musí overiť: či slúži SMART, DEGRADED, BASIC alebo STOP; či Uno zbytočne neopakuje prácu Mega; či BASIC/SAFETY nezávisí viac od Mega alebo UART; či sa zbytočne nezmenšuje pamäťová rezerva Una; a či zostáva zachovaná hierarchia SMART → DEGRADED → BASIC → STOP. Konflikt s týmito bodmi vyžaduje výslovné rozhodnutie pred zmenou.

## Úlohy riadiacich jednotiek

### Mega – SMART

Mega 2560 je hlavný SMART kontrolér a zdroj pravdy pre aktuálnu reguláciu. Meria teploty a prostredie, používa RTC, LCD a tlačidlá, riadi aktuálny solár a filtráciu a posiela telemetriu ESP8266. AHT10 je fyzicky namontovaný vonkajší senzor umiestnený v tieni, funkčný a prevádzkovo overený. Mega z neho dostáva reálne hodnoty vonkajšej teploty a relatívnej vlhkosti vo formáte `VONKU: xx.x C RH: xx.x %`. Jeho aktuálna úloha je iba diagnostická/informačná; nevstupuje do regulácie ani safety.

Mega má dočasnú ľahkú diagnostiku časovania posledného dokončeného procesného cyklu a jeho častí cez rollover-safe rozdiely `micros()`. V existujúcom 10-sekundovom DIAG bloku zobrazuje LAST/MAX pre celý cyklus, vstupy/teploty, DS18B20, AHT10, akumulovaný RTC I²C čas, spracovanie, bezpečnosť, reguláciu a výstupy spolu s počtami cyklov nad 1 000 ms a 1 500 ms. Diagnostika nič neriadi a nemení SYSTEM, agreement ani safety. Procesný interval zostáva 2 000 ms; účelom je fyzicky zmerať časovú rezervu pred prípadným osobitne schváleným budúcim testom 1 000 ms.

Po fyzickom A/B/A audite Mega 1-Wire zbernice je produkčná DS obsluha deterministická a nespolieha sa na `isConversionComplete()`. Pri boote zostávajú všetky Mega DS hodnoty neplatné, spustí sa jedna broadcast 12-bitová konverzia a loop pokračuje. V nasledujúcom 2 s procesnom cykle, po uplynutí minimálne 750 ms, sa T1/T2/T3/T4/TBOX najprv načítajú do jedného lokálneho snapshotu, potom sa naraz zverejnia a okamžite sa spustí ďalšia broadcast konverzia. Ak čas ešte neuplynul, posledný snapshot sa nemení a nevytvára sa falošná chyba. MEGA_TBOX zostáva monitor-only. Timing `LAST` blok sa vypisuje až po uzavretí celého meraného cyklu, takže nemieša starý `CYCLE` s aktuálnym `TEMP/DS`.

**FYZICKY OVERENÉ 23. 8. 2026:** produkčná deterministická pipeline `START → neskorší READ snapshotu` bola overená v normálnom stave, pri odpojení G2/MEGA_T3 aj po jeho opätovnom pripojení. V normálnom stave bol DS čas približne 64 ms, celý procesný cyklus približne 376–395 ms a `OVER_1000MS=0`. Pri odpojenom G2 prešla MEGA_T3 na `-127 °C / T3_CHYBA`, čerstvý `UNO_T3_REMOTE_FALLBACK` zostal funkčný, `SYSTEM=DEGRADED`, bez STOP, a `SOLAR control` zostal validný. DS čas zostal približne 64 ms, cyklus približne 378–396 ms a `OVER_1000MS=0`; pôvodné opakované približne 741 ms DS čakanie bolo odstránené. Po pripojení G2 sa automaticky obnovili `MEGA_T3_PRIMARY`, `SYSTEM=OK` a `PROBLEM=NONE`. Timing `LAST` snapshot bol fyzicky potvrdený ako konzistentný: `CYCLE` už nie je menší než `TEMP/DS` z toho istého reportu. Počas testu pribudli dve jednotlivé udalosti `REPLY_TIMEOUT_COUNT`, ale `CRC_FAIL=0`, `FRAME_INVALID=0`, `LINK_TIMEOUT_COUNT=0`, `SEQ_GAP_COUNT=0` a link zostal `OK`; ide iba o pozorovanie a V4 ani Uno timing sa nemenia.

Pôvodný produkčný problém DS busy-pollingu cez `isConversionComplete()` je týmto považovaný za vyriešený deterministickou pipeline celej Mega 1-Wire zbernice. Samostatný `DS18B20_TIMING_DIAG` zostáva zachovaný iba ako historický a servisný bench nástroj, nie ako súčasť produkčnej regulácie.

LCD 20×4 na spoločnej I²C zbernici používa štyri neblokujúco rotované obrazovky: `PREVÁDZKA`, `PROSTREDIE`, `MANUÁLNE REŽIMY` a `PORUCHY / DIAGNOSTIKA`. Bežný interval každej stránky je 5 s. Nadradený používateľský stav sa odvodzuje z autoritatívneho `SystemMode`; `MODE_STOP` sa nezobrazuje ako `SYS:OK` ani `SYSTEM: OK`. Lokálny `megaXkcTrip` má diagnostickú STOP prioritu aj pred najbližším resolver cyklom a poruchová stránka zobrazuje `XKC LOW WATER STOP`; všeobecný STOP bez iného problému zobrazí `TOTAL STOP AKTIVNY`, nie `ZIADNA AKTIVNA`. Ak existuje aktuálny problém z rovnakého spoločného zoznamu podmienok, ktorý vytvára USB Serial súhrn `PROBLEM:`, stránka porúch sa pri svojom príchode drží 30 s a potom sa rotácia vždy vráti na prvú stránku. Agreement vypnutý počas normálneho priebehu `SMART_STABLE=n/180s` nie je problém a 30-sekundové držanie nevyvolá. Obrazovka nastavovania teploty má naďalej nadradenú prioritu. Rotácia používa `millis()`, bez `delay()`, `String` a samostatnej paralelnej režimovej autority.

Hlavná doska je fyzicky namontovaná kombinovaná **Arduino Mega + WiFi, Techfun, katalógové číslo IOT382**. Podľa produktovej dokumentácie obsahuje ATmega2560 (16 MHz, 256 kB Flash, 8 kB SRAM, 4 kB EEPROM), samostatný 32-bitový ESP8266, prevodník CH340G, micro-USB a osem DIP prepínačov pre smerovanie komunikácie medzi Mega, ESP8266 a USB/CH340. ESP8266 je samostatný procesor použiteľný na vlastný aplikačný firmvér, nie iba pasívny Wi-Fi modem.

Mega HY-SRF05 je fyzicky a funkčne overený ako samostatný monitorovací vstup: D38 TRIG a D39 ECHO. Izolovaný diagnostický test nameral približne 19,3–19,8 cm. D39 musí byť obyčajný `INPUT` bez interného pull-upu. Hlavné meranie používa neblokujúci stavový automat cez `micros()`, bez `delay()` a `pulseIn()`, v intervale 250 ms, s timeoutom 30 ms, rozsahom 2–450 cm a stavmi `MEGA_SONAR_OK`, `MEGA_SONAR_TIMEOUT` a `MEGA_SONAR_CHYBA`. Chyba sonaru nevstupuje do `system_OK`, regulácie ani safety logiky a nesmie zastaviť ostatné funkcie Mega. D40–D43 zostávajú voľná rezerva.

Spoločný XKC-Y25-NPN je fyzicky pripojený cez dve samostatné optočlenové cesty na Mega D30 a Uno A2. XKC Safety V1 dáva každej doske lokálne rozhodovanie: 5 s potvrdený LOW WATER aktivuje vlastný TOTAL STOP a 10 s súvislého WATER trip zruší; UART ani druhá doska nie sú podmienkou. Naďalej plánované zostávajú samostatný safety heartbeat, vlastné LOW WATER relé FIL/SOLAR a lux senzor. Starý softvérový heartbeat Mega D44 bol ako fyzicky nepoužitý zvyšok odstránený a D44 je voľný. Skladovaný BH1750 je kandidát lux senzora, ale jeho pin, I²C adresa a prahy zatiaľ nie sú pridelené. Lux má byť iba doplnkový vstup solárnej diagnostiky; nesmie byť safety dôkaz ani sám definitívne povoliť solár.

### Uno – bezpečnostná/BASIC jednotka

Uno má fungovať nezávisle od ESP, Wi-Fi a T20. Aktuálny MONITOR-ONLY kód na D2 obsluhuje `UNO_T1`, nový dlhokáblový `UNO_T2`, preklasifikovaný krátkokáblový `UNO_T3` a `UNO_TBOX`, vlastný HY-SRF05 na D3/D4 a MicroSD na D10–D13. Všetky štyri teplotné role majú fyzicky potvrdenú pevnú ROM, samostatné meranie, validitu/chybový stav a spoločný neblokujúci 5 300 ms merací cyklus; OneWire recovery zahŕňa všetky štyri senzory. Fyzický commissioning test našiel všetky štyri DS18B20 a potvrdil ich stav `OK`, preto bol jednorazový startup ROM scanner následne odstránený. Spoločná 1-Wire zbernica používa jeden fyzicky osadený pull-up 4,7 kΩ; pri presune sa zachová a ďalší paralelný sa nepridáva. Napätie hornej strany pull-upu zostáva `NEOVERENÉ`. UNO_T1/T2/T3/TBOX sa naďalej zobrazujú v kompaktnom 10 s Serial výpise ako hodnota + `OK/ERR` a zapisujú do denného prevádzkového logu. MicroSD logger zapisuje každých 60 s; chyba SD nesmie ovplyvniť meranie ani ostatné funkcie a obnovu skúša raz za 60 s.

Uno už používa korektnú časovanú asynchrónnu pipeline: jeden broadcast START, návrat do loop a READ celého snapshotu až po pevnom 800 ms bezpečnom čase. Nepoužíva `isConversionComplete()` ako autoritu pripravenosti. Prevádzkový servisný interval zostáva 5 300 ms, recovery 5 000 ms a nepribudla žiadna ďalšia periodická OneWire operácia.

Fyzický A/B test potvrdil časovú kolíziu pravidelnej UNO OneWire/DS18B20 komunikácie so SoftwareSerial RX V3. V pôvodnom približne sekundovom režime vzniklo na Uno `CRC_FAIL=50`, `FRAME_INVALID=17`, `SEQ_GAP=64` a Mega zaznamenala `REPLY_TIMEOUT_COUNT=88`. Po diagnostickom zmrazení periodickej OneWire komunikácie prešlo minimálne 2 378 rámcov s nulovými `CRC_FAIL`, `FRAME_INVALID`, `SEQ_GAP`, `LINK_TIMEOUT` aj Mega `REPLY_TIMEOUT_COUNT`. Diagnostický freeze bol po teste odstránený a priebežné asynchrónne meranie aj recovery boli obnovené. Aktuálny prevádzkový interval UNO DS18B20 je 5 300 ms namiesto približne 1 s. Hodnota 5 300 ms sa zámerne vyhýba presnému pomeru 5:1 voči približne sekundovému Mega V3 requestu, aby sa vzájomná fáza priebežne posúvala a nevznikal stabilný opakovaný súbeh. Toto opatrenie neodstraňuje fyzikálnu príčinu krátkych interrupt-off úsekov OneWire, iba výrazne znižuje pravdepodobnosť ich prekrytia s Mega→Uno SoftwareSerial RX. OneWire recovery interval zostáva samostatne 5 000 ms a SMART link/stale timeout 10 s poskytuje rezervu proti jednotlivým strateným rámcom.

Mega↔Uno UART je aktuálne priama neinvertovaná TTL diagnostická/komunikačná linka pri 38400 Bd: Mega Serial2 D16/TX2 → sériový 10 kΩ → Uno D7/SoftwareSerial RX a Uno D8/SoftwareSerial TX → sériový 10 kΩ → Mega D17/RX2. Dosky majú spoločnú GND, nemajú medzi sebou prepojené +5 V a PC817 nie sú v dátovej ceste. Aktuálny kód používa binárny protokol V5 s pevnými rámcami 22 B Uno→Mega a 24 B Mega→Uno. Každý rámec obsahuje magic `BA 5E`, verziu, typ, pevnú dĺžku, 16-bitovú sekvenciu a CRC-8/ATM. V5 zachováva celý V4 layout a iba prideľuje dva predtým rezervované bity commissioning telemetrii XKC. Link timeout a stale limit zostávajú 10 s. UART stále nie je finálnym nezávislým safety heartbeat prvkom.

Aktuálne schválené a fyzicky overené časovanie zostáva **MASTER→REPLY**. Mega približne raz za sekundu odošle svoj 24 B rámec; Uno ho úplne prijme a overí a až potom odošle 22 B odpoveď. Mega počas neblokujúceho 500 ms okna nezačína ďalší rámec. CRC algoritmus, sekvencie, 10 s link/stale timeout a 180 s agreement stabilizácia zostávajú zachované. Bench test V4 z 22. 8. 2026 prebehol bez `CRC_FAIL`, `FRAME_INVALID`, `LINK_TIMEOUT` a `SEQ_GAP`; V5 XKC prenos oboma smermi je fyzicky overený commissioning testom 1. 9. 2026.

### Mega↔Uno UART – aktuálna fyzická vrstva

**AKTUÁLNY STAV:** priame neinvertované TTL UART prepojenie bez PC817 v dátovej ceste:

- Mega D16/TX2 → sériový 10 kΩ → Uno D7/SoftwareSerial RX;
- Uno D8/SoftwareSerial TX → sériový 10 kΩ → Mega D17/RX2;
- Mega GND ↔ Uno GND;
- +5 V medzi doskami nie je prepojené;
- Uno používa `SoftwareSerial megaLinkSerial(MEGA_LINK_RX_PIN, MEGA_LINK_TX_PIN, false);`;
- obe strany používajú 38400 Bd.

Sériový odpor 10 kΩ v každom dátovom smere je zámerná ochrana proti phantom/backfeed napájaniu pri vypnutej druhej doske. Nejde o galvanické oddelenie; spoločná GND je súčasťou aktuálneho priameho TTL zapojenia.

Komunikačná vrstva používa V5, MASTER→REPLY, 24 B Mega→Uno, 22 B Uno→Mega, CRC-8/ATM, 16-bitovú sekvenciu, 500 ms reply okno, 10 s link/stale timeout a 180 s agreement stabilizáciu. V5 nemení fyzickú UART cestu ani safety autoritu; TOTAL STOP a BASIC/SMART resolver zostávajú oddelené.

**HISTORICKÝ / NEAKTUÁLNY STAV:** zapojenie cez dva kanály HY-M154/PC817, oddelené GND, lokálne 4,7 kΩ pull-upy, 9600 Bd a inverse `SoftwareSerial(..., true)` bolo predchádzajúcim commissioning riešením. PC817 testy a vtedajšie výsledky zostávajú historickým záznamom, ale nesmú sa používať ako opis dnešnej kabeláže alebo konfigurácie.
### 16R relay board – residual backfeed do Mega

**STATUS: KNOWN / PHYSICALLY CONFIRMED / FUNCTIONALLY ISOLATED / ACCEPTED – 23. 8. 2026.** Použitá je [16-kanálová opticky oddelená 5 V reléová doska](https://www.drotik-elektro.sk/arduino-platforma/1278-modul-16-rele-s-optickym-oddelenim-5v-pre-arduino.html).

Po odstránení UART phantom/backfeedu sa fyzicky preukázala druhá, nezávislá spätná napájacia cesta do nenapájanej Mega:

- Mega vlastné napájanie OFF, Uno vlastné napájanie OFF, ostatná zostava napájaná: Mega 5 V vetva približne `1,58 V`;
- so zaťažovacím rezistorom 1 kΩ medzi Mega 5 V a GND zostalo približne `1,445 V`, takže nejde iba o zanedbateľný vysokoimpedančný únik;
- pri napájanej 16R doske bolo na Mega približne `1,58 V`;
- po odpojení napájania 16R dosky klesla Mega 5 V vetva na `0 V`.

Zdroj residual backfeedu je tým fyzicky lokalizovaný na 16R reléovú dosku a jej vstupnú väzbu na Mega. Presná vnútorná elektrická cesta dosky nebola rozoberaná a zostáva pracovnou diagnózou; pravdepodobnou cestou je vstupná elektronika aktívne-LOW kanálov a GPIO nenapájanej Mega.

Pri strate vlastného napájania Mega sa fyzické SMART kanály R5–R16 na napájanej 16R doske reálne aktivujú/zopnú. Nejde iba o rozsvietenie signalizačných LED. Mega pritom zostáva funkčne mŕtva: nebootuje, nevykonáva program a nedrží SMART agreement. Tento stav bol fyzicky pozorovaný približne jednu hodinu bez spontánneho oživenia Mega alebo ďalšieho nového nestabilného prejavu.

Samotné zopnutie R5–R16 však nemá právo spustiť technológiu. R5–R16 sú podradené SMART kanály a ich funkčný účinok je blokovaný nadradenou BASIC/SMART povoľovacou cestou a agreement/watchdog mechanikou. R1/R2 sú fyzicky zapojené do príslušných motorových FIL/SOLAR ciest; R3 je fyzicky použitý v 12 V napájacej ceste W1209 a R4 zostáva rezervovaný. Strata Mega znamená stratu jej SMART agreement, takže bez platného nadradeného povolenia zopnuté R5–R16 nevytvoria aktívnu SMART prevádzku.

Po návrate napájania Mega najprv bootuje a inicializuje SMART výstupy do OFF. Agreement sa neobnoví prvým rámcom: musí sa obnoviť platná linka a stav a následne prebehnúť existujúca 180 s stabilizácia. Až potom môže byť SMART znovu povolený.

Residual backfeed sa v aktuálnej zostave ďalej neodstraňuje. Ďalšie galvanické oddelenie celej skupiny R5–R16 alebo prestavba napájacej/vstupnej architektúry by výrazne zvýšili množstvo optočlenov, kabeláže a zložitosť bez aktuálne preukázaného funkčného alebo bezpečnostného prínosu. Znovu sa otvorí iba vtedy, ak budúci fyzický test preukáže nový funkčný alebo safety dopad. Platí projektové pravidlo: „Neopravovať fabriku, keď stačí odstrániť konkrétny problém.“

Tvrdé rozlíšenie:

- `UART PHANTOM/BACKFEED` – HISTORICKÉ: spôsoboval čiastočné ožitie Una a cvakanie relé; dočasne bol odstránený PC817 izoláciou. Aktuálne PC817 nie sú v UART dátovej ceste a ochranu proti backfeedu tvoria sériové 10 kΩ odpory bez spoločného +5 V;
- `16R RESIDUAL BACKFEED DO MEGA`: približne 1,58 V, pri mŕtvej Mega zopína R5–R16, ale jeho funkčný účinok je nadradene izolovaný; stav je akceptovaný bez ďalšej prestavby.

Pri commissioning teste štvorice UNO DS18B20 zostala V3 linka `OK`: `CRC_FAIL=1`, `FRAME_INVALID=0`, `LINK_TIMEOUT=0`, `SEQ_GAP=2`; po štartovacích udalostiach sa čítače ďalej nezvyšovali. Ojedinelé CRC/sequence straty sú kompatibilné so známym časovacím javom OneWire↔SoftwareSerial, nevyvolali link timeout ani zrušenie agreement. Stabilizácia 180 s úspešne skončila a `UNO_AGREEMENT=ON`. Tento výsledok nie je dôvodom meniť 5 300 ms OneWire interval, V3 časovanie ani komunikačnú architektúru.

Pôvodná verzia používala dva nezávislé približne sekundové TX timery. Rámce sa mohli prekrývať: Mega vysielala hardvérovým UART, zatiaľ čo Uno mohlo cez SoftwareSerial vysielať práve počas príchodu nového Mega rámca. Fyzickým prejavom bola asymetria: Mega prijímala Uno rámce bez chýb, ale Uno evidovalo `CRC_FAIL=19` a `SEQ_GAP=12`. Toto prekrytie/asymetria SoftwareSerial TX/RX je **FYZICKY POTVRDENÁ KOREŇOVÁ PRÍČINA** pôvodných V3 chýb. Nezávislý periodický TX oboch dosiek je **HISTORICKÝ / NEAKTUÁLNY** a nesmie sa obnoviť.

Po prechode na postupnosť `Mega MASTER rámec → úplný príjem a validácia na Uno → Uno REPLY → ticho do ďalšieho Mega cyklu` fyzický test na oboch stranách potvrdil `CRC_FAIL=0`, `FRAME_INVALID=0`, `LINK_TIMEOUT=0`, `SEQ_GAP=0`; `RX_FRAME_OK` rastie približne raz za sekundu. MASTER→REPLY preto predstavuje aktuálnu referenčnú komunikačnú architektúru.

**USB servisné odporúčanie:** staršie podozrenie, že USB hub je hlavnou príčinou V3 chýb, sa nepotvrdilo; koreňovou príčinou bolo prekrytie nezávislých TX timerov a asymetria SoftwareSerial. Pri programovaní a USB Serial diagnostike sa napriek tomu odporúča priamy USB port základnej dosky PC, pretože USB hub môže testovacie prostredie zhoršiť. USB hub však nie je potvrdenou koreňovou príčinou pôvodných `CRC_FAIL/SEQ_GAP`.

### NÁVRH DO BUDÚCNA – UNO → MEGA #2

**STATUS: NÁVRH DO BUDÚCNA – NEIMPLEMENTOVAŤ TERAZ.** Tento návrh nemení dnešnú architektúru, fyzickú zostavu, vlastníctvo funkcií, piny ani programy.

Pri prechode na väčší bazén približne 18–20 m³ alebo po získaní druhej fyzickej dosky Mega 2560 možno zvážiť nahradenie Uno doskou Mega #2.

- **Mega #1 – MASTER / CONTROL:** hlavné riadenie bazéna, SMART regulácia, vlastné prioritné senzory a akčné členy a pridelená časť diagnostiky.
- **Mega #2 – SUPERVISOR / SAFETY / BASIC / BLACK BOX:** prevezme dnešnú úlohu Uno, vlastné prioritné senzory, BASIC, watchdog/reset, agreement a BLACK BOX. Vďaka väčšej pamäti môže prevziať aj ďalšiu jasne pridelenú časť diagnostiky a odľahčiť Mega #1.

Budúci princíp rozdelenia diagnostiky: ak majú obe dosky potrebné rovnaké vstupné údaje, rovnaký nekritický výpočet sa nemá bezdôvodne vykonávať na oboch. Každá diagnostická funkcia má mať jasného vlastníka a medzi procesormi sa má prenášať prednostne výsledný logický stav, nie dve kompletné kópie rovnakého výpočtu.

Kritické safety funkcie sú výnimka. Každá doska musí nezávisle vykonať jednoduché lokálne overenie potrebné pre vlastné bezpečnostné rozhodnutie. Zostáva zachované:

- bežná diagnostika → rozdeliť medzi procesory;
- každá funkcia → jeden jasný vlastník;
- kritická ochrana → nezávislé overenie oboma doskami;
- SMART → dve nezávislé agreement `ÁNO`.

Prípadná migrácia nemá znamenať návrh systému od začiatku. Má zachovať súčasnú filozofiu, protokolové princípy a role, využiť väčšiu pamäť Mega #2 a podľa možností nahradiť SoftwareSerial hardvérovým UART. Konkrétna doska, UART, piny, rozdelenie diagnostických funkcií, migrácia BLACK BOX a zmeny protokolu zostávajú `NEURČENÉ` až do osobitného schválenia.

Uno číta XKC lokálne na A2 a agreement riadi osobitne na D9. XKC Safety V1 po 5 s súvislého lokálneho LOW WATER nastaví `UNO_XKC_TRIP` a aktivuje A0 aj bez Mega/UART/agreement; po 10 s súvislého WATER ho zruší. Prenášaný Mega XKC stav a konflikt zostávajú iba diagnostické. Samostatný heartbeat, ACK/reset, ďalšie závažné poruchy a autonómna BASIC logika zostávajú predmetom osobitného schválenia.

### ESP-01S pôvodne pri Uno

ESP-01S bol fyzicky odstránený zo zostavy Uno. Modul je funkčný a uložený SKLADOM; jeho budúce použitie je `NEURČENÉ`. UNO sketch už neobsahuje podporu ESP/Wi-Fi, AT príkazy, SSID/heslo, IP diagnostiku ani recovery. D5 a D6 sú voľné. Samostatná fyzicky overená 3,30 V vetva už nie je aktívnou súčasťou Uno zostavy a jej ďalšie použitie je `NEURČENÉ`.

### ESP8266 – Wi-Fi/web HMI

ESP prijíma telemetriu Mega cez UART 115200 baud, poskytuje lokálne web HMI a preposiela povolené príkazy Mega. Neovláda priamo relé a nemá vlastnú regulačnú logiku. Bezpečnosť nesmie závisieť od ESP, Wi-Fi ani T20.

## MEGA+WIFI – DIP PREPÍNAČE A UART REŽIMY

### Istota podkladov

- **POTVRDENÉ PRE IOT382:** identita dosky a vyššie uvedené technické parametre podľa [Techfun](https://techfun.sk/produkt/arduino-mega-wifi/).
- **AKTUÁLNE POTVRDENÉ KÓDOM:** Mega USB `Serial0` aj Uno hardvérový USB `Serial` používajú 115200 Bd. Mega↔Uno V5 zostáva oddelene na Mega `Serial2` D16/D17 ↔ Uno neinvertovaný SoftwareSerial D7/D8 pri 38400 Bd. Mega `Serial3` zostáva na 115200 Bd; na ATmega2560 zodpovedá D14/TX3 a D15/RX3.
- **REFERENČNÉ, FYZICKY NEOVERENÉ NA NAŠOM KUSE:** presná DIP tabuľka nižšie pochádza z dokumentácie konštrukčne zhodnej rodiny Mega+WiFi R3/RobotDyn. Techfun na produktovej stránke presnú revíziu DPS, schému ani tabuľku jednotlivých prepínačov nezverejňuje. Zhodný vzhľad a funkčný princíp nie sú dostatočný dôkaz elektrickej identity; režimy sa preto nesmú označiť ako fyzicky potvrdené bez testu našej IOT382.

Referenčné podklady: [RobotDyn Mega+WiFi R3](https://robotdyn.com/mega-wifi-r3-atmega2560-esp8266-flash-32mb-usb-ttl-ch340g-micro-usb), [referenčná schéma](https://content.instructables.com/F8K/SN06/KRRSAYXR/F8KSN06KRRSAYXR.pdf) a [diskusia Arduino Forum k súčasnému USB Mega + Serial3 ESP režimu](https://forum.arduino.cc/t/connecting-and-working-with-mega-wifi-r3-atmega2560-esp8266-32mb-memory/476356?page=3).

### Referenčná funkcia DIP 1–8

| DIP | Referenčná funkcia | Stav pre našu Techfun IOT382 |
|---:|---|---|
| 1–2 | pár obojsmerných UART ciest ATmega2560 ↔ ESP8266; cieľový Mega UART určuje samostatný UART selector | Funkcia páru pravdepodobná; presná funkcia DIP 1 oproti DIP 2 a elektrické smerovanie sú `NEOVERENÉ` |
| 3–4 | pár obojsmerných ciest CH340 ↔ ATmega2560 Serial0 | Referenčné; presná funkcia DIP 3 oproti DIP 4 je `NEOVERENÉ` |
| 5–6 | pár obojsmerných ciest CH340 ↔ ESP8266 UART | Referenčné; presná funkcia DIP 5 oproti DIP 6 je `NEOVERENÉ` |
| 7 | programovací boot ESP8266, v upload režime privádza GPIO0 do programovacej úrovne | `NEOVERENÉ` na našej revízii |
| 8 | v referenčnej dokumentácii nepoužitý/rezervovaný | `NEOVERENÉ`; ponechať OFF, kým nebude fyzicky alebo schémou potvrdený |

Poloha samostatného UART selectoru je súčasťou výsledného režimu. Pre aktuálny projekt musí smerovať interné spojenie na **RXD3/TXD3**, teda Mega Serial3. Jeho presné označenie a fyzická poloha na našom kuse zostávajú `NEOVERENÉ`.

### Referenčné režimy prepínačov

`ON/OFF` znamená DIP 1 až DIP 8 v uvedenom poradí. Všetky polohy v tabuľke sú zatiaľ **REFERENČNÉ – FYZICKY NEOVERENÉ NA NAŠEJ IOT382**.

| Režim | DIP 1–8 | Fyzické prepojenie | Mega UART a piny | Programovanie Mega | USB Serial Monitor Mega | Mega ↔ ESP8266 | Konflikty/poznámky |
|---|---|---|---|---|---|---|---|
| USB/CH340 ↔ ATmega2560 | OFF, OFF, ON, ON, OFF, OFF, OFF, OFF | CH340 ↔ Mega | Serial0: D0/RX0, D1/TX0 | ÁNO | ÁNO | NIE | ESP je od UART oddelený; D0/D1 nesmie súčasne používať iné zariadenie |
| USB/CH340 ↔ ESP8266 – programovanie | OFF, OFF, OFF, OFF, ON, ON, ON, OFF | CH340 ↔ ESP UART; GPIO0 v boot režime | žiadny Mega UART | NIE | NIE | NIE | Určené na flash ESP; po skončení treba DIP7 vrátiť OFF |
| USB/CH340 ↔ ESP8266 – diagnostika/runtime | OFF, OFF, OFF, OFF, ON, ON, OFF, OFF | CH340 ↔ ESP UART | žiadny Mega UART | NIE | NIE | NIE | USB terminál komunikuje s ESP, nie s Mega |
| ATmega2560 ↔ ESP8266 – interný UART | ON, ON, OFF, OFF, OFF, OFF, OFF, OFF | Mega ↔ ESP; CH340 odpojený | pri selectore RXD3/TXD3: Serial3, D14/TX3 a D15/RX3 | NIE cez onboard CH340 | NIE | ÁNO | Aktuálny kód vyžaduje Serial3/115200; selector musí byť správne nastavený |
| USB Mega + Mega ↔ ESP súčasne | ON, ON, ON, ON, OFF, OFF, OFF, OFF | CH340 ↔ Mega Serial0 a zároveň Mega Serial3 ↔ ESP | USB: D0/RX0, D1/TX0; ESP: D14/TX3, D15/RX3 | Referenčne ÁNO | Referenčne ÁNO | Referenčne ÁNO | Žiadaný servisný režim; musí sa fyzicky overiť, že upload/reset Mega nenaruší ESP a obe linky sú oddelené |
| Všetko UART odpojené / neutrál | OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF | žiadna z troch UART ciest | žiadny | NIE cez onboard CH340 | NIE | NIE | Mega a ESP môžu byť napájané a bežať samostatne; podpora presne tohto neutrálneho stavu je `NEOVERENÉ` |

Pri internom spojení sa fyzicky krížia smery TX→RX. Označenie D14/TX3 a D15/RX3 opisuje piny ATmega2560, nie piny ESP. Existujúce pridelenie Serial3 sa týmto dokumentačným doplnením nemení.

## Plánovaná architektúra logovania

- Mega je hlavný prevádzkový logger: bude ukladať hlavné procesné údaje v základnom intervale 1 minúta a okamžité `EVENT` záznamy pri zmene stavov a poruchách.
- Uno je jednoduchá nezávislá MONITOR/BASIC/watchdog jednotka a diagnostická čierna skrinka. Periodické a eventové záznamy sú oddelené do denných FAT 8.3 súborov `LYYMMDD.CSV` a `EYYMMDD.CSV`. Eventový súbor zapisuje iba zmeny lokálnych validít, linky, dostupnosti výsledku Mega, prijatých výsledných diagnostických bitov a časového stavu, nie každý cyklus.
- Uno SD vrstva je fyzicky otestovaná. Mega SD vrstva zostáva plánovaná do fyzického testu druhého modulu a karty.
- RTC DS3231 ostáva hlavný zdroj dátumu a času na Mega. Mega V5 prenáša RTC ako platné iba pri úspešnom čítaní a úspechu existujúcej autoritatívnej kontroly `rtcCasJePlatny()`, ktorá zahŕňa OSF a existujúce rozsahy; ide o rovnakú validitu ako používa filtrácia. Uno vedie lokálny softvérový dátum/čas cez `millis()`, pri prvom platnom rámci a následne približne raz za hodinu synchronizuje základ; rozdiel dátumu po obnove linky vyvolá okamžitú synchronizáciu. Pri strate UART pokračuje lokálny čas aj kalendár vrátane prechodu cez polnoc. Po resete bez platného RTC dátumu Uno nevymýšľa dátum a používa súbory `UNDTLOG.CSV` a `UNDTEVT.CSV`; po prijatí platného dátumu ďalšie zápisy automaticky smerujú do príslušných denných súborov.
- Diagnostická fyzická vrstva Mega↔Uno používa priame TTL prepojenie Mega Serial2 D16/D17 a Uno D7/D8 pri 38400 Bd cez sériový 10 kΩ v každom smere; nie je tým schválený safety heartbeat ani SMART/BASIC protokol.

## Senzorová redundancia a krížová diagnostika

Kompletná analytická vrstva je implementovaná iba na Mega: čerstvosť meraní Una, výber primárnej/fallback hodnoty, T1/T2 konflikty, 10-minútová krížová diagnostika a SUSPECT stavy. Uno lokálne validuje UNO_T1 rolovým rozsahom `>-10 && <60 °C`, UNO_T2/UNO_T3 rozsahom `>-10 && <100 °C` a TBOX ponecháva v technickom DS18B20 rozsahu. Mega nespolieha iba na validity bit: pred fallbackom prijatú UNO_T1/T2/T3 znovu kontroluje rovnakými rolovými rozsahmi. Pri strate Mega zostáva lokálne meranie a logger Una funkčný; budúca BASIC/safety logika sa musí vedieť oprieť o lokálne validity. UART nie je safety prvok a žiadny z týchto stavov zatiaľ neovláda BASIC_R1–R4, LOW WATER, watchdog ani SMART/BASIC agreement.

Binárna telemetria V5 Mega→Uno obsahuje sekvenciu, výslednú vybranú teplotu bazéna s validitou a zdrojom, výslednú vybranú T2 s validitou a zdrojom, bitové výsledky `CONFLICT/SUSPECT`, commissioning flag `MEGA_XKC_LOW_WATER`, výsledný kompatibilný health stav Mega, RTC sekundy od polnoci a dátum s validitou. Jej 24 B layout sa oproti V4 nezmenil. Aktuálny 22 B rámec Uno→Mega obsahuje UNO_T1, UNO_T2, UNO_TBOX, UNO_SONAR so stavom, UNO_T3, jednoduchý lokálny stav Una, sekvenciu a flagy `UNO_AGREEMENT_ON` a `UNO_XKC_LOW_WATER`. Teploty sa prenášajú ako znamienkové 16-bitové stotiny °C, sonar ako neznamienkové 16-bitové desatiny cm a viacbajtové čísla v poradí little-endian. Neplatné hodnoty majú v poli hodnoty nulu a rozhodujúci je príslušný validity bit.

Pevná hlavička oboch rámcov: bajt 0 `0xBA`, bajt 1 `0x5E`, bajt 2 verzia `5`, bajt 3 typ (`1` Uno→Mega, `2` Mega→Uno), bajt 4 celková dĺžka, bajty 5–6 sekvencia. Posledný bajt je CRC-8/ATM s polynómom `0x07`, počiatočnou hodnotou `0x00`, počítané cez všetky predchádzajúce bajty.

- Uno→Mega, 22 B: bajt 7 validity (`bit0 T1`, `bit1 T2`, `bit2 TBOX`, `bit3 sonar`, `bit4 T3`, bity 5–7 musia byť 0 a Mega rámec pri ich nastavení odmietne), 8 stav Una, 9 stav sonaru, 10–11 UNO_T1, 12–13 UNO_T2, 14–15 UNO_TBOX, 16–17 sonar, 18–19 UNO_T3, 20 flagy (`bit0 UNO_AGREEMENT_ON`, `bit1 UNO_XKC_LOW_WATER`, bity 2–7 musia byť 0), 21 CRC-8/ATM počítané cez bajty 0–20.
- Mega→Uno, 24 B: bajt 7 validity (`bit0 pool`, `bit1 T2`, `bit2 RTC`, bity 3–7 musia byť 0 a Uno rámec pri ich nastavení odmietne), 8 zdroj pool, 9 zdroj T2, 10 diagnostické bity (`bit0 T1_CONFLICT`, `bit1 T2_CONFLICT`, `bit2 MEGA_T1_SUSPECT`, `bit3 UNO_T1_SUSPECT`, `bit4 MEGA_T2_SUSPECT`, `bit5 UNO_T2_SUSPECT`, `bit6 MEGA_XKC_LOW_WATER`, `bit7 musí byť 0`), 11 stav Mega, 12–13 výsledný pool, 14–15 výsledná T2, 16–19 RTC sekundy od polnoci, 20 rok `00–99`, 21 mesiac, 22 deň, 23 CRC.

**V5 XKC FYZICKY OVERENÉ / COMMISSIONING PASS 1. 9. 2026:** obe dosky používajú `LINK_PROTOCOL_VERSION=5`. WATER bol na Mega aj Uno zobrazený ako WATER a `XKC_CONFLICT=NO`; LOW_WATER bol na oboch LOW_WATER a `XKC_CONFLICT=NO`; zámerné odpojenie jedného kanála vytvorilo WATER proti LOW_WATER a `XKC_CONFLICT=YES`; odpojený signál/pin prešiel cez `INPUT_PULLUP` bezpečným smerom na LOW_WATER. V5 prenos XKC oboma smermi je fyzicky potvrdený. Rozmery rámcov, časovanie, CRC, sekvencie a agreement logika sa nezmenili. Tento commissioning prebehol ešte pred aktivovaním XKC Safety V1; vzdialený XKC flag a konflikt naďalej zostávajú iba diagnostické, lokálna doska však po vlastnom 5 s potvrdení LOW WATER aktivuje svoj TOTAL STOP.

Stav Mega v bajte 11 má význam: `0 = MEGA_CONTROL_OK`; `1 = MEGA_CONTROL_DEGRADED_TRUSTED`, keď je niektorá funkcia degradovaná, používa fallback alebo je bezpečne izolovaná, ale Mega naďalej garantuje riadenie/izoláciu; `2 = MEGA_CONTROL_UNTRUSTED`, keď control vrstva túto garanciu stratila. Aktuálna jediná implementovaná podmienka pre 2 je `FIL_CONTROL_VALID=false`. Neplatný pool, T2, T3, ich kombinácie, platné T4/UNO_T1/UNO_T2/UNO_T3 fallbacky aj `SOLAR_CONTROL_VALID=false` dávajú pri platnej filtrácii stav 1; solár je pri neplatnom control vstupe existujúcou logikou bezpečne vypnutý. Budúce kritické control podmienky sa smú pridať až po samostatnom schválení a implementácii. Tento bajt je iba kompatibilný vstup health/agreement pre Uno; jedinou autoritou runtime režimu zostáva `SystemMode systemMode` na Mega.

**V4 IMPLEMENTOVANÉ/SKOMPILOVANÉ/FYZICKY OVERENÉ 22. 8. 2026:** obe dosky používajú `LINK_PROTOCOL_VERSION=4`. Mega parser prijíma 22 B Uno rámec, snapshot obsahuje `t3/t3Ok` a bajty 18–19 fyzicky prenášajú UNO_T3. Mega→Uno zostáva 24 B s nezmeneným obsahom. V3 a V4 sa nesmú miešať. Pri tomto teste zostali všetky linkové chybové čítače nulové. Startup/commissioning ROM scanner bol už odstránený a pri fyzickom V4 boote sa nespustil.

Stav Una v bajte 8 má význam: `0 = UNO_OK` (procesor a supervisor vrstva žijú), `1 = UNO_SENSOR_DEGRADED` (jeden alebo viac lokálnych meracích senzorov je neplatných, ale Uno naďalej vykonáva dohľad, logovanie a budúci BASIC), `2 = UNO_CHYBA` (rezervované až pre skutočnú kritickú poruchu Una alebo jeho supervisor/safety vrstvy). Samotná chyba UNO_T1, UNO_T2, UNO_T3, UNO_TBOX alebo sonaru nesmie znamenať `UNO_CHYBA` ani automaticky zhodiť SMART.

Pravidelný USB Serial servisný blok Una sa vypisuje najviac raz za 10 s. Po schválenom prvom SAFE TRIM je kompaktný a priamo zobrazuje stav Una, Mega linky, agreement, lokálny/vzdialený XKC a konflikt, `XKC_CONFIRM=0..5s`, `XKC_TRIP=YES/NO`, `XKC_RECOVERY=0..10s`, SD, hodnotu aj `OK/ERR` pre UNO_T1/T2/T3/TBOX, stav sonaru a povinné chybové čítače `CRC_FAIL`, `FRAME_INVALID`, `LINK_TIMEOUT` a `SEQ_GAP`. Vývojové čítače `RX_FRAME_OK`, `REQUEST_OK_COUNT`, `TX_REPLY_COUNT`, meranie `SD_MAX_BLOCK_MS`, RTC `DRIFT`, rozsiahly vzdialený Mega dump a textový `PROBLEM` dekodér boli z USB diagnostiky odstránené; prijaté Mega dáta a diagnostické flagy zostali zachované pre V5, agreement a SD BLACK BOX. Zmeny lokálneho raw stavu, začiatku potvrdenia, tripu, začiatku recovery a clear medzi blokmi vypíšu iba krátke riadky `EVENT: ...` alebo `RECOVERY: ...`. Meracie, UART, agreement, regulačné a SD intervaly sa nezmenili.

Kontrolná kompilácia po implementácii V4 používa na Uno 26 182 B Flash a 1 239 B globálnej RAM; zostáva 809 B SRAM. Mega používa 40 222 B Flash a 4 277 B RAM; zostáva 3 915 B RAM. SD BLACK BOX obsah ani 512 B cache sa nemenili. Následný bench test 22. 8. 2026 fyzicky potvrdil prevádzku V4; uvedené pamäťové hodnoty zostávajú z kontrolnej kompilácie.

Kontrolná kompilácia po doplnení denného BLACK BOX logovania používa na Uno 27 402 B Flash a 1 228 B globálnej RAM; zostáva 820 B SRAM pre zásobník a budúce nevyhnutné supervisor/safety funkcie. Mega používa 34 458 B Flash a 4 077 B RAM. Táto rezerva sa nesmie považovať za voľný priestor na duplicitnú diagnostiku. Existujúci 512 B SD cache sa nemení, pretože SD plní úlohu BLACK BOX.

### T1 – teplota bazéna

- V SMART režime je `MEGA_T1` primárny a `UNO_T1` jeho nezávislá náhrada.
- Automatické prevzatie `UNO_T1` je povolené iba pri preukázanej poruche alebo neplatnosti `MEGA_T1` a pri platnom, čerstvom údaji z Una.
- Ak sú oba T1 platné, ale výrazne sa nezhodujú, UART komunikácia sama nesmie určiť víťaza ani automaticky nahradiť jeden senzor druhým.
- V budúcom BASIC smere zostáva lokálny `UNO_T1` primárny. Uno dnes neprerátava vzdialený fallback; pri živom Mega môže prijať jeho hotovú vybranú hodnotu a zdroj, pri strate Mega pokračuje s vlastnou lokálnou validitou.
- Implementovaný SMART fallback na Mega sa aktivuje iba pri lokálnej neplatnosti a používa poradie MEGA_T1 → MEGA_T4 → čerstvý UNO_T1. Uno nastaví T1 validity iba v rozsahu `>-10 && <60 °C` a Mega prijatú hodnotu pred fallbackom znovu kontroluje rovnakým rozsahom. Platný nesúlad vytvorí iba diagnostický konflikt. Finálne využitie výsledku v BASIC, potvrdenie konfliktu, hysterézia, recovery a safety následky zostávajú `NÁVRH – TREBA FYZICKY OVERIŤ`.

### T2 – solárny výstup

- `UNO_T2` je nový dlhokáblový DS18B20 pre teplotu vody na výstupe zo soláru, ROM `28 F8 23 5B 00 00 00 37`.
- Lokálne meranie, validita, recovery, Serial a SD sú implementované a fyzicky potvrdené v stave `OK`.
- V SMART režime je `MEGA_T2` primárny a `UNO_T2` jeho nezávislá náhrada pri preukázanej poruche/neplatnosti MEGA_T2.
- V budúcom BASIC režime je `UNO_T2` primárny. Uno dnes neprerátava vzdialený fallback; môže iba prijať hotovú výslednú T2 a jej zdroj od živého Mega.
- Pri dvoch platných, ale výrazne nezhodných T2 sa automatické prevzatie nevykoná bez ďalšej diagnostiky.
- Implementovaný SMART fallback na Mega je MEGA_T2 → čerstvý UNO_T2. Uno aj prijímajúca Mega vyžadujú pre UNO_T2 rozsah `>-10 && <100 °C`; samotný validity bit nestačí. Platný nesúlad vytvorí diagnostický konflikt bez automatického prevzatia. Finálne využitie výsledku v BASIC, potvrdenie konfliktu, hysterézia, recovery a safety následky zostávajú `NÁVRH – TREBA FYZICKY OVERIŤ`.

### Krížová fyzikálna diagnostika T1/T2

Diagnostický model má pri dostupnosti porovnávať štvoricu `MEGA_T1 + UNO_T1 + MEGA_T2 + UNO_T2`. Nejde zatiaľ o definitívnu safety ani regulačnú logiku.

- Bezprostredne po zapnutí solárneho čerpadla sa T2 na krížovú diagnostiku nepoužíva. Stojaca voda a nahriate panely môžu krátkodobo ukazovať približne 40–50 °C bez toho, aby išlo o chybu T2.
- **Schválený stabilizačný čas je 10 minút stabilného chodu solárneho okruhu.** Počas týchto 10 minút sa krížová diagnostika T1/T2 nesmie použiť na označenie senzora ako chybného alebo podozrivého.
- Stabilizačný čas blokuje iba krížovú diagnostiku T1/T2. Regulácia soláru môže počas neho fungovať podľa svojich normálnych platných vstupov a existujúcich podmienok.
- Desaťminútový stabilizačný čas sa musí v budúcej implementácii spustiť znova po každom štarte solárneho okruhu a po každej významnej zmene jeho hydraulickej konfigurácie, napríklad po otvorení alebo zatvorení ďalšej vetvy. Konkrétna detekcia hydraulických zmien zatiaľ nie je implementovaná.
- Dôvodom 10 minút je bezpečné vytlačenie stojatej alebo prehriatej vody a ustálenie teploty aj prietoku okruhu.
- Reálne pozorovaný ustálený rozdiel býva približne do 1 °C. Pre diagnostický model je schválená pracovná tolerancia ustáleného rozdielu T1↔T2 rádovo ±1–2 °C.
- Príklad diagnostickej indície: `MEGA_T1=18 °C`, `UNO_T1=29,2 °C` a ustálené `T2≈30,1 °C` silne označujú MEGA_T1 ako podozrivý, ale samy ešte nevytvárajú finálny safety verdikt.
- Pri bazéne približne 27 °C je ustálená T2 približne 36 °C alebo 19 °C fyzikálne podozrivá a má vyvolať diagnostický konflikt.
- Ak sú dostupné oba T2, model má najprv posúdiť ich vzájomnú zhodu a až potom ich použiť ako fyzikálnu referenciu pre T1.
- Implementácia po 10 minútach vytvára iba okamžitý diagnostický `SUSPECT`, ak sa zhodnú oba T2 a iba jeden T1 je v pracovnej tolerancii, alebo opačne. SUSPECT neinvaliduje senzor a nezasahuje do výkonu. Časové potvrdenie konfliktu, hysterézia, recovery, detekcia hydraulickej zmeny a následná fallback/safety reakcia zostávajú `NÁVRH – TREBA FYZICKY OVERIŤ`.

### T3 – teplota solárneho panela

- `UNO_T3` je pôvodný krátkokáblový DS18B20 s ROM `28 29 81 5E 00 00 00 F5`, určený na teplotu solárneho panela a nezávislú náhradu `MEGA_T3`.
- Premenovanie a plnohodnotné lokálne meranie sú implementované a fyzicky potvrdené v stave `OK`. Bench test 22. 8. 2026 potvrdil, že V4 prenáša platnú UNO_T3 do Mega v bajtoch 18–19 a Mega ju korektne prijíma.
- V SMART režime zostáva `MEGA_T3` primárny. Ak je lokálny MEGA_T3 neplatný a vzdialený UNO_T3 je platný, čerstvý a v rolovom rozsahu `>-10 && <100 °C`, Mega použije `UNO_T3_REMOTE_FALLBACK`; Mega rozsah kontroluje aj pri nastavenom validity bite. Ak nie je platný ani jeden, výsledná T3 je `INVALID`. Pri dvoch platných hodnotách zostáva bez porovnávania víťazom MEGA_T3.
- **FYZICKY OVERENÉ 22. 8. 2026:** v normálnom stave bola `T3 EFEKTIVNA = MEGA_T3_PRIMARY`. Po odpojení odbočky G2 iba lokálna MEGA_T3 prešla na `-127 °C / T3_CHYBA`; vzdialená UNO_T3 zostala platná a čerstvá a `T3 EFEKTIVNA` prešla na `UNO_T3_REMOTE_FALLBACK`. Systém prešiel do `DEGRADED`, nie do STOP, a `SOLAR control` zostal validný. Po opätovnom pripojení G2 sa automaticky obnovili `MEGA_T3_PRIMARY`, `SYSTEM=OK` a `PROBLEM=NONE`. Tento degraded/fallback stav nezrušil počítanie `SMART_STABLE`; Uno po 180 s dosiahlo `RECOVERY: UNO_AGREEMENT=ON`.
- Týmto testom nebolo fyzicky overené odpojenie alebo invalidita UNO_T3, správanie validity bitu 4 pri neplatnej UNO_T3, stav oboch T3 invalid, opačný BASIC fallback na Uno ani dnešný stav `MEGA_AGREEMENT=ON`.
- V BASIC režime bude lokálny UNO_T3 primárny a MEGA_T3 môže byť fallback iba pri preukázanej poruche/neplatnosti UNO_T3.
- Pri dvoch platných, ale nezhodných T3 sa automaticky nevyberie víťaz.
- Presné diagnostické prahy, validita a recovery zostávajú `NÁVRH – TREBA FYZICKY OVERIŤ`.
- Opačný BASIC fallback `UNO_T3 primary → MEGA_T3 fallback` zatiaľ nie je implementovaný. Presné T3 conflict/SUSPECT prahy zostávajú `TBD – TREBA FYZICKY OVERIŤ`.

### Lux ako doplnkový fallback

- Pri strate oboch T3 môže budúci lux senzor indikovať podmienky vhodné na kontrolovaný skúšobný chod soláru.
- Lux samotný nesmie byť safety dôkaz ani definitívne povolenie soláru.
- Po skúšobnom chode sa musí skutočný tepelný zisk potvrdiť cez ustálené T2 voči teplote bazéna.
- Konkrétny pin, adresa, lux prahy, dĺžka skúšobného chodu a podmienky ukončenia zostávajú `NÁVRH – TREBA FYZICKY OVERIŤ`.

### Rozšírená kontextová teplotná autodiagnostika – SCHVÁLENÝ NÁVRH / TBD

Vlastníkom budúcej rozšírenej analytiky je Mega. Uno ju nemá paralelne duplikovať; naďalej poskytuje vlastné merania a lokálne validity. Tento návrh zatiaľ nemení implementovanú reguláciu, fallback priority, validity, agreement ani safety. Základné pravidlo je:

`MEASURE / LOG ALWAYS`  
`DIAGNOSTIC EVALUATION ONLY WHEN CONDITIONS ARE VALID`

Senzory sa môžu merať, zobrazovať a logovať prakticky stále, ale ich rozdiel sa smie interpretovať ako diagnostický dôkaz iba vtedy, keď prevádzkový a hydraulický stav fyzikálne umožňuje očakávať zhodu. Ak je `DIAGNOSTIC_VALID=false`, údaje sú iba `LOG / DISPLAY`; normálna fyzikálna odchýlka nesmie vytvoriť poruchu. Cieľom nie je pridávať ďalšie senzory iba kvôli diagnostike, ale lepšie využívať existujúce meracie body podľa kontextu systému.

#### Bazénová skupina MEGA_T1 / UNO_T1 / MEGA_T4

- `MEGA_T1` a `UNO_T1` reprezentujú rovnakú bazénovú teplotu a môžu sa diagnosticky porovnávať priebežne podľa existujúcich pravidiel čerstvosti a validity.
- `MEGA_T4` meria pri dne bazéna. Stále sa meria, loguje, zobrazuje a sleduje sa jeho trend, ale bez dostatočného premiešania sa nesmie použiť ako rozhodujúca referencia voči MEGA_T1/UNO_T1. Rozdiel spôsobený stratifikáciou nie je porucha.
- T4 môže dostať diagnostickú váhu až po dostatočne dlhom a stabilnom miešaní. Prvá pracovná podmienka je približne 30 minút súvislého chodu príslušného solárneho/miešacieho čerpadla; budúce potvrdenie prietokom zostáva `TBD`.
- Diagnostické okno sa nepovoľuje pri čerstvom dopúšťaní studenej vody, bezprostredne po štarte čerpadla, krátko po zmene hydraulickej vetvy ani v inom potvrdenom prechodovom stave. Presná detekcia týchto stavov a finálne časy zostávajú `TBD – TREBA URČIŤ Z REÁLNYCH DÁT`.
- Tolerancia `MEGA_T1 ↔ UNO_T1` má byť prísnejšia než tolerancia ktorejkoľvek z týchto hodnôt voči T4. Ani premiešaný bazén nemusí byť homogénny na desatinu stupňa; konkrétne tolerancie sa zatiaľ nezamykajú.

Ak vznikne konflikt medzi platnými MEGA_T1 a UNO_T1 a súčasne je platné diagnostické okno T4, Mega vyhodnotí sériu meraní všetkých troch párov: `MEGA_T1 ↔ UNO_T1`, `MEGA_T1 ↔ MEGA_T4` a `UNO_T1 ↔ MEGA_T4`. Jedno meranie nestačí. Ak sa počas platného okna opakovane zhoduje jasná dvojica 2 z 3 a tretí senzor sa významne odlišuje, tretí sa označí `SUSPECT`. Ak jasná dôveryhodná dvojica nevznikne, výsledkom zostane iba `CONFLICT / DEGRADED`; systém nesmie automaticky vybrať víťaza, meniť kalibráciu ani vyradiť senzor.

Orientačný budúci stavový tok bazénovej skupiny je:

`NORMAL → CONFLICT → čakanie na platné miešanie → 3-WAY DIAGNOSTIC → 2 z 3?`

- bez jasnej dvojice: `CONFLICT / DEGRADED`;
- s jasnou dvojicou: `SUSPECT → OBSERVE → CALIBRATION_CANDIDATE alebo FAULT → VERIFY`.

#### Dlhodobé pozorovanie SUSPECT a kalibrácia

Po nájdení kandidáta Mega nemá okamžite meniť kalibráciu. Počas ďalších diagnosticky platných období miešania má zbierať:

- `REF = priemer dvoch dôveryhodných senzorov`;
- `OFFSET = SUSPECT_RAW - REF`.

Pracovný cieľ je približne 3–6 hodín nazbieraného času aktívneho a diagnosticky platného miešania. Nemusí ísť o jeden neprerušený blok. Keď miešanie alebo ostatné podmienky nie sú platné, hodnoty sa môžu ďalej logovať, ale diagnostický čas sa nezvyšuje a vzorky sa nepoužijú na rozhodnutie o kalibrácii.

- Stabilný a rozumne malý offset: `CALIBRATION_CANDIDATE`.
- Nestabilná alebo meniaca sa odchýlka: `SENSOR_UNSTABLE / FAULT`; bez rekalibrácie.
- Stabilná, ale príliš veľká odchýlka: `SENSOR_FAULT / SERVICE`; bez automatickej kalibrácie. Orientačná hranica približne 2 °C zostáva konfigurovateľný návrh a nie je finálnym limitom.

Pri každej budúcej kalibrácii sa musia oddelene zachovať `RAW_VALUE`, `CAL_OFFSET` a `USED_VALUE`. RAW hodnota sa nikdy nesmie stratiť. Po korekcii sa ďalej sleduje trend potrebného offsetu, pretože jeho rast môže znamenať degradáciu senzora. Automaticky sa nesmú kalibrovať dva senzory naraz a kalibrácia je prípustná iba pri jasnej a dlhodobo potvrdenej dvojici 2 z 3. Konkrétne uloženie korekcií, limity stability, minimálny počet vzoriek, potvrdenie a návrat zostávajú `TBD`.

#### T2 – kontext solárneho výstupu

T2 má inú fyziku než bazénové senzory. Po štarte solárneho okruhu môže stojaca a prehriata voda vytvoriť rozdiel voči bazénu aj viac než 20 °C bez poruchy senzora. Rozšírený model preto rozlišuje:

- prvých približne 0–30 minút stabilného chodu: `T2 = LOG ONLY`;
- po minimálne približne 30 minútach: `T2 = DIAGNOSTIC VALID`, iba ak čerpadlo stále beží, hydraulická konfigurácia sa nezmenila a systém nie je v prechodovom stave; budúce potvrdenie stabilného prietoku zostáva `TBD`.

Reálne pozorovania hovoria, že po ustálení trvajúcom približne 10–15 minút býva rozdiel T2 voči bazénu typicky približne do 2 °C aj pri zmenách slnka. Pre rozšírenú autodiagnostiku sa však zámerne navrhuje konzervatívne 30-minútové okno. Hodnota približne 2 °C je iba pracovný poznatok pre zber dát, nie tvrdý finálny limit.

Počas fyzikálne platného režimu bude možné T2 porovnávať s bazénovou referenciou, vlastným historickým trendom a neskôr s ďalšími senzormi rovnakej hydraulickej vetvy. Stabilný malý offset môže vytvoriť kandidáta na kalibráciu, nestabilná odchýlka `FAULT` a extrémny stabilný rozdiel `SERVICE / FAULT`; mimo platného režimu sa žiadny z týchto verdiktov nevytvára.

**Vzťah k aktuálnej implementácii:** dnešná základná krížová diagnostika T1/T2 zostáva implementovaná po 10 minútach podľa predchádzajúcej sekcie a vytvára iba diagnostické `CONFLICT/SUSPECT` bez zásahu do výkonu. Nové približne 30-minútové okno patrí výhradne budúcej rozšírenej autodiagnostike, dlhodobému trendu a kalibračnému modelu. Neznamená tichú zmenu existujúceho kódu ani existujúceho 10-minútového pravidla. Pred implementáciou sa musí osobitne schváliť presné zlúčenie alebo nahradenie týchto dvoch úrovní.

#### Pravidlo pre ďalšie senzorové skupiny

Budúce rozšírenie ani prípadná náhrada Una druhou Mega nesmie vytvoriť pravidlo „všetko porovnávaj so všetkým“. Každá skupina musí mať jasne určené:

- čo fyzicky meria;
- kedy sa majú hodnoty zhodovať a kedy je rozdiel normálny;
- ktoré prechodové obdobie sa ignoruje;
- ktoré prevádzkové podmienky vytvárajú `DIAGNOSTIC_VALID=true`;
- vlastníka výpočtu a jeho výstupný stav.

Bežnú komplexnú analytiku vlastní Mega. Kritické safety overenia zostávajú samostatnou nezávislou vrstvou podľa tvrdej architektúry a tento návrh ich nemení. Pred implementáciou treba najprv nazbierať reálne údaje MEGA_T1, UNO_T1, MEGA_T4 a oboch T2 v jednotlivých hydraulických režimoch a z nich určiť tolerancie, časové potvrdenia, hysteréziu a recovery.

## Sledovanie teploty rozvádzača

- UNO_TBOX je fyzicky zapojený ako tretí DS18B20 na existujúcej Uno 1-Wire zbernici D2.
- Pevná ROM UNO_TBOX: `28 16 2E 09 00 03 24 29`.
- Má samostatné meranie, validitu a chybový stav, zobrazuje sa v Serial diagnostike a zapisuje do denného `LYYMMDD.CSV` alebo pred synchronizáciou do `UNDTLOG.CSV`.
- Fyzickým testom bol potvrdený stav `TBOX_OK`.
- UNO_T1, UNO_T2, UNO_T3 a UNO_TBOX používajú spoločnú D2 1-Wire zbernicu s jedným fyzicky osadeným pull-upom 4,7 kΩ. Spoločný fyzický test našiel všetky štyri pevné ROM a potvrdil všetky štyri senzory ako `OK`. Ďalší paralelný pull-up sa nepridáva; pripojovacie napätie existujúceho rezistora zostáva `NEOVERENÉ`.
- MEGA_TBOX je fyzicky nainštalovaný ako piaty DS18B20 na Mega 1-Wire zbernici D2, s pevnou ROM `28 B1 31 05 00 03 24 F1`. Sketch ho identifikuje názvom `MEGA_TBOX`, samostatne meria a diagnostikuje jeho platnosť.
- MEGA_TBOX je zatiaľ iba monitorovací/diagnostický senzor. Jeho chyba nemení `system_OK`, nespúšťa alarm, nevypína zariadenia a nevstupuje do regulačnej ani safety logiky. Výpadok MEGA_TBOX nesmie zablokovať MEGA_T1–T4 ani základnú funkciu Mega.
- UNO_TBOX a MEGA_TBOX sú nezávislé senzory teploty rozvádzača a do budúcna môžu slúžiť na vzájomnú kontrolu merania. Tretí holý DS18B20 zostáva rezerva.
- Podľa reálnych prevádzkových dát môže teplota skrine v budúcnosti ovládať malé chladiace ventilátory cez relé. Orientačne sa uvažuje zásah približne pri 50–60 °C; hranice, relé a logika nie sú určené ani implementované.

## Vrstva 1 – SMART/BASIC dohoda

- Mega drží vlastné SMART povoľovacie relé.
- Uno drží vlastné SMART povoľovacie relé.
- SMART platí iba pri aktívnom držaní oboch relé.
- Odpadnutie ktoréhokoľvek relé mechanicky cez NC aktivuje BASIC vetvu.
- Reset, strata napájania alebo strata dohody jedného kontroléra preto vedie do BASIC.
- Prechod do BASIC sám nezapína BASIC_R1 až BASIC_R4.

Logika: SMART = MEGA_AGREE AND UNO_AGREE; BASIC = NOT SMART.

Dôvod: režim sa nesmie vyberať iba softvérovou správou.

Fyzický stav Mega D31 bol 1. 9. 2026 potvrdený ako watchdog/povoľovacia vetva riadená existujúcou MEGA_AGREEMENT logikou: boot/reset LOW, po 180 s stability HIGH, pri strate podmienok okamžite LOW a po recovery nový celý 180 s interval. Uno agreement zostáva na D9, Uno TOTAL STOP na A0, Mega TOTAL STOP na D32 a W1209 fackovač na D33. Pre H/L moduly platí fyzicky overené `HIGH → COM–NO`, `LOW/strata napájania → COM–NC`. Automatické fault podmienky TOTAL STOP a W1209 resetu týmto commissioning checkpointom nevznikli.

**HISTORICKÝ STAV PRED PRVOU AGREEMENT IMPLEMENTÁCIOU:** programy pôvodne nastavovali D9 a D31 ako `INPUT` bez interného pull-upu. Mega D31 nemal inú aktívnu periodickú funkciu a Uno D9 nekolidoval s existujúcou funkciou. Tento vysokoodporový stav už nie je aktuálny; hlavné programy ich teraz od bootu nastavujú ako `OUTPUT LOW` a následne riadia podľa platnosti aktuálnej linky.

Aktívna polarita oboch H/L modulov je **FYZICKY OVERENÁ** a zhodná: `HIGH → COM–NO`, `LOW → COM–NC`. Pri strate napájania je mechanický pokojový/fail-safe stav `COM–NC`. Samostatné diagnostické projekty `HL_RELAY_MEGA_D31_TEST` a `HL_RELAY_UNO_D9_TEST` túto polaritu potvrdili automatickým striedaním HIGH a LOW po 5 sekundách. Konkrétne finálne použitie kontaktov COM/NO/NC v SMART/BASIC agreement vrstve ešte nie je schválené ani implementované.

### Prvá fyzická implementácia agreement – linkový test

Hlavné programy Mega a Uno od 21. 8. 2026 priamo riadia fyzicky overené H/L relé; samostatné 5-sekundové testovacie sketche už nie sú potrebné na bežný test linkového agreement. Ide o **PRVÚ FYZICKÚ IMPLEMENTÁCIU AGREEMENT**, nie o finálnu BASIC/safety logiku:

- Mega D31 aj Uno D9 sú od bootu/resetu `OUTPUT LOW`; pin nesmie zostať `INPUT/FLOAT`.
- `LOW = vlastné SMART agreement NEPOVOLENÉ = COM–NC`.
- platný V5 rámec približne každú sekundu udržiava informáciu o čerstvosti; rozhoduje čas od posledného platného rámca, nie počet vynechaných paketov;
- link timeout aj stale limit sú 10 sekúnd; rozhoduje vek posledného platného rámca, takže niekoľko vynechaných približne sekundových rámcov agreement nezhodí, ale po skutočnom prekročení 10 sekúnd zdravá doska okamžite nastaví vlastné relé LOW a vynuluje stabilizáciu;
- po boote, resete alebo každom zrušení agreement musí každá doska absolvovať 180 sekúnd nepretržite stabilného stavu. Počas intervalu zostáva výstup LOW;
- minimálne podmienky implementovanej stabilizácie sú platná V5 linka, čerstvé vzdialené dáta a vzdialená doska bez kritického stavu procesora/supervisor vrstvy;
- `UNO_SENSOR_DEGRADED` ani výsledný stav Mega `DEGRADED` samy osebe stabilizáciu nerušia;
- každá relevantná chyba interval okamžite vynuluje; po jej odstránení začne nový interval od nuly;
- Mega nastaví D31 na HIGH a Uno nastaví D9 na HIGH až po dokončení celých 180 sekúnd. Recovery teda už nie je okamžitý po prvom obnovenom rámci;
- fyzický princíp je `2× HIGH = obe dosky dávajú ÁNO`; ktorákoľvek úroveň LOW ruší najmenej jedno agreement a SMART preto nesmie byť povolený;
- ak jedna doska zamrzne s výstupom HIGH, zdravá druhá doska po strate V5 linky zruší svoje agreement prechodom na LOW;
- strata napájania relé mechanicky vráti na COM–NC bez závislosti od programu.

Diagnostika oboch dosiek zobrazuje `SMART_STABLE=BLOCKED`, priebeh `SMART_STABLE=n/180s` alebo `SMART_STABLE=READY`. Zmena stabilizácie používa krátke `EVENT:`/`RECOVERY:` riadky bez blokovania hlavného programu.

Táto implementácia stále neovláda autonómne BASIC_R1–R4, automatický reset druhej dosky, RESET_LOCKOUT, motorový dead-time ani ďalšie neschválené safety podmienky. LOW WATER XKC Safety V1 je implementovaný oddelene: každý lokálny kanál po 5 s aktivuje vlastný TOTAL STOP. Fyzicky sú BASIC_R1/R2 vložené do motorových ciest a BASIC_R3 do 12 V napájacej cesty W1209 cez pokojový `COM–NC`; BASIC_R4 zostáva nezapojenou rezervou. UART/V5 je podmienkou aktuálneho základného agreement, ale nie je potrebný pre lokálny XKC trip a stále nie je finálnym samostatným safety heartbeat prvkom. XKC telemetrické bity agreement nijako nemenia.

## Vrstva 2 – LOW WATER FIL/SOLAR

Jeden spoločný fyzický XKC-Y25-NPN je pripojený dvoma nezávislými optočlenovými cestami:

`1× XKC → 2× PC817/HY-M154 read path → Mega D30 / Uno A2`

Fyzické meranie samotného XKC potvrdilo: voda prítomná/LED ON dáva na žltom OUT približne 0 V; sucho/LED OFF približne +5 V. Za optočlenmi používajú oba MCU `INPUT_PULLUP` a interpretáciu `LOW = WATER`, `HIGH = LOW_WATER / DRY / otvorená signálová cesta`.

Mega D30 a Uno A2 majú každý vlastný optočlenový kanál. PC817 sa nevracia do Mega↔Uno UART; dátová linka zostáva priame TTL cez 10 kΩ pri 38400 Bd.

Dve optočlenové cesty poskytujú nezávislé čítanie a interpretáciu procesormi, ale nevytvárajú druhý fyzický snímač: XKC zostáva spoločným sensing elementom. Fyzický commissioning oboch MCU vstupov a V5 prenosu je uzavretý ako PASS: zhodné WATER aj LOW_WATER dávajú `XKC_CONFLICT=NO`, rozpojenie jedného kanála dáva `XKC_CONFLICT=YES` a otvorený vstup smeruje na LOW_WATER.

XKC Safety V1 používa na oboch doskách rovnaký neblokujúci stavový automat cez `millis()`: lokálny HIGH/LOW WATER musí trvať súvisle aspoň 5 000 ms, potom sa nastaví lokálny trip; kratší impulz sa zahodí a návrat na WATER potvrdzovací čas vynuluje. Mega trip aktivuje D32 a `explicitnySystemStopAktivny()` ho mapuje na `MODE_STOP`. Uno trip aktivuje A0 bez závislosti od Mega, UART alebo agreement. Po tripe musí lokálny WATER trvať súvisle 10 000 ms; až potom sa trip a vlastný TOTAL STOP uvoľnia, bez priameho štartu filtrácie alebo soláru. Prerušené recovery sa vynuluje. `TOTAL_STOP_REQUEST` každej dosky sa agreguje na jednom mieste; dnes je jeho jediným schváleným dôvodom príslušný lokálny XKC trip a budúce dôvody sa smú dopĺňať iba explicitným OR.

Vzdialený XKC stav a `XKC_CONFLICT` zostávajú diagnostické. Konflikt sám nevytvára osobitný trip, ale lokálny 5 s LOW WATER aktivuje príslušnú dosku aj počas konfliktu. XKC nemení `megaStav`, D31/D9 agreement, `aktualnyUnoStav()`, BASIC_R1–R4, W1209 ani R9/R10. V5 layout, časovanie, CRC a sekvencie sa nemenia. Fyzický commissioning automatickej TOTAL STOP akcie pri bazéne ešte čaká.

Filtrácia má dve blokovacie relé v sérii: jedno ovláda Mega, druhé Uno. Solár má rovnakú dvojicu. Ich logika je odlišná od SMART/BASIC:

- normálne sú cievky neaktívne a výkonová vetva je povolená;
- pri LOW WATER kontrolér aktivuje svoje relé a vetvu fyzicky zakáže;
- na STOP stačí rozhodnutie ktoréhokoľvek procesora;
- strata napájania alebo reset jedného Arduina sama osebe pri tejto vrstve neznamená STOP.

FIL je povolený iba ak nie je aktívne FIL_BLOCK Mega ani Uno. SOLAR je povolený iba ak nie je aktívne SOLAR_BLOCK Mega ani Uno.

LOW WATER je nadradený SMART aj BASIC a prechod režimu ho nesmie obísť. Mega a Uno majú mať približne 500 Wh zálohované napájanie; bezpečnosť je redundantná dvoma procesormi.

## Vrstva 3 – BASIC R1–R4

BASIC relé sú napájané z potvrdenej 5 V vetvy veľkého CC/CV buck meniča #2. Táto vetva napája 16R relé/BASIC časť, nie Arduino Uno. Uno má vlastnú samostatnú vetvu cez LM2596 nastavený na 7,5 V.

Existuje iba jedna fyzická 16-kanálová reléová doska. Jej vlastníctvo je pevne rozdelené:

- fyzické kanály 1–4 patria výhradne Uno/BASIC/watchdog vrstve; vodiče z Mega boli fyzicky odstránené a Mega ich už neovláda;
- fyzické kanály 5–16 zostávajú reléovou časťou Mega/SMART;
- pôvodné Mega priradenie 16R kanálov 1–4 na D30–D33 je softvérovo aj fyzicky zrušené; D30 je nanovo pridelený commissioning vstupu MEGA_XKC, D31 fyzickej watchdog/povoľovacej vetve riadenej agreement logikou, D32 MEGA_TOTAL_STOP a D33 W1209_FACKOVAC; nejde o obnovenie väzby na 16R kanály 1–4;
- BASIC_R1/R2 sú fyzicky zapojené do motorových ciest. BASIC_R3 je fyzický kanál 3 na 16R doske a jeho `COM–NC` kontakt je zapojený v 12 V napájacej ceste W1209 pred samostatným supervision modulom. Konkrétne Uno/BASIC riadiace piny a autonómna logika zostávajú `NEURČENÉ`; BASIC_R4 zostáva rezervovaný a fyzicky ďalej nezapojený.

| Fyzický kanál 16R | Vlastník | Názov | Funkcia |
|---:|---|---|---|
| 1 | Uno/BASIC/watchdog | BASIC_R1 | BASIC filtrácia |
| 2 | Uno/BASIC/watchdog | BASIC_R2 | BASIC solár |
| 3 | Uno/BASIC/watchdog | BASIC_R3 | fyzicky zapojený `COM–NC` v 12 V napájacej ceste W1209; riadiaci pin/logika TBD |
| 4 | Uno/BASIC/watchdog | BASIC_R4 | BASIC 12 V |

Prechod SMART→BASIC iba sprístupní BASIC vetvu; automaticky nezapína žiadny z kanálov BASIC_R1–R4. BASIC_R1/R2 sú fyzicky vložené do motorových ciest, ale ich individuálne autonómne ovládanie zostáva úlohou budúcej Uno/BASIC/watchdog logiky. BASIC_R3 je fyzicky použitý ako pokojová `COM–NC` časť 12 V napájania W1209; jeho budúci riadiaci pin a logika zostávajú TBD. BASIC_R1 podlieha fyzickému FIL LOW WATER reťazcu a BASIC_R2 SOLAR reťazcu. LOW WATER automaticky nevypína BASIC_R3 ani BASIC_R4. Presná autonómna BASIC logika zatiaľ nie je definovaná.

Fyzické kanály 5–16 zostávajú vyhradené Mega/SMART. Aktívne sú dnes iba kanály 9 a 10; funkcie ostatných zostávajú podľa aktuálneho pinoutu neimplementované.

## Hladina

1. HY-SRF05 Uno: fyzicky testovaný na D3/D4; nezávislý monitorovací senzor.
2. HY-SRF05 Mega: fyzicky funkčne overený na D38 TRIG/D39 ECHO meraním približne 19,3–19,8 cm; nezávislý monitorovací senzor s monitor-only kódom.
3. XKC-Y25-NPN: jeden spoločný LOW WATER sensing element, fyzicky čítaný cez dve samostatné optočlenové cesty na Mega D30 a Uno A2; WATER, LOW_WATER, konflikt kanálov, otvorený vstup aj V5 prenos majú fyzický commissioning PASS. Nejde o dva fyzicky redundantné snímače. Každý lokálny kanál má XKC Safety V1 autoritu nad vlastným TOTAL STOP po 5 s potvrdení a 10 s WATER recovery. Fyzický test 2026-09-02 pri odpojenom UART potvrdil aktiváciu oboch TOTAL STOP relé, ich 10 s recovery aj reset recovery novým LOW WATER; stav je `PHYSICAL PASS / COMMISSIONED`.

MEGA_SONAR a UNO_SONAR sú zatiaľ iba dvojica nezávislých monitorovacích senzorov. Nesmú sa používať na rozhodovanie o hladine, LOW WATER ani dopúšťaní, kým nebude fyzicky hotová finálna vyrovnávacia nádoba, referenčná geometria a kalibrácia.

## Dopúšťanie vody – ochrana proti preplneniu

Dopúšťanie má dve od elektroniky nezávislé ochranné úrovne. Arduino/Mega smie iba vytvoriť požiadavku na otvorenie COAX ventilu; nemá právomoc obísť bezpečnostné kontakty.

Schválená fyzická cesta:

    požiadavka Arduino/Mega
      → elektrický plavákový bezpečnostný kontakt
      → relé/COAX ventil
      → mechanický plavákový uzáver
      → bazén

Elektrický plavák je nadradená hardvérová ochrana proti preplneniu:

- jeho bezpečnostný kontakt je fyzicky v ovládacej ceste relé/COAX ventilu;
- pri maximálnej hladine preruší povolenie dopúšťania bez ohľadu na program;
- Arduino ho nesmie softvérovo obísť;
- COAX ventil nesmie dostať povolenie ani pri zamrznutí Arduina, zaseknutom výstupe, chybe programu alebo trvalom príkaze dopúšťať.

Za COAX ventilom je druhá, úplne mechanická ochrana podobná WC napúšťaciemu ventilu. Pri vysokej hladine fyzicky uzavrie prívod aj bez elektriny a bez funkčnej elektroniky.

Dôvod: jediná softvérová alebo elektrická porucha nesmie spôsobiť nekontrolované preplnenie. Elektrický plavák odoberá programu konečnú autoritu nad ventilom a mechanický plavák tvorí poslednú nezávislú poistku.

### T_FILL – plánované meranie teploty dopúšťacej vody

`T_FILL` je pracovný názov budúceho samostatného snímača teploty dopúšťacej/studničnej vody. Má merať prívodnú vodu ešte pred zmiešaním s vodou bazéna. Konkrétny typ senzora, fyzický kus, ROM adresa, MCU vlastník, zbernica a pin zatiaľ nie sú pridelené a zostávajú `TBD / NEURČENÉ`. Ak sa použije DS18B20, konkrétny kus a ROM sa zapíšu až po fyzickom potvrdení.

Plánovaný účel merania:

- zaznamenávať reálnu teplotu dopúšťanej vody;
- umožniť neskorší odhad tepelného dopadu dopustenia na bazén;
- korelovať teplotu prívodu so stavom dopúšťacieho ventilu, budúcim potvrdením prietoku a zmenou hladiny;
- podporiť diagnostiku, či pri povolenom dopúšťaní skutočne prichádza nová voda;
- uchovať údaj pre budúce energetické a prevádzkové štatistiky.

`T_FILL` je iba meracia a diagnostická vrstva, nie safety signál. Nenahrádza elektrický bezpečnostný plavák, mechanický plavákový uzáver ani budúce potvrdenie prietoku. Ak reálne dopúšťanie alebo prietok nie sú potvrdené, údaj môže predstavovať iba stojacu vodu v potrubí a nesmie sa automaticky považovať za aktuálnu teplotu zdroja. Plnohodnotné diagnostické vyhodnocovanie `T_FILL` je prípustné iba počas potvrdeného reálneho dopúšťania.

`T_FILL` nemení ani nesmie obísť schválenú fyzickú cestu `Arduino/Mega → elektrický plavákový bezpečnostný kontakt → relé/COAX ventil → mechanický plavákový uzáver → bazén`.

Historické ručné merania studničnej vody boli približne 12,8 °C dňa 20. 6. 2026 a približne 13,7 °C dňa 28. 6. 2026. Ide o jednotlivé reálne prevádzkové merania, nie o pevnú alebo garantovanú teplotu zdroja.

## Hydraulická topológia

Hlavný výstup vody z bazéna sa rozdeľuje na T-kuse do dvoch samostatných hydraulických vetiev:

    BAZÉN VÝSTUP
      → T-kus
        ├─ Vetva 1: filtrácia → chlorovač → návrat do bazéna
        └─ Vetva 2: samostatné solárne čerpadlo → X-kus

### Vetva 1 – filtrácia

    filtrácia → chlorovač → návrat do bazéna

Filtrácia a chlorovač tvoria vlastnú samostatnú hydraulickú vetvu. Horák/výmenník nie je zapojený za filtráciou v sérii.

### Vetva 2 – solár/ohrev

Samostatné solárne čerpadlo privádza vodu do X-kusu. Z X-kusu pokračujú tri vetvy:

1. Strešný solár – päť panelov.
2. Samostatný horák/výmenník.
3. T-kus, ktorý sa ďalej delí na chrlič a bazénový/spodný solár.

Schéma:

    samostatné solárne čerpadlo
      → X-kus
        ├─ strešný solár: 5 panelov
        ├─ horák/výmenník: ventil + 2 malé samostatné čerpadlá
        └─ T-kus
            ├─ chrlič
            └─ bazénový/spodný solár

Vetva horáka/výmenníka má dve malé samostatné čerpadlá. Tie umožňujú prevádzku horáka aj pri vypnutom hlavnom solárnom čerpadle. V tejto vetve je aj ventil.

Strešné aj bazénové solárne panely majú:

- rozdeľovače prietoku do jednotlivých panelov;
- samostatné zberače vody z jednotlivých panelov.

Solárne vetvy majú ventily na presmerovanie toku. Podľa polohy ventilov možno prietok presmerovať aj do chrliča.

Dôvod oddelenia: filtrácia/chlorovač má zostať nezávislá hydraulická cesta. Solár, horák/výmenník, chrlič a bazénový solár sa rozdeľujú až v samostatnej vetve za solárnym čerpadlom a X-kusom.

## Bazén, vodné okruhy a reálne prevádzkové parametre

Údaje v tejto sekcii sú potvrdené fyzické údaje zostavy a reálne prevádzkové pozorovania. Hodnoty označené ako reálne, približné alebo pozorované nie sú garantované výrobné parametre.

### Bazén

- Výrobca/typ: Intex.
- Približné rozmery: 4,5 × 2,2 × 0,84 m.
- Približný objem vody: 7,1 m³.

### Filtrácia

- Výrobca: Bestway.
- Reálne pozorovaný elektrický príkon: približne 380 W.
- Reálne pozorovaný prietok: približne 7–8 m³/h.

Uvedený príkon a prietok sú údaje z reálnej prevádzky zostavy, nie garantované výrobné parametre.

#### Bestway interný približne 6 h časovač – SCHVÁLENÉ FYZICKÉ PRAVIDLO / IMPLEMENTOVANÉ, ČAKÁ NA FYZICKÝ TEST

Filtrácia Bestway má vlastný interný približne 6-hodinový časovač. Dlhodobé držanie externého napájacieho/povoľovacieho relé ON preto samo negarantuje chod filtrácie dlhší ako približne 6 hodín. Ak SMART Mega požaduje pokračovanie filtrácie po dosiahnutí maximálneho súvislého ON intervalu, vykoná krátky power-cycle napájacej cesty filtrácie a následne pokračuje, pokiaľ stále existuje požiadavka na chod.

Produkčná Mega implementácia sleduje iba čas fyzického povolenia/napájania cez `MEGA_R9 / D22`; bez flow senzora netvrdí, že pozná skutočný mechanický chod alebo prietok čerpadla. Samostatné centrálne konštanty sú `FILTRACIA_MAX_SUVISLE_ON = 6 h` a pracovné `FILTRACIA_RESET_OFF_CAS = 2 000 ms`. Stavový automat `FIL_RUN_NORMAL → FIL_RESET_OFF → FIL_RUN_NORMAL` je neblokujúci a používa `millis()`:

- bez výslednej požiadavky je R9 OFF a continuous-run čas sa zruší;
- pri požiadavke je R9 ON a meria sa jeden súvislý interval fyzického povolenia;
- po približne 6 h a pri naďalej platnej požiadavke prejde R9 približne na 2 s OFF;
- po OFF intervale sa R9 zapne iba vtedy, ak požiadavka stále trvá;
- ak požiadavka počas resetovacieho OFF zanikne, reset sa zruší a R9 zostane OFF;
- po boote sa nevymýšľa predchádzajúca história a nový interval začína až prvým reálnym zapnutím R9.

Continuous-run ochrana je samostatná od `FILTRACIA_CAS_ON/OFF` millis fallbacku, RTC harmonogramu aj `FILTRACIA_MANUAL_6H_CAS`. Power-cycle nemení ani nereštartuje čas zdrojovej požiadavky. Príklad: filtrácia už beží 3 h a používateľ zapne `FIL MANUAL 6H`; približne po ďalších 3 h sa vykoná krátky power-cycle, ale manual požiadavka ďalej končí pôvodných 6 h od aktivácie, takže po resete zostávajú približne 3 h. Pri prirodzených RTC blokoch `00–06 ON`, `06–12 OFF`, `12–18 ON`, `18–24 OFF` nevzniká extra OFF→ON, pretože skončenie požiadavky má prednosť a prirodzené OFF už interný časovač preruší.

Implementácia nemení fyzickú mapu: `MEGA_R9 / D22 = filtrácia`, `MEGA_R10 / D23 = solár/chrlič`. Oba active-LOW piny dostanú pri boote bezpečný HIGH/OFF latch ešte pred `pinMode(OUTPUT)`, bez zmeny ďalšej regulácie alebo runtime správania. Budúci flow senzor môže potvrdiť skutočný chod, ale nie je súčasťou tohto checkpointu.

### Solárna zostava

- Samostatné solárne čerpadlo: približne 350 W podľa reálnej zostavy.
- Stav zostavy po úprave 6. 8. 2026:
  - päť panelov na streche;
  - tri panely dole pri bazéne;
  - spolu osem panelov.
- Typ panelov: Intex 28685.
- Približný rozmer jedného panelu: 1,2 × 1,2 m.
- Reálne pozorovaný prietok celou solárnou zostavou: približne 9–9,5 m³/h.
- Reálne namerané teploty na paneloch: približne 45–63 °C.

Prevádzkové skúsenosti:

- v lete dokázal solár bez horáka zvýšiť teplotu bazéna o niekoľko °C denne;
- počas horúčav bol častejším problémom nadmerný ohrev bazéna nad približne 32–34 °C než nedostatok solárneho výkonu.

Denník uvádza napríklad tieto denné nárasty počas dní so solárnou prevádzkou a priaznivým počasím:

- 16. 6.: približne 20,7 °C o 06:40 → 27,6 °C o 17:20;
- 17. 6.: približne 26,1 °C o 05:45 → 30,4 °C o 18:28;
- 27. 6.: približne 29,4 °C o 11:00 → 34,4 °C o 18:15;
- 13. 7.: približne 25,8 °C o 09:00 → 31,0 °C o 15:45;
- 14. 7.: približne 26,7 °C o 06:00 → 33,0 °C o 15:30.

Tieto rozdiely sú reálne zmeny teploty celej zostavy v daných podmienkach. Nie sú izolovaným meraním samotného výkonu solárnych panelov; ovplyvňovali ich počasie, zakrytie, kúpanie, dopúšťanie a prevádzkové zásahy.

Prehrievanie je doložené napríklad hodnotami 33,6–34,4 °C dňa 27. 6., 33,0 °C dňa 14. 7. a opakovanými hodnotami približne 32,6–32,8 °C v júli a auguste.

Rozsah panelov približne 45–63 °C potvrdzujú okrem iného merania 60,6 °C dňa 19. 6., 62,3 °C dňa 21. 6. a 63 °C dňa 29. 6.

Prietok, teploty a denné nárasty sú reálne merania alebo pozorovania konkrétnej zostavy. Nejde o garantované parametre panelov ani čerpadla.

### Návrat solárnej vody

Vratná hadica bola pôvodne umiestnená pri hladine. Dňa 15. 7. 2026 bola stiahnutá ku dnu bazéna.

Dôvod úpravy:

- lepšie premiešanie vody v celom objeme;
- zníženie teplotnej stratifikácie medzi hladinou a dnom.

Dňa 28. 6. po dopúšťaní bolo pri meraní na viacerých miestach zapísané, že rozdiel podľa hĺbky bol iba približne 0,1 °C. Po havárii hadice 11. 8. boli pri večernom meraní zaznamenané približne 26,6 °C pri vstupe a 26,8 °C na výstupe. Ide o jednotlivé prevádzkové pozorovania, nie o trvalú garanciu rovnomernosti.

Studničná voda použitá na dopúšťanie mala podľa denníka približne 12,8 °C dňa 20. 6. a približne 13,7 °C dňa 28. 6.

### Vývoj a poruchy solárnej hydrauliky

- 15. 7. 2026: návrat solárnej vody premiestnený od hladiny ku dnu.
- 26. 7.: zaznamenaný tečúci panel a odstavenie ohrevu.
- 28.–30. 7.: zaznamenané výmeny/ďalšie poruchy panelov; presná identita jednotlivých panelov je v RAW denníku nejednoznačná.
- 6. 8.: solár opravený, zostava prešla na konfiguráciu 5 strešných + 3 spodné panely; reálny prietok po úprave približne 9–9,5 m³/h.
- 11. 8.: utrhla sa hadica a uniklo približne 2,5 m³ vody. Oprava zahŕňala výmenu za hrubšie hadice a úpravu hladiny.

### Horák/výmenník

Horák/výmenník je samostatný rýchly zdroj tepla. Jeho hydraulická vetva je popísaná v predchádzajúcej sekcii a nie je zapojená sériovo za filtráciou.

- Staršie prevádzkové údaje uvádzajú približne 11 kW stabilného výkonu horáka/V2.
- Krátkodobé reálne testy naznačujú, že systém vedel preniesť aj vyšší tepelný výkon.
- Tieto hodnoty sú pozorovania konkrétnej zostavy, nie garantovaný výrobný výkon.

Reálne testy ohrevu:

| Test | Zmena teploty | Čas | Poznámka |
|---|---|---|---|
| 10. 7. | približne 15,5 °C o 11:40 → 28,7 °C o 18:40 | približne 7 h | ručne označený rýchly ohrev; palivo došlo o 18:10, presný podiel zdrojov tepla NEURČENÉ |
| 25. 7. | približne 21,5 °C o 10:00 → 30,3 °C o 13:50 | približne 3 h 50 min | reálny test celej zostavy; horák aj solár boli aktívne, spotreba približne 5–6 l paliva |

Pri teste 25. 7. nemožno celý nameraný tepelný účinok pripísať iba horáku, pretože počas testu bol aktívny aj solár. Výsledky záviseli od konkrétnych prevádzkových podmienok a nie sú garantovaným výkonom pre každý režim.

### Pomocné čerpadlá horáka

Vo vetve horáka/výmenníka sú dve malé záložné/samostatné čerpadlá. Umožňujú cirkuláciu cez horák aj bez chodu hlavného solárneho čerpadla.

- Počet: 2.
- Funkcia: samostatná cirkulácia cez horák/výmenník.
- Presný typ: NEURČENÉ.
- Elektrický príkon, napätie a prietok: NEURČENÉ.

## Kontrola horáka

Infračervený snímač plameňa je určený na kontrolu prítomnosti plameňa horáka pri ohreve bazéna cez samostatnú vetvu horáka/výmenníka. Konkrétne piny, poruchová logika a zásah do riadenia zatiaľ nie sú určené.

## Heartbeat, ACK a komunikácia

- Heartbeat Mega→Uno a Uno→Mega budú samostatné fyzické signály.
- Starý softvérový heartbeat na fyzicky nepoužitom Mega D44 bol odstránený; D44 je aktuálne voľný a nemá protokolovú ani safety funkciu.
- Budúci fyzický RESET/ACK nesmie zmazať aktívnu príčinu. Resetovanie bude obmedzené, po vyčerpaní pokusov vznikne `RESET_LOCKOUT` a systém zostane v BASIC, ak nie je aktívna samostatná kritická fyzická podmienka vyžadujúca STOP.
- Diagnostická komunikácia Mega↔Uno používa priame TTL Serial2 D16/D17 ↔ Uno D7/D8 pri 38400 Bd, neinvertovane a cez sériový 10 kΩ v každom smere. Safety heartbeat a SMART/BASIC dohoda zostávajú samostatná budúca vrstva.
- Výpadok jedného senzora nesmie zastaviť ostatné merania.
- Budúca architektúra nepoužíva blokujúce čakanie.

## Napájacia architektúra

Stavy napájacích prvkov sa rozlišujú na FYZICKY NAMONTOVANÉ, SKLADOM/NA CESTE a PLÁNOVANÉ.

### FYZICKY NAMONTOVANÉ

- Hlavný zdroj systému: Siemens SITOP PSU200M, objednávacie číslo 6EP1334-3BA10.
- LM2596HVS: existujúca samostatná napájacia vetva watchdogu.
- DC/DC CC/CV buck menič do približne 300 W, kus č. 1: výstup 7,5 V pre Arduino Mega.
- DC/DC CC/CV buck menič do približne 300 W, kus č. 2: výstup 5 V pre 16R relé/BASIC vetvu.
- Samostatný LM2596: výstup 7,5 V pre Arduino Uno. Táto vetva nie je watchdog LM2596HVS ani samostatný nový LM2596 pre 12 V W1209.
- Samostatný nový LM2596: výstup 12 V pre W1209 BASIC vetvu.

Aktuálny tok napájania pred doplnením plánovanej batériovej zostavy:

    Siemens SITOP PSU200M
      ├─ veľký CC/CV buck #1 → 7,5 V → Arduino Mega
      ├─ veľký CC/CV buck #2 → 5 V → 16R relé + BASIC vetva
      ├─ samostatný LM2596 → 7,5 V → Arduino Uno
      ├─ LM2596HVS → watchdog
      └─ samostatný nový LM2596 → 12 V → W1209 BASIC vetva

Dva veľké CC/CV meniče do približne 300 W sú už súčasťou zostavy. Nesmú sa zamieňať s dvoma novo kúpenými malými LM2596.

### SKLADOM / evidencia pôvodu NEURČENÁ

- Historický nákup 2× LM2596 sa už nesmie používať ako tvrdenie, že oba kusy sú nenamontované. Aktuálne sú fyzicky potvrdené samostatná 7,5 V LM2596 vetva Una a samostatná nová 12 V LM2596 vetva W1209; presné priradenie týchto fyzických kusov k pôvodným nákupným dávkam a zostávajúca skladová zásoba sú `NEURČENÉ`.
- Funkčný ESP-01S odstránený z Uno zostavy; budúce použitie `NEURČENÉ`.
- Samostatná 3,30 V vetva pôvodného ESP-01S nie je aktívnou súčasťou Uno zostavy; ďalšie použitie `NEURČENÉ`.
- Presný vzťah kusov LM2596 k bývalej ESP vetve nie je potvrdený.

### PLÁNOVANÁ batériová/UPS vrstva

Celá 3S batériová zostava s kapacitou približne 500 Wh je zatiaľ PLÁNOVANÁ a nie je fyzicky postavená.

Schválená topológia:

    SITOP 24 V
      → DC/DC CC/CV 12,2 V / max. 12 A
      → hardvérový doraz nabíjania cez programovaciu dosku s relé
      → 3S BMS
      → 3S batéria približne 500 Wh
      → existujúce a budúce výstupné DC/DC vetvy

Z batériovej vetvy budú napájané:

- existujúci veľký CC/CV buck #1, 7,5 V pre Mega;
- existujúci veľký CC/CV buck #2, 5 V pre 16R relé/BASIC;
- samostatný LM2596, 7,5 V pre Uno;
- existujúca samostatná watchdog vetva LM2596HVS;
- samostatný nový LM2596, 12 V pre W1209 BASIC vetvu;
- neskôr plánovaný step-up na 24 V;
- neskôr plánovaná 3,3 V vetva.

Súčasťou 3S zostavy bude aktívne balansovanie 5 A.

Nejde o prepínanie SITOP ↔ batéria. Batériová zostava bude vložená priamo do napájacej cesty. Pri strate vstupu zo SITOP má napájanie pokračovať z batérie bez samostatného prepínacieho režimu.

Zatiaľ nie sú potvrdené a nesmú sa domýšľať:

- typ článkov;
- konfigurácia paralelných skupín;
- konkrétny model 3S BMS;
- konkrétny model step-up meniča 24 V;
- poistky a ich hodnoty;
- konkrétny model programovacej dosky s relé.

### Dôvody rozdelenia

- Mega dostáva definovanú samostatnú 7,5 V vetvu.
- Uno dostáva vlastnú samostatnú 7,5 V vetvu z LM2596.
- Druhý veľký CC/CV menič poskytuje 5 V iba pre 16R relé/BASIC vetvu.
- Watchdog má samostatnú existujúcu vetvu, aby nebol zamieňaný s napájaním riadiacich jednotiek.
- Evidenčné oddelenie typov meničov zabraňuje zámene fyzicky namontovaných výkonových CC/CV modulov s nenamontovanými pomocnými LM2596.
- Vložená batériová vrstva má zabezpečiť kontinuitu bez osobitného prepínania zdrojov.

## Otvorené rozhodnutia

- piny budúcich, zatiaľ neschválených funkcií Mega/Uno; dnešné D30/D31/D32/D33 a Uno A0/A2/D9 sú pridelené podľa PINOUT;
- budúca samostatná safety heartbeat vrstva; aktuálna Mega↔Uno UART vrstva je priame TTL 38400 Bd cez dva sériové 10 kΩ odpory, spoločnú GND a bez spoločného +5 V;
- reprodukcia jednorazového externého incidentu Uno D9, pri ktorom softvér hlásil agreement ON, ale pripojená vetva stiahla D9 približne na 0 V; presná H/L/napájacia/backfeed/spojová príčina zostáva nepotvrdená;
- autonómna BASIC logika a podmienky BASIC_R3/R4;
- prípadné budúce použitie skladovaného ESP-01S zostáva `NEURČENÉ`;
- fyzický test druhej MicroSD vrstvy plánovanej pre Mega;
- účel druhého novo kúpeného LM2596; dve aktuálne XKC read paths používajú dva optočlenové kanály, ďalšie použitie zostávajúcich PC817 kanálov po odstránení PC817 z UART dátovej cesty je `NEURČENÉ`;
- typ článkov, paralelné skupiny, model BMS, model 24 V step-up a poistky plánovanej UPS;
- konkrétne elektrické zapojenie dopúšťania, typ COAX ventilu, riadiaci pin a diagnostika plavákov;
- piny a poruchová logika infračerveného snímača plameňa.

## ESP Wi-Fi/web HMI – implementované vo firmvéri, čaká na fyzický test (2026-08-31)

- ESP web HMI prijíma rozšíriteľnú telemetriu Mega cez existujúci Serial3 link; riadiace príkazy zostávajú iba `SETTEMP=`, `FIL6H=TOGGLE` a `CHR1H=TOGGLE`.
- Hlavná stránka zobrazuje Mega T1–T4, TBOX, AHT10 OUT/RH, efektívnu regulačnú `teplotaBazena` s `POOL_TEMP_VALID` vrátane platného UNO_T1 fallbacku, limity a nastavenie MAX_BAZEN, režim systému, R9/R10 súvisiace stavy a Mega↔Uno link/agreement.
- ESP web HMI má diagnostickú stránku `/errors` a stránku `/logs`. Nezasahuje do TOTAL STOP, SMART/BASIC agreementu ani regulácie.
- Denné teplotné CSV logy sú implementované v LittleFS ESP v intervale 60 s. Súbory majú tvar `/log_YYYYMMDD.csv`; automatické mazanie starších súborov nie je implementované. Pri nedostupnom LittleFS alebo chybe zápisu pokračuje HMI aj Mega/Uno bez zmeny riadenia; stav logovania je diagnostický.
- Uno SD logger sa týmto nemení a ostáva nezávislou diagnostickou/bezpečnostnou vrstvou. UNO chrlič ani UNO preplach nie sú v aktuálnom kóde/protokole implementované; web ich nesmie prezentovať ako funkčné ovládanie.
- Predvolená Wi-Fi konfigurácia ESP firmvéru je `STRONG`; fyzické pripojenie novej siete a dostupná veľkosť LittleFS sú `NEOVERENÉ`.
