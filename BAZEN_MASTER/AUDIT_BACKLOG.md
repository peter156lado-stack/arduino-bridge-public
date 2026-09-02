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
| DEFERRED | 6 |
| WAITING_PHYSICAL_TEST | 2 |
| PLANNED | 2 |
| ACCEPTED_RESIDUAL_RISK | 0 |
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

- **STATUS:** DEFERRED
- **Závažnosť:** HIGH, fyzicky podmienené
- **Problém:** Regulácia, manual/test override a fyzické výstupy nepoužívajú SystemMode ako všeobecný výstupný gate.
- **Dopad:** Príkaz môže zostať aktívny počas STOP alebo sa po recovery obnoviť; požadované správanie v BASIC závisí od fyzickej architektúry.
- **Istota:** Cesty v kóde sú isté; správna fyzická matica pre STOP/BASIC je **NEOVERENÁ**.
- **Odporúčaný smer:** Najprv schváliť presnú mode/output maticu a až potom meniť gating. HOME SAFE FIX PACK túto časť zámerne nemenil.

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

- **STATUS:** OPEN
- **Závažnosť:** HIGH
- **Problém:** Po resete začína lokálne XKC potvrdenie od nuly a nový trip vznikne až po 5 s súvislého LOW WATER.
- **Dopad:** Pri súčasnom alebo nevhodne načasovanom resete oboch dosiek môže vzniknúť dočasné okno bez aktívneho softvérového TOTAL STOP.
- **Istota:** Reset správanie kódu je isté; fyzický dopad celej zapojenej vetvy je **NEOVERENÝ**.
- **Odporúčaný smer:** Fyzicky charakterizovať reset počas LOW WATER a až následne rozhodnúť o reziduálnom riziku alebo nezávislom latchi/ochrane.

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
- **Istota:** Statická RAM rezerva je istá; stack watermark, worst-loop a chybové SD časy sú **NEOVERENÉ**.
- **Odporúčaný smer:** Zmerať stack watermark, SD fault timing a realistický worst-loop bez zmeny BLACK BOX schémy.

### 20. SoftwareSerial a OneWire majú potvrdené timing riziko

- **STATUS:** WAITING_PHYSICAL_TEST
- **Závažnosť:** MEDIUM
- **Problém:** Interval 5300 ms znižuje fázové kolízie, ale pri dlhých OneWire operáciách a opakovanom sensors.begin() ich úplne nevylučuje.
- **Dopad:** Pri trvalej DS18B20 chybe môže Uno strácať alebo poškodzovať V5 komunikáciu.
- **Istota:** Mechanizmus kolízie je známy; správanie V5/38400 pri trvalej chybe je **NEOVERENÉ**.
- **Odporúčaný smer:** Fyzicky testovať V5 pri trvalej DS chybe a až podľa merania meniť timing.

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

- **STATUS INCIDENTU:** OBSERVED_ONCE / WAITING_REPRODUCTION
- **Priebeh:** Po fyzickom teste výpadku Mega↔Uno UART a následnom obnovení komunikácie Uno softvérovo korektne dokončilo celý 180 s stabilizačný interval. Sériový log obsahoval RECOVERY: UNO_AGREEMENT=ON a pravidelná diagnostika zobrazovala AGR=ON.
- **Fyzické pozorovanie:** Pri pripojenom agreement obvode bolo na fyzickej vetve D9 namerané približne 0 V a relé nezoplo. Po rozpojení príslušného vodiča/vetvy sa priamo na Uno D9 okamžite objavilo približne +5 V. Po opätovnom pripojení vodiča zostalo D9 HIGH a agreement relé začalo fungovať správne. Následný reset a opakovaný test sa už správali normálne.
- **Potvrdené:** 180 s agreement algoritmus softvérovo PASS. Uno D9 dokáže vytvoriť HIGH.
- **Nepotvrdená príčina:** Incident zatiaľ poukazuje na možný externý hardvérový jav v H/L module, jeho napájaní, backfeed/power-up stave, vodiči alebo spoji. Presná príčina je **NEPOTVRDENÁ**.
- **Rozhodnutie:** Produkčný kód sa kvôli jednorazovému incidentu nemení. Najprv sa musí incident reprodukovať a fyzicky izolovať jeho zdroj.

### XKC SAFETY BEZ UART – PHYSICAL PASS

- **Výsledok:** PHYSICAL PASS
- **Priebeh:** Mega↔Uno UART bol fyzicky odpojený. Obe lokálne XKC vetvy po viac než 5 s súvislého LOW_WATER fyzicky zopli svoje vlastné TOTAL STOP relé: Mega D30 aktivovalo Mega D32 a Uno A2 aktivovalo Uno A0.
- **Recovery:** Po obnovení WATER a 10 s súvislého potvrdenia obe TOTAL STOP relé odpadli.
- **Záver:** Lokálna XKC safety na Mega aj Uno funguje nezávisle od UART komunikácie. Test nemení otvorené riziko resetu počas trvalého LOW WATER ani ostatné nevykonané fyzické testy.

## CURRENT PHYSICAL TEST BACKLOG

- [x] **PHYSICAL PASS 2026-09-02:** Mega XKC D30 aktivovalo D32 po viac než 5 s súvislého LOW WATER.
- [x] **PHYSICAL PASS 2026-09-02:** Uno XKC A2 aktivovalo A0 po viac než 5 s súvislého LOW WATER.
- [x] **PHYSICAL PASS 2026-09-02:** obe lokálne XKC vetvy aktivovali svoje TOTAL STOP relé pri fyzicky odpojenom Mega↔Uno UART.
- [x] **PHYSICAL PASS 2026-09-02:** po WATER a 10 s súvislého recovery obe TOTAL STOP relé odpadli.
- [ ] Overiť reset jednej aj oboch dosiek počas trvalého LOW WATER a zmerať okno bez tripu.
- [ ] Fyzicky overiť absenciu R9/R10 boot LOW pulzu po softvérovej HIGH-latch oprave.
- [ ] Overiť I2C stuck-bus správanie, čas do safety inicializácie a loop latenciu.
- [ ] Zmerať Uno SD/stack watermark, SD fault čas a realistický worst-loop.
- [ ] Overiť UART V5 pri trvalej DS18B20 chybe a opakovanom OneWire recovery.
- [ ] Zopakovať cold boot/power-cycle Uno spolu s pripojenou H/L agreement vetvou.
- [ ] Po 180 s zmerať D9 priamo na Uno aj na vstupe H/L modulu.
- [ ] Overiť, či sa kombinácia AGR=ON a fyzické D9 približne 0 V zopakuje.
- [ ] Ak sa incident zopakuje, postupne izolovať napájanie, H/L modul, vodič a spoj a zistiť, čo sťahuje D9 do LOW.

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

I2C timeout a skorá safety inicializácia, SystemMode gating R9/R10/manual/test, watchdog/freeze architektúra, reset počas LOW WATER, Uno BLACK BOX XKC/A0/D9, ESP autentifikácia a secrets, Uno RAM/SD/timing merania a všetky fyzické commissioning testy zostávajú podľa statusov a physical backlogu.
