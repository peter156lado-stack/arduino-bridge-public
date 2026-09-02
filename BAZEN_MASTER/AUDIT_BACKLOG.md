# BAZEN AUDIT BACKLOG

Tento dokument je živý register auditných nálezov, otvorených technických rizík a odložených opráv. Nie je to CHANGELOG a nenahrádza MASTER_KONCEPT.

## AUDIT 2026-09-02

- **Dátum:** 2026-09-02
- **Rozsah:** produkčné projekty Mega, Uno a ESP8266; pinové a fyzické autority; safety architektúra; fallbacky a diagnostika; megaStav, SystemMode a agreement; UART V5; výstupy a regulácia; W1209/BASIC; RAM a runtime riziká; dokumentácia; historický kód; Wi-Fi/HMI/bridge.
- **Poradie zdrojov pravdy:** fyzicky potvrdený stav > aktuálny pracovný kód > dokumentácia > historické poznámky.
- **Charakter auditu:** audit bol read-only. Následný HOME SAFE FIX PACK uzavrel iba presne uvedené softvérové body; ostatné nálezy zostávajú bez automatického preklasifikovania.

### Povolené hodnoty STATUS

- **OPEN** – potvrdený alebo relevantný otvorený problém.
- **FIXED_SOFTWARE** – softvérová časť bola implementovaná a overená; prípadný samostatný fyzický test môže zostať v physical backlogu.
- **WAITING_PHYSICAL_TEST** – rozhodnutie alebo uzavretie závisí od fyzického merania/commissioningu.
- **ACCEPTED_RESIDUAL_RISK** – vedome prijaté zvyškové riziko.
- **PLANNED** – schválený budúci rozsah, nie aktuálna porucha.
- **NOT_A_PROBLEM** – auditom alebo spresnením potvrdené ako správny zámer.
- **DEFERRED** – oprava alebo rozhodnutie je vedome odložené.

### Súhrn stavov 28 nálezov

| STATUS | Počet |
|---|---:|
| OPEN | 12 |
| FIXED_SOFTWARE | 6 |
| DEFERRED | 5 |
| WAITING_PHYSICAL_TEST | 1 |
| PLANNED | 2 |
| ACCEPTED_RESIDUAL_RISK | 2 |
| NOT_A_PROBLEM | 0 |
| **Spolu** | **28** |

> **Spresnenie rezervných relé:** Mega R5–R8 a R11–R16 sú **PLANNED / RESERVED OUTPUT – OK**. Ich existencia bez runtime logiky nie je dead code ani cleanup problém. Nález 28 sa na ne nevzťahuje. Toto spresnenie má stav **NOT_A_PROBLEM**, ale nie je samostatným auditným nálezom a preto nemení počet 28.

## AUDIT FINDINGS – 2026-09-02

### 1. I2C môže zablokovať lokálnu Mega safety

- **STATUS:** OPEN
- **Závažnosť:** HIGH
- **Problém:** Inicializácia a obsluha RTC/AHT10/I2C vrátane skenu zbernice prebieha bez potvrdeného Wire timeoutu a časť z nej je pred plnou inicializáciou lokálnej XKC/TOTAL STOP safety.
- **Dopad:** Zaseknutá I2C zbernica môže zdržať alebo zastaviť safety inicializáciu či pravidelné vyhodnocovanie.
- **Istota:** Absencia timeoutu a poradie v kóde sú isté; správanie konkrétneho hardvéru pri stuck-bus je **NEOVERENÉ**.
- **Odporúčaný smer:** Samostatne schváliť I2C timeout/recovery a skoršiu safety inicializáciu; následne fyzicky otestovať stuck-bus.

### 2. Remote T1/T2/T3 nemali rolovú validáciu na oboch stranách

- **STATUS:** FIXED_SOFTWARE
- **Závažnosť:** HIGH
- **Problém:** Uno pôvodne používalo všeobecný DS18B20 rozsah a Mega dôverovalo vzdialenému validity bitu bez vlastnej rolovej revalidácie.
- **Dopad:** Hodnota platná ako všeobecná DS teplota, ale neplatná pre konkrétnu procesnú rolu, mohla byť použitá ako fallback.
- **Istota:** Isté; potvrdené auditom kódu.
- **Odporúčaný smer:** Zachovať spoločné rolové limity a nezávislú revalidáciu na Mega.
- **Uzavretie 2026-09-02:** Uno T1 používa bazénový rozsah Mega, Uno T2/T3 solárny rozsah Mega a Mega každú prijatú UNO_T1/T2/T3 hodnotu pred fallbackom znovu rolovo validuje. Poradie fallbackov sa nezmenilo.

### 3. Webové command endpointy nemajú autentifikáciu

- **STATUS:** OPEN
- **Závažnosť:** CRITICAL
- **Problém:** ESP endpointy pre zmenu teploty, FIL 6 h, chrlič a konfiguráciu nemajú autentifikáciu/autorizáciu; online stav Mega nie je bezpečnostná kontrola klienta.
- **Dopad:** Klient s prístupom do príslušnej LAN/AP môže meniť prevádzkové príkazy alebo konfiguráciu.
- **Istota:** Stav kódu je istý; reálna sieťová dostupnosť a izolácia sú **NEOVERENÉ**.
- **Odporúčaný smer:** Navrhnúť primeranú autentifikáciu, autorizáciu a ochranu command požiadaviek bez pridania controller authority ESP.

### 4. Wi-Fi/AP secrets sú uložené priamo v ESP zdroji

- **STATUS:** OPEN
- **Závažnosť:** CRITICAL, podmienené expozíciou
- **Problém:** Sieťové prihlasovacie údaje sú v plaintext podobe v produkčnom ESP sketchi.
- **Dopad:** Pri zdieľaní zdroja alebo úniku snapshotu môžu sprístupniť LAN/AP.
- **Istota:** Uloženie v zdroji je isté; platnosť údajov a ich predchádzajúca publikácia sú **NEOVERENÉ**.
- **Odporúčaný smer:** Overiť expozíciu, podľa potreby údaje rotovať a oddeliť lokálne secrets od publikovaného zdroja.

