# CT Praktikum: Buszyklen

## 1. Einleitung

Ein Logic Analyzer dient dazu, den Zeitverlauf von digitalen Signalen zu erfassen und darzustellen. Gegenüber einem reinen Oszilloskop können wesentlich mehr Eingänge erfasst werden. Es werden allerdings keine analogen Signalpegel, sondern nur die digitalen Logikpegel 0/1/undefiniert erfasst und angezeigt.

In diesem Praktikum analysieren wir den zeitlichen Verlauf von Zugriffen auf dem Speicherbus mit Hilfe der Logic Analyzer Funktionalität des Tektronix Oszilloskops.

Die Messungen erfolgen am externen, synchronen Bus des STM32F4xx. Es wird ein 16-Bit breiter Datenbus verwendet. Die Signale und Abläufe entsprechen dem in der Vorlesung präsentierten Systembus.

## 2. Lernziele

- Zeitlichen Verlauf von Bussignalen mit einem einfachen Logic Analyzer messen und interpretieren
- Erklären können, wie der synchrone Speicherbus funktioniert
- Einfluss der Software auf die Buszyklen erkennen

## 3. Vorbereitung

### 3.1. Aufbau CT Board

*Abbildung: Blockdiagramm des CT-Boards*

Das CT-Board besteht aus:
- **CPU**: ARM Cortex-M4 (STM32F429ZI) mit NVIC
- **On-Chip Memory**: Flash und SRAM
- **FMC** (Flexible Memory Controller): Verbindet den internen System Bus mit dem externen Speicherbus
- **CPLD**: Steuert die externen Peripherie-Module (GPIO, Switches, Buttons, LEDs, 7-Segment)
- **Externer Bus**: 16-Bit synchroner Datenbus, verbunden über Stecker P1–P4

> **Wichtig:** Für dieses Praktikum muss das CT-Board in **Modus 3** betrieben werden!

### 3.2. Anschluss Logic Analyzer an CT Board

Das Oszilloskop verfügt neben 4 analogen Kanälen über 16 digitale Logic Analyzer Kanäle. Beide können gleichzeitig genutzt werden.

**Digitale Kanäle (16 Stück):**

| Kanal | Signal | Funktion |
|-------|--------|----------|
| D0 | `CLK` | Clock |
| D1 | `NBL1` | Byte Lane 1 (active-low) — Upper byte enable |
| D2 | `NBL0` | Byte Lane 0 (active-low) — Lower byte enable |
| D3 | `NWE` | Write Enable (active-low) |
| D4 | `NOE` | Output Enable (active-low) |
| D7…5 | `A[10:8]` | Address Bits 10…8 |
| D15…8 | `D[7:0]` oder `D[15:8]` | Data Bits 7…0 oder 15…8 |

**Anschluss an Stecker:**
- **P1**: D[15..8] — oberes Datenbyte
- **P2**: D[7..0] — unteres Datenbyte
- **P3**: A[11..4] — Adressbits
- **P4**: Control Signals + A[10..8]

**Analoger Kanal:**

| Kanal | Signal | Funktion | Abgriff |
|-------|--------|----------|---------|
| Kanal 1 | `NE` | Chip Enable (active-low) | Pin PD7 auf Discovery Board |

> **Hinweis:** Aufgrund der beschränkten Anzahl Kanäle des Logic Analyzers können nicht alle Daten- und Adressleitungen gleichzeitig angeschaut werden. Bei Bedarf werden die Logic Probes umgesteckt.

### 3.3. Einstellungen am Oszilloskop / Logic Analyzer

**Grundeinstellungen:**
1. „Default Setup" drücken → nur Kanal 1 (gelbe Linie) wird angezeigt
2. Zeitauflösung auf 400 ns/div einstellen

**Anzeige Control Signal NE:**
- Auflösung Kanal 1 auf 5 V/div

**Anzeige Control Bus (D15-D0):**
- D15-D0 On/Off: Kanäle D0 bis D4 auf On
- Thresholds: 1.4 Volt
- Edit Labels: Kanäle D0 bis D4 gemäss Abschnitt 3.2 beschriften

**Anzeige Address Bus (B1):**
- Bus B1: Parallel
- Define Inputs: Clocked Data: No, Number of Data Bits: 3, Define Bits: D7 bis D5 (LSB)
- Thresholds: 1.4 Volt
- B1 Label: `ADDR[10:8]`
- Bus Display: Hex

