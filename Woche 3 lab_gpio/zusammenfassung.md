# Woche 3 — GPIO (General Purpose Input/Output)

## Überblick

In dieser Woche wurde die Konfiguration und Verwendung von GPIO-Pins am STM32F429ZI erarbeitet. Über direkte Registerzugriffe wurden GPIOA-Pins als Eingänge und GPIOB-Pins als Ausgänge konfiguriert — mit verschiedenen Output-Typen (Push-Pull, Open Drain), Pull-Widerständen und Geschwindigkeiten. Der Signalfluss wurde über Verbindungsdrähte zwischen den Ports getestet und alle Kombinationen systematisch untersucht.

## Lernziele

- GPIO mit Hilfe von technischen Dokumentationen (Reference Manual, `reg_stm32f4xx.h`) initialisieren
- GPIO zur Ein- und Ausgabe verwenden
- Verschiedene Konfigurationsoptionen (Mode, Type, Speed, Pull) verstehen und anwenden
- Strukturen des HAL (Basisadressen, Macros, Structs) zur Registerkonfiguration einsetzen
- Unterschied zwischen Push-Pull und Open Drain in der Praxis erfahren

## Kernkonzepte

### Registerzugriff über Structs und Macros

Der STM32F4xx bildet alle Peripherie-Register auf Speicheradressen ab. Der Zugriff erfolgt über Macros und Structs aus `reg_stm32f4xx.h`:

```c
#define GPIOB ((reg_gpio_t *) 0x40020400)

// Zugriff auf MODER-Register von Port B:
GPIOB->MODER &= ~(0x03 << 0);  // Clear: Bits 1..0 löschen
GPIOB->MODER |=  (0x01 << 0);  // Set:   Bits 1..0 auf 01 (Output)
```

Die Struktur `reg_gpio_t` bildet alle GPIO-Register eines Ports in der richtigen Reihenfolge und mit dem richtigen Offset ab:

| Offset | Feld | Bits/Pin | Funktion |
|--------|------|----------|----------|
| `0x00` | `MODER` | 2 | Mode: Input/Output/AF/Analog |
| `0x04` | `OTYPER` | **1** | Output Type: Push-Pull/Open Drain |
| `0x08` | `OSPEEDR` | 2 | Output Speed: Low/Medium/High/Very High |
| `0x0C` | `PUPDR` | 2 | Pull-up/Pull-down/None |
| `0x10` | `IDR` | 1 (read-only) | Input Data Register |
| `0x14` | `ODR` | 1 | Output Data Register |
| `0x18` | `BSRR` | — | Bit Set/Reset Register |

> **Achtung:** `OTYPER` hat nur **1 Bit pro Pin** — alle anderen Konfigurationsregister haben 2 Bits pro Pin.

### Clear-then-Set Pattern

Beim Konfigurieren von Registern müssen die betroffenen Bits zuerst gelöscht und dann gesetzt werden, um vorherige Konfigurationen sauber zu überschreiben:

```c
// Pattern: Clear mit invertierter Maske, dann Set mit Wert
register &= ~(bit_maske << bit_position);  // Clear
register |=  (neuer_wert << bit_position); // Set
```

Beispiel für PA0 als Input mit Pull-down:

```c
GPIOA->MODER &= ~(0x03 << 0);  // Bits 1..0 auf 00 (Input)
GPIOA->PUPDR &= ~(0x03 << 0);  // Clear
GPIOA->PUPDR |=  (0x02 << 0);  // Set Pull-down (10)
```

### Push-Pull vs. Open Drain

Zwei grundlegend verschiedene Output-Typen:

**Push-Pull** (`OTYPER` = `0`):
- Kann **aktiv High und Low** treiben
- Zwei Transistoren: einer zieht auf $V_{DD}$, einer auf $GND$
- Funktioniert immer zuverlässig, unabhängig von der Eingangs-Konfiguration