### 5. LCD mohlo počas TOTAL STOP zobrazovať SYS:OK

- **STATUS:** FIXED_SOFTWARE
- **Závažnosť:** HIGH
- **Problém:** Nadradený LCD stav bol odvodený zo staršieho health súhrnu a problémový výpis nepoznal lokálny XKC trip.
- **Dopad:** Pri MODE_STOP/D32 mohol používateľ vidieť SYS:OK, SYSTEM:OK alebo ZIADNA AKTIVNA.
- **Istota:** Isté; potvrdené auditom kódu.
- **Odporúčaný smer:** SystemMode ponechať jediným zdrojom nadradeného režimu a explicitne zobrazovať safety príčinu.
- **Uzavretie 2026-09-02:** LCD používa autoritatívny SystemMode; lokálny megaXkcTrip má diagnostickú STOP prioritu a XKC LOW WATER/TOTAL STOP už nemôže skončiť ako OK. USB súhrn PROBLEM pozná XKC_LOW_WATER_TOTAL_STOP.

### 6. ESP diagnostika nevie vysvetliť XKC TOTAL STOP

- **STATUS:** OPEN
- **Závažnosť:** HIGH
- **Problém:** ESP dostáva režim, ale nemá úplnú diagnostickú informáciu o lokálnom XKC tripe a fyzických D32/A0/D9 príčinách.
- **Dopad:** Web môže vidieť STOP bez jednoznačného vysvetlenia jeho lokálnej safety príčiny.
- **Istota:** Isté na úrovni prenášanej telemetrie; používateľský dopad závisí od konkrétnej situácie.
- **Odporúčaný smer:** V budúcom samostatne schválenom diagnostickom rozšírení preniesť príčinu bez vytvorenia Wi-Fi controller authority.

### 7. SystemMode nie je všeobecný gate R9/R10 a override ciest

- **STATUS:** ACCEPTED_RESIDUAL_RISK
- **Závažnosť:** HIGH, fyzicky podmienené
- **Pôvodný problém:** Regulácia, manual/test override a fyzické SMART výstupy nepoužívajú SystemMode ako všeobecný softvérový output gate. SMART command alebo reléový výstup preto môže pri prechode do BASIC zostať ON.
- **Fyzicky potvrdený dopad:** **COMMAND/RELAY STATE != PHYSICAL POWER AUTHORITY.** Po fyzickom prerušení Mega↔Uno komunikácie systém po timeoute prešiel zo SMART do BASIC, agreement/povoľovacia cesta odpadla a aktívne SMART relé mohli zostať commanded/fyzicky zopnuté na vlastnom výstupe, ale ich výkonová/povoľovacia vetva stratila prívod. Nemali preto fyzickú autoritu ovládať zariadenie a BASIC vetva zostala autoritatívna.
- **Istota:** **PHYSICAL MODE ISOLATION CONFIRMED 2026-09-02.** Test prešiel na reálnom zapojení a po obnovení komunikácie systém normálne dokončil recovery späť.
- **Architektonické rozhodnutie:** Aktuálna architektúra zámerne odoberá SMART fyzickú povoľovaciu/napájaciu autoritu namiesto povinného softvérového nulovania všetkých SMART commandov. Prevádzkovateľ toto správanie považuje za správne a akceptuje ho.
- **Residual behavior:** Ak pri obnovení SMART/povoľovacej vetvy stále platí ON command, môže bez nového ON príkazu znovu nadobudnúť fyzickú autoritu. Ide o známe a prevádzkovateľom akceptované správanie aktuálnej architektúry.
- **Odporúčaný smer:** Nález je uzavretý bez zmeny SystemMode, R9/R10 alebo BASIC/SMART logiky. Znovu ho otvoriť iba pri zmene fyzickej povoľovacej architektúry alebo požiadavky na command reset pri recovery.
- **STATUS TESTU:** PHYSICALLY_CONFIRMED / ARCHITECTURALLY_ACCEPTED / CLOSED
- **Taxonomická poznámka:** Hlavný status používa existujúce ACCEPTED_RESIDUAL_RISK; nejde o FIXED_SOFTWARE a R9/R10 nie sú v BASIC softvérovo nútene OFF.

### 8. Active-LOW R9/R10 mali neistý boot latch

- **STATUS:** FIXED_SOFTWARE
- **Závažnosť:** MEDIUM
- **Problém:** pinMode(OUTPUT) bolo vykonané pred nastavením bezpečnej HIGH úrovne.
- **Dopad:** Mohol vzniknúť krátky LOW impulz a neželané zopnutie active-LOW relé počas bootu.
- **Istota:** Poradie v starom kóde bolo isté; reálne zopnutie relé nebolo fyzicky zmerané.
- **Odporúčaný smer:** Zachovať poradie HIGH latch pred OUTPUT a fyzicky overiť boot osciloskopom/logickým analyzátorom.
- **Uzavretie 2026-09-02:** R9/D22 a R10/D23 dostanú HIGH latch pred pinMode(OUTPUT). Runtime regulácia sa nezmenila.

### 9. Mega D32 TOTAL STOP sa inicializuje neskoro

- **STATUS:** DEFERRED
- **Závažnosť:** HIGH
- **Problém:** D32 sa nastaví až po časti RTC/I2C/AHT a ďalšej inicializácie, takže predtým môže zostať Hi-Z.
- **Dopad:** Pri pomalom alebo zablokovanom boote môže byť lokálna TOTAL STOP autorita pripravená neskoro.
- **Istota:** Poradie kódu je isté; reálne správanie vstupu H/L modulu počas Hi-Z je **NEOVERENÉ**.
- **Odporúčaný smer:** Riešiť spolu s bodom 1 ako osobitne schválenú zmenu poradia safety inicializácie.

### 10. Agreement D31/D9 nie je hardvérový pulzný watchdog