**Anzeige Data Bus (B2):**
- Bus B2: Parallel
- Define Inputs: Clocked Data: No, Number of Data Bits: 8, Define Bits: D15 bis D8 (LSB)
- Thresholds: 1.4 Volt
- B2 Label: `DATA`
- Bus Display: Hex

**Trigger:**
- Type: Edge
- Source: D4
- Slope: Falling Edge
- Level: 1.4 Volt

> Wir triggern auf das **NOE**-Signal (Falling Edge = high→low), da die Flanke immer am gleichen Ort ist.

## 4. Aufgabenstellung

Das C-Programm lässt sich mittels `#defines` unterschiedlich konfigurieren. In der Endlosschleife wird jeweils ein DIP-Switch-Wert gelesen und an die LEDs geschrieben: `CT_LED = CT_DIPSW;`

### 4.1. Halfword Zugriffe

**Aufgabe:** `MODE_16BIT` aktivieren. Es wird jeweils ein Halfword (16 Bit) von den DIP Switches S15..0 gelesen und an die LEDs 15..0 geschrieben.

```c
#define MODE_16BIT

#define CT_LED    (*((volatile uint16_t *) ADDR_LED))     // 0x60000100
#define CT_DIPSW  (*((volatile uint16_t *) ADDR_DIPSW))   // 0x60000200
```

**Analyse mit Logic Analyzer — Read und Write Cycle identifizieren:**

**Werte auf dem Bus (DIP Switch S0 = ON, Rest OFF):**

| Zyklus | Adresse (32-bit) | Daten (16-bit) |
|--------|-----------------|----------------|
| **Read Cycle** | `0x60000200` | `0x01` |
| **Write Cycle** | `0x60000100` | `0x01` |

> **Erklärung:** Beim Read Cycle erscheinen im Timeslot T5 zusätzlich die Daten der nachfolgenden Speicheradresse. Diese werden bei einem Halfword-Zugriff durch die CPU ignoriert. Da auf dem Bus kein gesondertes Kontrollsignal für die Unterscheidung von Halfword und Word Zugriffen existiert, führt die CPU den gleichen Buszyklus durch und ignoriert im ersten Fall die hinteren Daten. Das CPLD weiss nicht, ob es 16-bit oder 32-bit Daten liefern muss — es liefert daher immer 32-bit.
>
> Das Verhalten im Write Cycle ist anders, da dort der Zyklus kürzer ist. Das Signal NE wechselt mit der steigenden Clockflanke im Zyklus T4 auf high. Die CPU benötigt beim Write im Gegensatz zum Read keine zusätzliche Zeit, um die Daten einzulesen.

*Screenshots: siehe `Screenshot Log Analyzer 1.jpeg` bis `Screenshot Log Analyzer 6.jpeg`*

---

### 4.2. Byte Zugriffe

**Aufgabe:** `MODE_8BIT_EVEN` aktivieren. Es wird jeweils ein Byte gelesen und geschrieben.

```c
#define MODE_8BIT_EVEN

#define CT_LED    (*((volatile uint8_t *) ADDR_LED))      // 0x60000100 (gerade)
#define CT_DIPSW  (*((volatile uint8_t *) ADDR_DIPSW))    // 0x60000200 (gerade)
```

**Frage: Was hat sich gegenüber dem Halfword Zugriff verändert?**
> NBL1 ist länger 1 (high), also ist die Upper Byte Lane nicht mehr enabled, weil sie nicht gebraucht wird.

**Programm: Das Programm liest die Werte der DIP Switches S7..0 ein und gibt diese auf LEDs 7..0 aus.**

---

**Aufgabe:** Auf `MODE_8BIT_ODD` umstellen.

```c
#define MODE_8BIT_ODD

#define CT_LED    (*((volatile uint8_t *) ADDR_LED_15_8))   // 0x60000101 (ungerade)
#define CT_DIPSW  (*((volatile uint8_t *) ADDR_DIPSW_15_8)) // 0x60000201 (ungerade)
```

**Frage: Was hat sich gegenüber `MODE_8BIT_EVEN` geändert?**
> Anstelle von NBL1 (high) ist dies jetzt NBL0 (high). Also ist die Lower Byte Lane jetzt nicht mehr enabled.

**Frage: Auf welchen Busleitungen werden die Daten übertragen?**
> DATA 15 bis 8 (oberes Byte)