**Open Drain** (`OTYPER` = `1`):
- Kann nur **aktiv Low** treiben
- Bei High wird der Ausgang **hochohmig** (floating)
- Benötigt einen **externen Pull-up-Widerstand** für einen definierten High-Pegel
- Vorteil: Ermöglicht Wired-OR-Verbindungen und unterschiedliche Spannungspegel

### Pull-up / Pull-down Widerstände

Interne Pull-Widerstände stabilisieren den Zustand eines Pins:

| PUPDR | Konfiguration | Wirkung |
|-------|--------------|---------|
| `00` | No Pull | Pin ist hochohmig (floating) — empfindlich gegenüber Störungen |
| `01` | Pull-up | Zieht den Pin auf High wenn nicht extern getrieben |
| `10` | Pull-down | Zieht den Pin auf Low wenn nicht extern getrieben |
| `11` | Reserved | Nicht verwenden |

> **Wichtig:** Unbeschaltete Eingänge ohne Pull-Widerstand (`00`) sind **floating** und liefern undefinierte Werte. Die Fingerprobe zeigt: Der menschliche Körper wirkt als Antenne und koppelt 50 Hz Störungen ein, die den Zustand des Pins verändern.

### GPIO Clock Enable

Bevor auf GPIO-Register zugegriffen werden kann, muss der Peripheral Clock für den entsprechenden Port aktiviert werden:

```c
GPIOA_ENABLE();  // RCC->AHB1ENR |= Bit für GPIOA
GPIOB_ENABLE();  // RCC->AHB1ENR |= Bit für GPIOB
```

Ohne Clock Enable bleiben alle Schreibzugriffe auf GPIO-Register wirkungslos.

### Sicherheit: Debugger-Pins schützen

Bei GPIOA und GPIOB sind nur die Pins 11–0 für den Anwender verfügbar. **Pins 15–12 sind für den Debugger reserviert** (JTAG/SWD). Werden diese umkonfiguriert, kann der Debugger nicht mehr zugreifen und das Board ist nicht mehr programmierbar.

Schutzmassnahmen im Code:
- `reset_GPIO()` setzt Register auf Reset-Werte zurück
- `request_abort_startup()` prüft ob Taster T0–T3 gedrückt werden und stoppt ggf. vor der Konfiguration
- `config_check_GPIOA()` prüft ob MODER den erwarteten Wert hat

## Aufgaben und Umsetzung

### Aufgabe 1: GPIOA als Eingänge konfigurieren (PA0–PA2)

- **Ziel:** Drei Eingänge mit verschiedenen Pull-Konfigurationen: PA0=Pull-down, PA1=Pull-up, PA2=No Pull
- **Umsetzung:** `MODER` auf `00` (Input), `PUPDR` individuell pro Pin konfiguriert
- **Schlüssel-Code:**
  ```c
  GPIOA->MODER &= ~(0x3F << 0);
  GPIOA->PUPDR &= ~(0x3F << 0);
  GPIOA->PUPDR |= (0x02 << 0);  // PA0: Pull-down
  GPIOA->PUPDR |= (0x01 << 2);  // PA1: Pull-up
  ```

### Aufgabe 2: GPIOB als Ausgänge konfigurieren (PB0–PB2)

- **Ziel:** Drei Ausgänge: PB0=Push-Pull, PB1=Open Drain, PB2=Open Drain mit Pull-up; verschiedene Speeds
- **Umsetzung:** Vier Register konfiguriert (MODER, OTYPER, OSPEEDR, PUPDR) — jeweils Clear-then-Set
- **Schlüssel-Code:**
  ```c
  GPIOB->MODER   |= 0x00000015;  // 01 01 01 (Output)
  GPIOB->OTYPER  |= 0x00000006;  // 1 1 0 (OD, OD, PP)
  GPIOB->OSPEEDR |= 0x00000024;  // 10 01 00 (High, Med, Low)
  GPIOB->PUPDR   |= 0x00000010;  // 01 00 00 (PU, None, None)
  ```

### Aufgabe 3: Endlosschleife — DIP Switches → GPIO → LEDs