- **STATUS:** DEFERRED
- **Závažnosť:** HIGH, architektonické
- **Problém:** Agreement je statická úroveň. Zamrznutý procesor môže držať pin HIGH; druhá doska stav zhodí až po komunikačnom timeoute, nie okamžite samotným zastavením CPU.
- **Dopad:** Tvrdenie o okamžitej reakcii na freeze nie je všeobecne garantované.
- **Istota:** Softvérové správanie je isté; konkrétne freeze scenáre sú **NEOVERENÉ**.
- **Odporúčaný smer:** Zosúladiť očakávania dokumentácie a prípadný nezávislý heartbeat/watchdog riešiť iba po samostatnom schválení architektúry.

### 11. Reset počas trvalého LOW WATER dočasne uvoľní vlastný trip

- **STATUS:** ACCEPTED_RESIDUAL_RISK
- **Závažnosť:** HIGH
- **Problém:** Po resete začína lokálne XKC potvrdenie od nuly a nový trip vznikne až po 5 s súvislého LOW WATER.
- **Dopad:** Pri resete jednej dosky zostáva spoločná ochranná cesta blokovaná tripom druhej dosky. Pri súčasnom resete Mega aj Uno vzniká fyzicky potvrdené permissive okno približne 2–3 s, počas ktorého by motorová cesta mohla byť povolená; potom oba kontroléry znovu vyhodnotia trvajúci LOW WATER a TOTAL STOP opäť aktivujú.
- **Istota:** **PHYSICALLY_CONFIRMED 2026-09-02.** Samostatný reset Uno aj samostatný reset Mega prešli; permissive okno pri súčasnom resete bolo fyzicky pozorované.
- **Odporúčaný smer:** Známe reziduálne riziko je prevádzkovateľom pre aktuálnu aplikáciu akceptované. Nález nie je FIXED a produkčný kód sa nemení. Prípadné budúce odstránenie okna by vyžadovalo osobitne schválenú perzistentnú alebo nezávislú ochranu.
- **STATUS TESTU:** PHYSICALLY_CONFIRMED / ACCEPTED_RESIDUAL_RISK

### 12. XKC súvislý čas je meraný iba vzorkovaním loopu

- **STATUS:** WAITING_PHYSICAL_TEST
- **Závažnosť:** MEDIUM
- **Problém:** Krátke zmeny XKC, ktoré celé prebehnú počas blokujúcej časti loopu, nemusia byť zaznamenané.
- **Dopad:** Skutočný čas potvrdenia alebo recovery sa môže líšiť od ideálneho priebehu pri dlhom loop čase.
- **Istota:** Vlastnosť polling implementácie je istá; reálna dynamika XKC a najhorší loop sú **NEOVERENÉ**.
- **Odporúčaný smer:** Zmerať worst-loop a priebeh XKC; až podľa výsledkov posudzovať potrebu zmeny.

### 13. Kritická FIL_CONTROL_VALID sa do lokálneho agreementu premieta cez Uno

- **STATUS:** DEFERRED
- **Závažnosť:** MEDIUM, latentné
- **Problém:** Mega vysiela megaStav=2, ale lokálny režim/agreement nemá samostatnú aktívnu kritickú FIL vetvu; reakcia D9 vzniká po prijatí na Uno.
- **Dopad:** Pri budúcej reálne implementovanej strate FIL control dôvery bude treba znovu overiť lokálnu aj vzdialenú reakciu.
- **Istota:** Aktuálna štruktúra je istá; FIL_CONTROL_VALID je dnes stále true.
- **Odporúčaný smer:** Nevymýšľať fault flagy. Pri schválení prvej reálnej FIL kritickej podmienky súčasne určiť lokálny SystemMode/agreement dopad.

### 14. megaStav=2 je v dnešnom rozsahu prakticky nedosiahnuteľný

- **STATUS:** PLANNED
- **Závažnosť:** INFO / latentné
- **Problém:** Jediná schválená kritická podmienka FIL_CONTROL_VALID je aktuálne konštantne platná.
- **Dopad:** Kanál megaStav=2 je zatiaľ pripravená štruktúra, nie aktívna detekcia novej control poruchy.
- **Istota:** Isté.
- **Odporúčaný smer:** Zachovať význam megaStav a pridať konkrétnu kritickú podmienku až po jej samostatnom schválení a implementácii.

### 15. V5 RTC validita sa líšila od validity filtrácie

- **STATUS:** FIXED_SOFTWARE
- **Závažnosť:** MEDIUM
- **Problém:** V5 validity pôvodne vychádzala iba z čitateľnosti RTC, zatiaľ čo filtrácia kontrolovala aj OSF a rozsahy.
- **Dopad:** Uno mohlo synchronizovať čas, ktorý Mega nepovažovalo za autoritatívny pre filtráciu.
- **Istota:** Isté; potvrdené auditom kódu.
- **Odporúčaný smer:** Používať jeden spoločný autoritatívny helper.
- **Uzavretie 2026-09-02:** V5 aj filtrácia používajú rtcCasJePlatny(), teda čitateľnosť, existujúcu OSF kontrolu a existujúce rozsahy.

### 16. RTC 12-hodinové dekódovanie má hraničný problém pri 12 AM

- **STATUS:** DEFERRED
- **Závažnosť:** LOW
- **Problém:** Pri externom prepnutí RTC do 12-hodinového režimu môže byť 12 AM interpretované ako 12 namiesto 0.
- **Dopad:** Nesprávny čas filtrácie alebo logovania v tejto konfigurácii.
- **Istota:** Kódová hrana je istá; použitie 12-hodinového režimu v reálnom RTC je **NEOVERENÉ**.
- **Odporúčaný smer:** Opraviť iba ak sa 12-hodinový režim podporuje alebo fyzicky zistí.

### 17. UART V5 prijímače nekontrolovali reserved validity bity

- **STATUS:** FIXED_SOFTWARE
- **Závažnosť:** MEDIUM/LOW
- **Problém:** Mega a Uno mohli prijať rámec s nepovolenými reserved validity bitmi.
- **Dopad:** Nekompatibilný alebo poškodený budúci rámec mohol prejsť validáciou napriek neznámej sémantike.
- **Istota:** Isté; potvrdené auditom kódu.
- **Odporúčaný smer:** Zachovať explicitné masky povolených bitov bez zmeny V5 formátu.
- **Uzavretie 2026-09-02:** Oba prijímače odmietajú nepovolené reserved validity bity; legitímny V5 rámec zostal prijateľný.