**Frage: Erklären Sie, wie der Microcontroller signalisiert, welche Datenleitungen gültig sind?**
> Der Mikrocontroller signalisiert über die aktiven Low-Byte-Enable-Signale NBL0 und NBL1, welches Datenbyte (unteres oder oberes) gültig ist.

**Frage: Was könnte der Grund sein, dass beim Read Zyklus keine Byte Signalisierung erfolgt?**
> Beim Read-Zyklus erfolgt keine Byte-Signalisierung, weil das Peripheriemodul immer 16 Bit bereitstellt und die CPU nicht benötigte Bytes intern ignoriert.

**Programm: Das Programm liest die Werte der DIP Switches S15..8 ein und gibt diese auf LEDs 15..8 aus.**

> **Erklärung:** Beim Bytezugriff werden die Daten jeweils auf einer Hälfte des Datenbusses durch den Empfänger ignoriert. D.h. beim Odd Zugriff sind die Werte auf den Leitungen DATA[7:0] bedeutungslos. Da die effektiven Werte bedeutungslos sind, wird oft der Einfachheit halber auf beiden Bushälften der gleiche Wert aufgeschaltet.

---

### 4.3. Word Zugriffe

**Aufgabe:** `MODE_32BIT` aktivieren. Der herausgeführte Datenbus auf dem CT Board ist nur 16-Bit breit. Wie kann ein ganzes Word (32-Bit) übertragen werden?

```c
#define MODE_32BIT

#define CT_LED    (*((volatile uint32_t *) ADDR_LED))     // 0x60000100
#define CT_DIPSW  (*((volatile uint32_t *) ADDR_DIPSW))   // 0x60000200
```

Logic Probe an `D[7:0]` anschliessen.

**Frage: Was hat sich gegenüber den vorherigen Zugriffen verändert?**
> Er schreibt auch die oberen (Bytes) — der Bus wird zweimal hintereinander verwendet (zwei 16-Bit Transfers für einen 32-Bit Zugriff).

---

### 4.4. Zugriffszeiten

**Aufgabe:** Programm auf `MODE_16BIT` umstellen. Mit der „Cursors"-Taste Zeiten messen. Die Zugriffsdauer entspricht der Zeit, in der das Signal **NE** aktiv (low) ist.

| Messung | Zeit |
|---------|------|
| Zugriffszeit Lesezyklus $t_{\text{read}}$ | 644.0 ns |
| Zugriffszeit Schreibzyklus $t_{\text{write}}$ | 644.0 ns |
| Zeit zwischen Lese- und Schreibzyklus $t_{\text{read\_to\_write}}$ | 440 ns |
| Zeit zwischen Schreib- und Lesezyklus $t_{\text{write\_to\_read}}$ | 784.0 ns |

---

### 4.5. Halfword Zugriffe auf ungerade Adressen

**Aufgabe:** Mit dem abgegebenen Programm können Byte Zugriffe auf gerade wie ungerade Adressen erfolgen. Ein Halfword (16-Bit) Zugriff auf ungerade Adressen ist im Programm nicht implementiert.

Programm erweitern, um die DIP Switches S23…8 mit einer Anweisung an die LEDs 23…8 zu schreiben.

**Lösung** (im `#else`-Block von `main.c`):

```c
#define CT_LED      (*((volatile uint16_t *) ADDR_LED_15_8))    // 0x60000101
#define CT_DIPSW    (*((volatile uint16_t *) ADDR_DIPSW_15_8))  // 0x60000201
```

**Frage: Was fällt Ihnen auf?**
> Er schreibt jetzt doppelt — die CPU muss den Halfword-Zugriff auf eine ungerade Adresse in zwei separate Buszyklen aufteilen, da der 16-Bit Bus nur aligned Zugriffe in einem Zyklus unterstützt.

---

### Bewertung

| Bewertungskriterien | Gewichtung |
|---------------------|-----------|
| Halfword Zugriffe gelöst (Screenshots, Fragen) | 1/4 |
| Byte Zugriffe gelöst (Screenshots, Fragen) | 1/4 |
| Word Zugriffe und Zugriffszeiten gelöst (Screenshots, Fragen) | 1/4 |
| Halfword Zugriffe auf ungerade Adressen gelöst (Screenshots, Fragen) | 1/4 |

## 5. Abschluss

Nach Abschluss des Praktikums die Einstellungen am Oszilloskop mit der Taste „Default Setup" löschen.