- **Ziel:** DIP Switches S10..S8 über GPIOB ausgeben, über GPIOA einlesen, auf LEDs anzeigen
- **Umsetzung:** `CT_DIPSW->BYTE.S15_8` lesen, `GPIOB->ODR` schreiben, `GPIOA->IDR` lesen, `CT_LED->BYTE` schreiben

### Aufgabe 4: I/O Kombinationen testen

- **Ziel:** Alle 9 Kombinationen (3 Outputs × 3 Inputs) durch Umstecken der Drähte testen
- **Ergebnis:** Push-Pull funktioniert immer; Open Drain ohne Pull-up + No Pull Input = undefiniert (kritisch)

## Wichtige Register / Hardware

| Register | Adresse/Pfad | Relevante Bits | Funktion |
|----------|-------------|----------------|----------|
| `GPIOA->MODER` | `0x40020000` | Bits [5:0] für PA0–PA2 (2 Bit/Pin) | Mode: `00`=Input, `01`=Output, `10`=AF, `11`=Analog |
| `GPIOA->PUPDR` | `0x4002000C` | Bits [5:0] für PA0–PA2 (2 Bit/Pin) | `00`=No Pull, `01`=Pull-up, `10`=Pull-down |
| `GPIOA->IDR` | `0x40020010` | Bits [2:0] für PA0–PA2 (read-only) | Input-Werte lesen |
| `GPIOB->MODER` | `0x40020400` | Bits [5:0] für PB0–PB2 (2 Bit/Pin) | Mode konfigurieren |
| `GPIOB->OTYPER` | `0x40020404` | Bits [2:0] für PB0–PB2 (**1 Bit/Pin**) | `0`=Push-Pull, `1`=Open Drain |
| `GPIOB->OSPEEDR` | `0x40020408` | Bits [5:0] für PB0–PB2 (2 Bit/Pin) | `00`=Low, `01`=Med, `10`=High, `11`=Very High |
| `GPIOB->PUPDR` | `0x4002040C` | Bits [5:0] für PB0–PB2 (2 Bit/Pin) | Pull-up/down konfigurieren |
| `GPIOB->ODR` | `0x40020414` | Bits [2:0] für PB0–PB2 | Output-Werte schreiben |

## Schlussfolgerungen

- **Clear-then-Set** ist das Standard-Pattern für Registerkonfiguration: Erst die betroffenen Bits mit invertierter Maske löschen (`&= ~mask`), dann den gewünschten Wert setzen (`|= value`). Nie direkt zuweisen, da sonst andere Bits des Registers überschrieben werden.
- **Push-Pull vs. Open Drain** ist eine fundamentale Designentscheidung: Push-Pull für einfache Ausgänge, Open Drain wenn Wired-OR oder Level-Shifting benötigt wird — aber immer mit Pull-up-Widerstand.
- **Floating Pins sind gefährlich**: Unbeschaltete Eingänge ohne Pull-Widerstand liefern undefinierte Werte und sind extrem störungsempfindlich. Immer Pull-up oder Pull-down konfigurieren.
- **OTYPER ist die Ausnahme**: Während alle anderen GPIO-Konfigurationsregister 2 Bits pro Pin verwenden, hat OTYPER nur 1 Bit pro Pin — ein häufiger Fehler beim Berechnen der Masken.
- **Debugger-Pins (PA/PB 12–15) nie anfassen**: Eine Fehlkonfiguration macht das Board unprogrammierbar. Der Schutzmechanismus (`request_abort_startup`, `config_check_GPIOA`) fängt dies softwareseitig ab.

## Verwandte Themen

- **Woche 1 (Coding Style):** Modulare Programmstruktur — GPIO-Konfiguration wird in eigene Funktionen (`init_GPIOA`, `init_GPIOB`) gekapselt
- **Woche 2 (Bus Cycles):** Volatile Pointer und Memory-Mapped I/O — gleicher Mechanismus, aber über externen Bus statt internen APB
- **Woche 4 (SPI):** GPIO im Alternate Function Mode (`MODER = 10`) für SPI-Pins — baut direkt auf der GPIO-Konfiguration dieser Woche auf