### 18. Uno BLACK BOX nezaznamenáva lokálne XKC/A0/D9

- **STATUS:** OPEN
- **Závažnosť:** HIGH, diagnostické
- **Problém:** CSV/event podpis neobsahuje plný lokálny Uno XKC stav, lokálny trip, A0 TOTAL STOP a D9 agreement; časť vzdialenej diagnostiky sa maskuje alebo zlučuje.
- **Dopad:** Po incidente nemusí byť možné jednoznačne rekonštruovať lokálnu safety príčinu a časovanie.
- **Istota:** Isté; potvrdené auditom schémy a eventov.
- **Odporúčaný smer:** Samostatne navrhnúť minimálne diagnostické polia/event bity; schema sa nesmie meniť bez výslovného schválenia.

### 19. Uno SRAM, SD stack a blokovanie nie sú zmerané

- **STATUS:** OPEN
- **Závažnosť:** HIGH/MEDIUM
- **Problém:** Po kompilácii zostáva približne 796 B SRAM; SD knižnica, File objekty, buffery a stack majú runtime nároky a SD operácie môžu blokovať.
- **Dopad:** Stack/heap kolízia alebo dlhý SD prístup môže poškodiť dáta, UART timing alebo lokálnu safety obsluhu.
- **Istota:** Statická RAM rezerva je istá. Boot/run bez SD karty je fyzicky overený ako funkčný; stack watermark, worst-case loop a blokovanie pri chybových SD operáciách zostávajú **NEOVERENÉ**.
- **Odporúčaný smer:** Zmerať stack watermark, SD fault timing a realistický worst-loop bez zmeny BLACK BOX schémy.
- **Fyzická podčasť 2026-09-02:** **SD ABSENT AT BOOT / RUN = PHYSICAL PASS.**
- **Boot bez SD:** Uno bolo zapnuté s fyzicky vybratou SD kartou. Korektne vypísalo SD: CHYBA INICIALIZACIE - LOGGER DEAKTIVOVANY a pokračovalo na UNO START V5.
- **Beh bez SD:** UNO=OK, LINK=OK; T1/T2/T3/TBOX, sonar a XKC zostali funkčné. SD=CHYBA zostalo iba diagnostickým stavom.
- **Automatický recovery:** Opakované SD: POKUS O OBNOVU a následné SD: CHYBA INICIALIZACIE - LOGGER DEAKTIVOVANY nespôsobili pozorovaný reset, freeze ani link loss.
- **Link počas testu:** Počítadlá zostali prakticky stabilné približne CRC=1, INV=0, TO=0, GAP=1.
- **Agreement:** Po 180 s normálne vzniklo RECOVERY: UNO_AGREEMENT=ON a následne UNO=OK LINK=OK AGR=ON SD=CHYBA.
- **Rozsah potvrdenia:** Neprítomnosť SD nebráni bootu ani prevádzke Uno supervisora a SD logger nie je podmienkou LINK, XKC ani agreement. Tento čiastkový PASS neuzatvára AUDIT #19.

### 20. SoftwareSerial a OneWire majú potvrdené timing riziko

- **STATUS:** OPEN
- **Závažnosť:** MEDIUM
- **Problém:** Interval 5300 ms znižuje fázové kolízie, ale pri dlhých OneWire operáciách a opakovanom sensors.begin() ich úplne nevylučuje.
- **Dopad:** Pri trvalej DS18B20 chybe sa zvyšuje miera poškodených/neplatných alebo vynechaných V5 rámcov. V commissioning teste nenastal úplný link loss ani agreement loss, ale znížila sa komunikačná rezerva.
- **Istota:** **PHYSICALLY_REPRODUCED_SYMPTOM / ROOT_CAUSE_NOT_PROVEN.** Silná časová korelácia s DS fault je potvrdená; presný mechanizmus, napríklad OneWire/sensors.begin() verzus SoftwareSerial timing collision, týmto testom definitívne dokázaný nebol. Jednotlivé link chyby existujú aj mimo DS fault.
- **Odporúčaný smer:** Nález ponechať OPEN. Pri ďalšom samostatne schválenom diagnostickom teste časovo korelovať OneWire recovery, interrupt-off úseky a V5 chyby; nemeniť architektúru ani timing bez merania príčiny.
- **Fyzický test 2026-09-02 – východisko:** Na bežiacom Uno bol odpojený UNO_TBOX DS18B20 a porucha zostala aktívna niekoľko minút. Pred TBOX_CHYBA už existovalo malé pozadie jednotlivých V5 chýb, približne CRC=0, INV=2, TO=0, GAP=3.
- **Správanie počas poruchy:** Po EVENT: TBOX_CHYBA prešlo Uno korektne na UNO=DEGRADED a TBOX=ERR. LINK zostal OK, AGR zostal ON, TO zostal 0, T1/T2/T3 aj lokálna XKC zostali funkčné.
- **Pozorované počítadlá:** Počas trvalej chyby narástli približne CRC 0→2, INV 2→4 a GAP 3→10; TO zostalo 0.
- **Recovery bez resetu:** Po opätovnom pripojení TBOX vzniklo RECOVERY: TBOX_OK a Uno sa bez resetu vrátilo na UNO=OK LINK=OK AGR=ON. Nárast chýb sa výrazne utíšil: CRC/INV zostali 2/4, GAP dlho 10 a neskôr pribudol iba jeden na 11.

### 21. Mega obsahuje commissioning TEST R9 a neobmedzený String vstup

- **STATUS:** OPEN
- **Závažnosť:** MEDIUM
- **Problém:** Sériová TEST cesta môže priamo ovládať R9 mimo SystemMode a vstupný String môže bez ukončenia rásť.
- **Dopad:** Servisný vstup môže obísť očakávaný režim a dlhý vstup môže fragmentovať alebo vyčerpať heap.
- **Istota:** Isté; potvrdené auditom kódu.
- **Odporúčaný smer:** Rozhodnúť, či commissioning cesta zostáva produkčne potrebná; ak áno, ohraničiť vstup a schváliť safety podmienky.

