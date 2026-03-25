# Woche 2 — Buszyklen (Bus Cycles)

## Überblick

In dieser Woche wurde der externe synchrone Speicherbus des STM32F4xx analysiert. Mit einem Logic Analyzer wurden die Bussignale bei verschiedenen Zugriffsarten (Byte, Halfword, Word) gemessen und interpretiert. Der Fokus lag auf dem Verständnis, wie die CPU über den 16-Bit Datenbus mit externen Peripheriegeräten kommuniziert.

## Lernziele

- Zeitlichen Verlauf von Bussignalen mit einem Logic Analyzer messen und interpretieren
- Funktionsweise des synchronen Speicherbusses erklären können
- Einfluss der Software (Zugriffsbreite, Adressausrichtung) auf die Buszyklen erkennen
- Unterschiede zwischen Byte-, Halfword- und Word-Zugriffen auf dem Bus verstehen

## Kernkonzepte

### Synchroner externer Speicherbus

Der STM32F429ZI besitzt einen **16-Bit breiten externen Datenbus**, der über den **FMC (Flexible Memory Controller)** angesteuert wird. Der Bus ist **synchron** — alle Transfers sind an ein Taktsignal (CLK) gebunden.

**Bussignale:**

| Signal | Funktion | Aktiv |
|--------|----------|-------|
| `CLK` | Taktsignal | — |
| `NE` | Chip Enable | Low |
| `NOE` | Output Enable (Lesen) | Low |
| `NWE` | Write Enable (Schreiben) | Low |
| `NBL0` | Lower Byte Lane Enable | Low |
| `NBL1` | Upper Byte Lane Enable | Low |
| `A[10:8]` | Adressbits | — |
| `D[15:0]` | Datenbus (16-Bit) | — |

> **Konvention:** Das Präfix „N" steht für **active-low** — das Signal ist aktiv wenn es auf 0 (Low) liegt.

### CT-Board Architektur

```
CPU (Cortex-M4)  ←→  System Bus  ←→  FMC  ←→  Externer Bus  ←→  CPLD  ←→  Peripherie
                      (intern)                  (16-Bit)           │
                                                                   ├── LEDs
                                                                   ├── DIP Switches
                                                                   ├── Buttons
                                                                   └── 7-Segment
```

- Die **CPU** greift über den internen System Bus auf den **FMC** zu
- Der **FMC** übersetzt interne Zugriffe in externe Buszyklen
- Das **CPLD** dekodiert die Adresse und steuert die richtige Peripherie an
- Das CT-Board muss in **Modus 3** betrieben werden, damit der externe Bus auf den Steckern P1–P4 verfügbar ist

### Memory-Mapped I/O und Volatile Pointer

Peripherie wird über feste Speicheradressen angesprochen. Der Zugriff erfolgt über **volatile Pointer Casts**:

```c
#define ADDR_LED   ((uint32_t) 0x60000100)
#define ADDR_DIPSW ((uint32_t) 0x60000200)

// 16-Bit Zugriff (Halfword)
#define CT_LED   (*((volatile uint16_t *) ADDR_LED))
#define CT_DIPSW (*((volatile uint16_t *) ADDR_DIPSW))
```

**Warum `volatile`?** Der Compiler darf Zugriffe auf Hardware-Register nicht wegoptimieren oder cachen. `volatile` erzwingt, dass jeder Lese-/Schreibzugriff im Code auch tatsächlich auf dem Bus ausgeführt wird.

**Zugriffsbreite durch Datentyp:** Der Cast-Typ bestimmt, wie breit der Buszugriff ist:

| Datentyp | Zugriffsbreite | Busverhalten |
|----------|---------------|-------------|
| `uint8_t *` | 8 Bit (Byte) | 1 Buszyklus, eine Byte Lane aktiv |
| `uint16_t *` | 16 Bit (Halfword) | 1 Buszyklus, beide Byte Lanes aktiv |
| `uint32_t *` | 32 Bit (Word) | 2 Buszyklen (Bus ist nur 16-Bit breit) |