### 22. Wi-Fi BAZEN ignoroval platný UNO_T1 fallback

- **STATUS:** FIXED_SOFTWARE
- **Závažnosť:** MEDIUM, diagnostické
- **Problém:** Webová validita bazéna bola odvodená len z lokálnych T1/T4 a nepoužívala autoritatívny POOL_TEMP_VALID.
- **Dopad:** Web mohol zobrazovať N/A, hoci Mega bezpečne používalo platný UNO_T1 fallback.
- **Istota:** Isté; potvrdené auditom kódu.
- **Odporúčaný smer:** Používať rovnakú efektívnu hodnotu a validitu ako control vrstva.
- **Uzavretie 2026-09-02:** Wi-Fi pole BAZEN používa POOL_TEMP_VALID a efektívnu teplotaBazena; nepribudla žiadna command ani safety autorita.

### 23. Web TOGGLE príkazy nie sú idempotentné ani potvrdené

- **STATUS:** OPEN
- **Závažnosť:** HIGH/MEDIUM
- **Problém:** TOGGLE požiadavky nemajú command ID ani potvrdený výsledný stav; opakovanie alebo dvojklik vykoná ďalší toggle.
- **Dopad:** Retry, latencia alebo dvojklik môže skončiť opačným stavom, než používateľ zamýšľal.
- **Istota:** Isté; potvrdené protokolom a obsluhou.
- **Odporúčaný smer:** Budúco používať idempotentný desired-state príkaz a potvrdenie, bez rozšírenia authority ESP.

### 24. Uno soft clock nie je bezpečný po viac než jednom millis rollover bez sync

- **STATUS:** OPEN
- **Závažnosť:** MEDIUM/LOW
- **Problém:** Soft clock počíta od jedného millis základu a po približne 49,7 dňa bez novej RTC synchronizácie sa delta pretočí.
- **Dopad:** BLACK BOX čas môže skočiť alebo byť nesprávny pri dlhom výpadku synchronizácie.
- **Istota:** Isté z aritmetiky; taký dlhý reálny výpadok sync je **NEOVERENÝ**.
- **Odporúčaný smer:** Použiť priebežne akumulovanú rollover-safe časovú bázu.

### 25. ESP LittleFS logy nemajú retention politiku

- **STATUS:** PLANNED
- **Závažnosť:** LOW
- **Problém:** Logy sa priebežne pridávajú bez potvrdeného limitu alebo rotácie.
- **Dopad:** LittleFS sa časom môže zaplniť a prestať prijímať nové záznamy.
- **Istota:** Absencia retention je istá; čas do zaplnenia je **NEOVERENÝ**.
- **Odporúčaný smer:** Navrhnúť limit, rotáciu alebo mazanie najstarších logov ako samostatnú údržbovú funkciu.

### 26. Dokumentácia obsahuje aktuálne pôsobiace stale údaje

- **STATUS:** OPEN
- **Závažnosť:** MEDIUM
- **Problém:** V MASTER/PINOUT/HARDWARE a komentároch zostali miesta s monitor-only XKC, staršími názvami/verziami alebo stavom V5 pending, ktoré môžu pôsobiť ako aktuálne.
- **Dopad:** Budúca oprava alebo commissioning sa môže riadiť nesprávnym opisom.
- **Istota:** Isté; konkrétne historické záznamy v CHANGELOG sa majú zachovať ako história.
- **Odporúčaný smer:** Urobiť samostatný consistency pass, jasne označiť historické tvrdenia a nemeniteľné fyzické potvrdenia. Fix pack upravil iba priamo dotknuté pasáže.

### 27. Bridge publikovaný snapshot nemusí zodpovedať aktuálnym zdrojom

- **STATUS:** OPEN
- **Závažnosť:** MEDIUM, publikačné
- **Problém:** Audit našiel rozdiely medzi publisher/synchronizačným stavom a lokálnymi produkčnými zdrojmi.
- **Dopad:** Publikovaný prehľad alebo externý spotrebiteľ nemusí obsahovať aktuálne XKC a HOME SAFE opravy.
- **Istota:** Rozdiely snapshotu pri audite boli isté; aktuálny beh služby a publikovaný remote stav sú **NEOVERENÉ**.
- **Odporúčaný smer:** Read-only preveriť službu a synchronizačný stav, potom samostatne schváliť bezpečné obnovenie publikovania.

### 28. Nepoužitá Uno safety kostra a stale komentáre

- **STATUS:** DEFERRED
- **Závažnosť:** LOW
- **Problém:** V Uno zostáva nepoužitá kostra BuduceBezpecnostneStavy a komentáre, ktoré už nemusia presne opisovať aktuálny runtime.
- **Dopad:** Zvyšujú riziko nesprávnej interpretácie pri budúcej údržbe, bez priameho aktuálneho runtime dopadu.
- **Istota:** Isté pre nepoužitú kostru a identifikované komentáre.
- **Odporúčaný smer:** Odstrániť alebo premenovať iba po samostatnom schválení údržby; nevykonávať funkčný refaktor.
- **Výslovná výnimka:** Mega R5–R8 a R11–R16 sú PLANNED / RESERVED OUTPUT – OK. Tento nález sa na ne nevzťahuje.

## COMMISSIONING INCIDENTS – 2026-09-02

### UNO D9 AGREEMENT OUTPUT

- **STATUS INCIDENTU:** CLOSED / ROOT_CAUSE_IDENTIFIED / PHYSICAL_CONTACT_FAULT
- **Priebeh:** Po fyzickom teste výpadku Mega↔Uno UART a následnom obnovení komunikácie Uno softvérovo korektne dokončilo celý 180 s stabilizačný interval. Sériový log obsahoval RECOVERY: UNO_AGREEMENT=ON a pravidelná diagnostika zobrazovala AGR=ON.
- **Prvý incident:** Pri pripojenom agreement obvode bolo na fyzickej vetve D9 namerané približne 0 V a relé nezoplo. Po rozpojení vodiča sa priamo na Uno D9 objavilo približne +5 V; po opätovnom pripojení zostalo D9 HIGH a relé fungovalo.
- **Historické reproduction testy:** Po prvom incidente prešli tri samostatné cold-boot/power-cycle testy. Vždy prebehlo UNO_SMART_STABLE, po 180 s nasledovalo UNO_AGREEMENT=ON a AGR=ON, D9 malo približne +5 V a agreement relé fyzicky zoplo.
- **Druhá reprodukcia:** Kombinácia AGR=ON a fyzicky vypnuté agreement relé sa 2026-09-02 znovu objavila. Dotyk/meranie v oblasti D9 vyvolalo chatter/cvakanie relé a manipulácia s konkrétnym kontaktom poruchu reprodukovala alebo odstránila.
- **ROOT CAUSE IDENTIFIED:** Nespoľahlivý prerušovaný elektrický kontakt na konektore/header spoji Uno D9. Kontakt bol mechanicky zasunutý na doraz, ale elektricky nebol spoľahlivý.
- **Potvrdené:** 180 s agreement algoritmus je softvérovo PASS a Uno D9 dokáže vytvoriť HIGH. Incident nie je software bug ani chyba agreement časovania.
- **Readback obmedzenie:** Aktuálne nie je implementovaný fyzický readback napätia D9 ani kontaktu/polohy agreement relé. AGR=ON je softvérový command/state, nie meranie skutočného napätia na D9 alebo fyzického zopnutia relé.
- **Po oprave kontaktu:** Incident je uzavretý ako fyzická kontaktná porucha. Zostáva iba bežné prevádzkové pozorovanie možnej recidívy kontaktu; agreement algoritmus sa nemení.

### MEGA ↔ UNO HARDWARE CROSS-RESET – SOFTWARE PREP

- **STATUS:** PREPARED / DISABLED / PINS_TBD / NOT_COMMISSIONED
- **Rozsah:** Mega aj Uno obsahujú samostatnú compile-time odrezanú prípravu pulzného heartbeat a jedného vzájomného resetovacieho pokusu. Default je `CROSS_RESET_ENABLED=0`.
- **Defaultný build:** Neinicializuje žiadny nový GPIO, neposiela heartbeat a nevydáva RESET. Bez pridelených troch pinových makier zapnutý build zámerne zlyhá hláškou `Assign cross-reset heartbeat/reset pins before enabling.`
- **Piny:** Heartbeat OUT, heartbeat IN a peer RESET OUT zostávajú na oboch doskách `TBD`; žiadne číslo pinu nebolo pridelené.
- **Nezávislosť:** Heartbeat vzniká iba z hlavného loopu a nesmie sa odvodiť z UART linky, D31/D9 agreement, SystemMode, XKC ani TOTAL STOP.
- **Pripravené časy:** toggle 500 ms, timeout 5 000 ms, startup grace 15 000 ms, pre-reset servisné grace 15 000 ms, reset pulse 200 ms a post-reset grace 15 000 ms; všetky sú pomenované konštanty pre budúci commissioning.
- **Anti-storm:** Pred armovaním sa vyžadujú najmenej dve reálne hrany peer heartbeat. Pri jednej súvislej strate je povolený najviac jeden reset; ďalší pokus sa odomkne až po dvoch nových hranách potvrdeného recovery.
- **Hardvér TBD pre budúcu montáž:** BC547B, collector na RESET cieľovej dosky, emitter na spoločnú GND, base cez 4k7 z reset-output GPIO a 47k pull-down base→GND. HIGH znamená aktívny RESET pull-down, LOW neaktívny.
- **Výslovné obmedzenie:** Vrstva nie je funkčne zapnutá, fyzicky zapojená, commissioned ani safety-tested. D31/D9, XKC, D32/A0, BASIC/SMART, SystemMode, V5, R9/R10, SD a 180 s recovery sa nemenia.

### XKC SAFETY BEZ UART – PHYSICAL PASS

- **Výsledok:** PHYSICAL PASS / COMMISSIONED
- **Priebeh:** Mega↔Uno UART bol fyzicky odpojený. Obe lokálne XKC vetvy po viac než 5 s súvislého LOW_WATER fyzicky zopli svoje vlastné TOTAL STOP relé: Mega D30 aktivovalo Mega D32 a Uno A2 aktivovalo Uno A0.
- **Recovery:** Pri návrate WATER zostali obe vetvy tripnuté počas recovery. Po približne 10 s stabilného WATER obe TOTAL STOP relé fyzicky odpadli. Nový LOW_WATER počas recovery správne vynuloval 10 s recovery interval.
- **Záver:** Lokálna XKC safety na Mega aj Uno funguje nezávisle od UART komunikácie. Reset počas aktívneho LOW WATER bol následne samostatne fyzicky otestovaný; jeho výsledok a akceptované reziduálne riziko sú vedené v audite #11.

### RESET DURING ACTIVE LOW_WATER – AUDIT #11

- **Dátum:** 2026-09-02
- **STATUS:** PHYSICALLY_CONFIRMED / ACCEPTED_RESIDUAL_RISK
- **Východiskový stav:** LOW_WATER bol aktívny a potvrdený; obe TOTAL STOP vetvy boli tripnuté.
- **Reset iba Uno:** Mega TOTAL STOP zostal aktívny a spoločná ochranná cesta zostala blokovaná – **PHYSICAL PASS**.
- **Reset iba Mega:** Uno TOTAL STOP zostal aktívny a spoločná ochranná cesta zostala blokovaná – **PHYSICAL PASS**.
- **Súčasný reset Mega + Uno:** oba lokálne trip stavy sa resetovali a vzniklo približne 2–3 s permissive okno, počas ktorého by motorová cesta mohla byť povolená. Následne oba kontroléry znovu vyhodnotili LOW_WATER a TOTAL STOP opäť aktivovali.
- **Záver:** Redundancia chráni pri resete jednej dosky. Súčasný reset oboch dosiek má krátke fyzicky potvrdené permissive okno. Prevádzkovateľ toto známe reziduálne riziko pre aktuálnu aplikáciu akceptuje; nejde o opravený nález a kód sa nemení.