### Byte Lane Signale (NBL0 / NBL1)

Die Byte Lane Signale zeigen an, welche Hälfte des 16-Bit Datenbusses gültige Daten enthält:

| Zugriff | NBL0 (Lower) | NBL1 (Upper) | Gültige Daten |
|---------|:---:|:---:|---|
| Halfword (16-Bit) | 0 (aktiv) | 0 (aktiv) | D[15:0] — alle |
| Byte Even (gerade Adresse) | 0 (aktiv) | 1 (inaktiv) | D[7:0] — unteres Byte |
| Byte Odd (ungerade Adresse) | 1 (inaktiv) | 0 (aktiv) | D[15:8] — oberes Byte |

> **Wichtig:** Beim **Read Cycle** erfolgt keine Byte-Signalisierung über NBL — das Peripheriemodul (CPLD) liefert immer 16 Bit, und die CPU ignoriert intern die nicht benötigten Bytes. Die Byte Lane Signale sind nur beim **Write Cycle** relevant.

### Read Cycle vs. Write Cycle

**Read Cycle (Lesen von Peripherie):**
- NE geht low → NOE geht low → Peripherie legt Daten auf den Bus → CPU liest bei steigender Flanke
- Dauert länger, weil die CPU im Timeslot T5 zusätzlich Zeit braucht, um die Daten einzulesen
- CPLD liefert immer 32-Bit Daten (unabhängig ob Halfword oder Word angefragt)

**Write Cycle (Schreiben an Peripherie):**
- NE geht low → NWE geht low → CPU legt Daten auf den Bus → Peripherie übernimmt bei steigender Flanke
- Kürzer als Read, da die CPU keine Daten einlesen muss

### Aligned vs. Unaligned Zugriffe

Ein **aligned** Zugriff bedeutet, dass die Adresse ein Vielfaches der Zugriffsbreite ist:
- Byte: jede Adresse ist aligned
- Halfword: Adresse muss gerade sein ($\text{Adresse} \mod 2 = 0$)
- Word: Adresse muss durch 4 teilbar sein ($\text{Adresse} \mod 4 = 0$)

Ein **Halfword-Zugriff auf eine ungerade Adresse** (unaligned) erfordert **zwei separate Buszyklen**, weil die gewünschten 16 Bit auf zwei verschiedenen Byte-Positionen liegen, die nicht in einem einzigen 16-Bit Buszyklus übertragen werden können.

## Aufgaben und Umsetzung

### Aufgabe 1: Halfword Zugriffe (`MODE_16BIT`)

- **Ziel:** 16-Bit Zugriffe auf DIP Switches und LEDs analysieren
- **Umsetzung:** `#define MODE_16BIT` aktivieren → `uint16_t` Pointer auf `0x60000200` (Read) und `0x60000100` (Write)
- **Beobachtung:** Read Cycle und Write Cycle im Logic Analyzer identifiziert; CPLD liefert immer 32-Bit, CPU ignoriert überschüssige Daten

### Aufgabe 2: Byte Zugriffe (`MODE_8BIT_EVEN` / `MODE_8BIT_ODD`)

- **Ziel:** Unterschied zwischen Even- und Odd-Byte-Zugriffen verstehen
- **Beobachtung:** Bei Even (Adresse `0x...00`) ist NBL1 inaktiv (nur unteres Byte); bei Odd (Adresse `0x...01`) ist NBL0 inaktiv (nur oberes Byte)
- **Schlüssel-Erkenntnis:** Byte Lane Signale nur beim Write relevant; beim Read ignoriert die CPU intern

### Aufgabe 3: Word Zugriffe (`MODE_32BIT`)

- **Ziel:** 32-Bit Zugriff über 16-Bit Bus beobachten
- **Beobachtung:** Zwei aufeinanderfolgende 16-Bit Buszyklen für einen 32-Bit Zugriff