### AGREEMENT 180 s – PHYSICAL/SOFTWARE PASS

- **Výsledok:** PHYSICAL/SOFTWARE PASS
- **Mega:** Po resetoch a obnovení komunikácie SMART_STABLE pokračovalo cez 10 až 170/180 s, nasledovalo RECOVERY: MEGA_AGREEMENT=ON a po prijatí vzdialeného Uno agreement SYSTEM_MODE=SMART REASON=FULL_SMART.
- **Uno:** Po RECOVERY: UNO_SMART_STABLE=START nasledovalo po celom 180 s intervale RECOVERY: UNO_AGREEMENT=ON; následná diagnostika uvádzala UNO=OK LINK=OK AGR=ON.
- **Záver:** 180 s stabilizácia, zapnutie agreement a návrat do FULL_SMART pri platných podmienkach fungujú. D31/D9 zostávajú statické agreement/permission výstupy, nie pulzný hardvérový watchdog.
- **Výhrada:** D9 incident je vedený samostatne ako CLOSED / ROOT_CAUSE_IDENTIFIED / PHYSICAL_CONTACT_FAULT. Potvrdená bola prerušovaná fyzická kontaktná porucha konektora, nie chyba softvérového algoritmu.

### FYZICKÉ TESTY TEPLOTNÝCH FALLBACKOV – FULL_SMART

| Test | Pozorovaný výsledok | Stav |
|---|---|---|
| Mega T2/H2 odpojený | T2 = -127 °C/CHYBA; platný UNO_T2 približne 26,00 °C bol použitý ako REMOTE_FALLBACK; MODE_DEGRADED / T2_REMOTE_FALLBACK; Mega agreement zostal ON; po pripojení návrat FULL_SMART | **PHYSICAL PASS** |
| Mega T3/G2 odpojený | T3 = -127 °C/CHYBA; platný UNO_T3 približne 19,75 °C bol použitý ako UNO_T3_REMOTE_FALLBACK; MODE_DEGRADED / T3_REMOTE_FALLBACK; Mega agreement zostal ON; po pripojení návrat FULL_SMART | **PHYSICAL PASS** |
| Mega T1/H1 odpojený | T1 = -127 °C/CHYBA; platný Mega T4/H3 približne 26,2 °C bol použitý ako LOCAL_FALLBACK; POOL=OK; MODE_DEGRADED / POOL_LOCAL_FALLBACK; Mega agreement zostal ON; bez BASIC/STOP | **PHYSICAL PASS – T1 → T4** |
| Mega T1 + T4 odpojené | Pre fyzicky nepraktický prístup ku konektorom nebolo vykonané overenie vzdialeného UNO_T1 fallbacku | **NOT TESTED / PHYSICAL ACCESS NOT PRACTICAL** |

### DIAGNOSTICKÉ A TIMING POZOROVANIA

- **STATUS FOLLOW-UP:** DEFERRED / DIAGNOSTIC TERMINOLOGY
- Počas funkčne správneho T2 remote fallbacku sa súčasne zobrazilo SYSTEM: HAVARIA a autoritatívne SYSTEM_MODE=DEGRADED REASON=T2_REMOTE_FALLBACK. Starší text SYSTEM: HAVARIA je diagnosticky mätúci, ale fallback ani SystemMode rozhodnutie tým neboli chybné.
- Počas T1 local fallbacku bol fyzický T1 v stave CHYBA, efektívny pool bol platný cez T4, ale diagnostika súčasne uvádzala DIAG: T1=OK T2=OK SUSPECT=NONE. Ide o terminologický/diagnostický nesúlad, nie potvrdenú chybu fallbacku.
- Kód sa kvôli týmto pozorovaniam teraz nemení; textové vrstvy sa majú neskôr zosúladiť s rozdielom medzi fyzickým senzorom a efektívnou funkčnou hodnotou.
- Počas ustáleného behu rámce pribúdali, CRC_FAIL bolo prevažne 0, FRAME_INVALID=0 a SEQ_GAP=0. Niekoľko jednotlivých REPLY_TIMEOUT vzniklo počas fyzickej manipulácie/testovania.
- Mega loop bol typicky približne 0,39–0,41 s, pozorované maximum približne 409 752 µs, OVER_1000MS=0 a OVER_1500MS=0.
- Toto je iba commissioning pozorovanie normálneho behu. Neuzatvára auditný nález stuck I2C, blocking fault ani worst-loop pri poruche.

## CURRENT PHYSICAL TEST BACKLOG

- [x] **AUDIT #7 – PHYSICAL MODE ISOLATION CONFIRMED / ARCHITECTURALLY ACCEPTED:** pri strate UART prešiel systém do BASIC a fyzická povoľovacia autorita SMART odpadla aj pri naďalej ON SMART commande.
- [x] **PHYSICAL PASS 2026-09-02:** Mega XKC D30 aktivovalo D32 po viac než 5 s súvislého LOW WATER.
- [x] **PHYSICAL PASS 2026-09-02:** Uno XKC A2 aktivovalo A0 po viac než 5 s súvislého LOW WATER.
- [x] **PHYSICAL PASS 2026-09-02:** obe lokálne XKC vetvy aktivovali svoje TOTAL STOP relé pri fyzicky odpojenom Mega↔Uno UART.
- [x] **PHYSICAL PASS 2026-09-02:** po WATER a 10 s súvislého recovery obe TOTAL STOP relé odpadli.
- [x] **PHYSICALLY_CONFIRMED / ACCEPTED_RESIDUAL_RISK 2026-09-02:** reset jednej dosky zachová blokáciu cez druhú; súčasný reset Mega+Uno vytvorí približne 2–3 s permissive okno a potom sa TOTAL STOP znovu aktivuje.
- [ ] Fyzicky overiť absenciu R9/R10 boot LOW pulzu po softvérovej HIGH-latch oprave.
- [ ] Overiť I2C stuck-bus správanie, čas do safety inicializácie a loop latenciu.
- [x] **SD ABSENT AT BOOT / RUN – PHYSICAL PASS 2026-09-02:** bez SD Uno nabootovalo, supervisor/link/XKC/agreement fungovali a opakované recovery pokusy nespôsobili pozorovaný reset, freeze ani link loss.
- [ ] **AUDIT #19 zostáva OPEN:** zmerať SRAM/stack watermark pri približne 796 B voľnej SRAM a worst-case runtime/SD blocking pri chybovom alebo pomalom SD médiu.
- [x] **PHYSICALLY_REPRODUCED_SYMPTOM 2026-09-02:** pri niekoľkominútovej trvalej UNO_TBOX chybe narástli CRC/INV/GAP, ale LINK a AGR zostali ON; po pripojení nastal recovery bez resetu.
- [ ] **ROOT_CAUSE_NOT_PROVEN – AUDIT #20:** časovo izolovať OneWire/sensors.begin(), SoftwareSerial a jednotlivé V5 chyby; jednotlivé chyby existujú aj mimo DS fault.
- [ ] Fyzicky overiť fallback Mega T1 + T4 → UNO_T1; aktuálne NOT TESTED / PHYSICAL ACCESS NOT PRACTICAL.
- [x] **NOT REPRODUCED 2026-09-02:** vykonané tri samostatné cold-boot/power-cycle testy Uno s pripojenou agreement vetvou.
- [x] **PHYSICAL PASS 3/3:** po 180 s bolo UNO_AGREEMENT=ON, AGR=ON, D9 približne +5 V a agreement relé fyzicky zoplo.
- [x] **NOT REPRODUCED AFTER 3 COLD BOOTS:** kombinácia AGR=ON a fyzické D9 približne 0 V sa nezopakovala.
- [x] **SECOND REPRODUCTION / ROOT CAUSE IDENTIFIED:** neskoršia recidíva AGR=ON + fyzicky OFF relé umožnila manipuláciou reprodukovať prerušovaný kontakt na Uno D9 konektore/header spoji.
- [x] **CLOSED / ROOT_CAUSE_IDENTIFIED / PHYSICAL_CONTACT_FAULT:** nejde o software bug ani chybu 180 s agreement algoritmu.

Pri prípadnej recidíve skontrolovať a zmerať D9 kontakt ešte pred manipuláciou. AGR=ON samo nepotvrdzuje fyzické napätie ani polohu reléového kontaktu.

## POST-COMMISSIONING DIAGNOSTIC FOLLOW-UP

- [ ] **DEFERRED:** zosúladiť starší text SYSTEM: HAVARIA s autoritatívnym MODE_DEGRADED pri bezpečne fungujúcom fallbacku.
- [ ] **DEFERRED:** rozlíšiť v texte diagnostiky chybu fyzického T1 od platnej efektívnej pool funkcie cez T4/UNO_T1.

## CLOSED IN HOME SAFE FIX PACK – 2026-09-02

### Vykonané opravy

1. **Audit #2:** Uno T1/T2/T3 používa rolové rozsahy Mega a Mega vzdialené hodnoty pred fallbackom znovu rolovo validuje. Fallback poradie sa nezmenilo.
2. **Audit #8:** Active-LOW R9/D22 a R10/D23 dostanú HIGH latch pred pinMode(OUTPUT).
3. **Audit #15:** V5 RTC validity aj filtrácia používajú spoločný rtcCasJePlatny() vrátane čitateľnosti, OSF a rozsahov.
4. **Audit #17:** Mega aj Uno odmietajú nepovolené reserved validity bity V5 bez zmeny layoutu, veľkostí alebo bitovej mapy.
5. **Audit #22:** Wi-Fi BAZEN používa POOL_TEMP_VALID a efektívnu teplotaBazena, preto zobrazuje aj platný UNO_T1 fallback.
6. **Audit #5:** LCD nadradený stav používa SystemMode; XKC LOW WATER/TOTAL STOP už nemôže byť SYS:OK, SYSTEM:OK ani ZIADNA AKTIVNA.
7. **Súvisiaca diagnostika k auditu #5:** USB súhrn PROBLEM pozná XKC_LOW_WATER_TOTAL_STOP.

### Výsledky kompilácie

| Projekt | Flash/code | RAM | Voľná RAM / IRAM | Výsledok |
|---|---:|---:|---:|---|
| Mega | 43 802 B | 4 419 B | 3 773 B free RAM | PASS |
| Uno | 27 858 B | 1 252 B | 796 B free RAM | PASS |
| ESP8266 | 319 248 B flash code | 34 100 B global RAM | 59 963 B IRAM | PASS |

### Výsledky testov

- Všetky truth testy HOME SAFE FIX PACK prešli.
- PASS: všetko platné a platné lokálne/remote fallback poradie.
- PASS: T1/T2/T3 hodnoty platné ako všeobecná DS18B20 teplota, ale mimo rolového rozsahu, sú odmietnuté.
- PASS: normálny V5 rámec je prijatý; rámec s nepovoleným reserved validity bitom je odmietnutý na Mega aj Uno.
- PASS: platný UNO_T1 fallback sa prenesie do Wi-Fi poľa BAZEN.
- PASS: statické poradie R9/R10 je HIGH latch pred OUTPUT.
- PASS: MODE_STOP a lokálny XKC trip nemôžu na LCD ani v USB PROBLEM súhrne skončiť ako OK.

### Zámerne neuzavreté týmto balíkom

I2C timeout a skorá safety inicializácia, SystemMode gating R9/R10/manual/test, watchdog/freeze architektúra, Uno BLACK BOX XKC/A0/D9, ESP autentifikácia a secrets, Uno RAM/SD/timing merania a zostávajúce fyzické commissioning testy zostávajú podľa statusov a physical backlogu. Reset počas LOW WATER bol fyzicky charakterizovaný a je vedený ako ACCEPTED_RESIDUAL_RISK, nie FIXED.