### Aufgabe 4: Zugriffszeiten messen

- **Ziel:** Timing des Busses quantifizieren
- **Messergebnisse:**

| Messung | Zeit |
|---------|------|
| $t_{\text{read}}$ | 644 ns |
| $t_{\text{write}}$ | 644 ns |
| $t_{\text{read→write}}$ | 440 ns |
| $t_{\text{write→read}}$ | 784 ns |

### Aufgabe 5: Halfword auf ungerade Adresse

- **Ziel:** 16-Bit Zugriff auf `0x60000101` (DIP Switches S23…8 → LEDs 23…8)
- **Lösung:**
  ```c
  #define CT_LED   (*((volatile uint16_t *) ADDR_LED_15_8))    // 0x60000101
  #define CT_DIPSW (*((volatile uint16_t *) ADDR_DIPSW_15_8))  // 0x60000201
  ```
- **Beobachtung:** CPU erzeugt **zwei Buszyklen** statt einem, da der Zugriff unaligned ist

## Wichtige Register / Hardware

| Register | Adresse/Pfad | Relevante Bits | Funktion |
|----------|-------------|----------------|----------|
| `CT_LED` (LEDs 7…0) | `0x60000100` | D[7:0] oder D[15:0] | LED-Ausgabe (8/16/32-Bit Zugriff) |
| `CT_LED` (LEDs 15…8) | `0x60000101` | D[15:8] | LED-Ausgabe oberes Byte (8-Bit odd) |
| `CT_DIPSW` (S7…0) | `0x60000200` | D[7:0] oder D[15:0] | DIP-Switch-Eingabe (8/16/32-Bit Zugriff) |
| `CT_DIPSW` (S15…8) | `0x60000201` | D[15:8] | DIP-Switch-Eingabe oberes Byte (8-Bit odd) |
| `NBL0` | Control Bus (P4) | 1 Bit (active-low) | Lower Byte Lane Enable |
| `NBL1` | Control Bus (P4) | 1 Bit (active-low) | Upper Byte Lane Enable |
| `NE` | Pin PD7 (analog) | 1 Bit (active-low) | Chip Enable — definiert Zugriffsdauer |
| `NOE` | Control Bus (P4) | 1 Bit (active-low) | Output Enable (Read Cycle) |
| `NWE` | Control Bus (P4) | 1 Bit (active-low) | Write Enable (Write Cycle) |

## Schlussfolgerungen

- **Der Datentyp des Pointer-Casts bestimmt die Zugriffsbreite** auf dem Bus — `uint8_t *` erzeugt einen Byte-Zugriff, `uint16_t *` einen Halfword-Zugriff, `uint32_t *` einen Word-Zugriff (zwei Buszyklen)
- **Byte Lane Signale** (NBL0/NBL1) sind nur beim Write Cycle relevant — beim Read liefert das CPLD immer alle 16 Bit, die CPU selektiert intern
- **Unaligned Zugriffe** sind teuer: Ein Halfword-Zugriff auf eine ungerade Adresse verdoppelt die Buszyklen. Daher: Daten immer aligned im Speicher ablegen
- **Volatile ist zwingend** bei Hardware-Zugriffen — ohne `volatile` darf der Compiler Zugriffe wegoptimieren oder zusammenfassen, was bei Memory-Mapped I/O zu Fehlverhalten führt
- **Read ist nicht gleich Write**: Der Read Cycle benötigt zusätzliche Zeit (Timeslot T5) für die Datenübernahme; der Write Cycle ist kürzer, da die CPU die Daten sofort auf den Bus legt

## Verwandte Themen

- **Woche 1 (Coding Style):** Grundlagen des modularen C-Codes und Memory-Mapped I/O (`CT_LCD->ASCII[]`)
- **Woche 3 (GPIO):** Direkter Registerzugriff auf GPIO-Pins — verwendet ebenfalls volatile Pointer und Bit-Manipulation, aber über den internen Bus (APB) statt den externen Speicherbus
