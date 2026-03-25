# CT2 Praktikum: General Purpose Input/Output (GPIO)

## 1. Einleitung

GPIOs erlauben eine universelle Verwendung von Ein- und Ausgabepins. Dazu müssen sie vor dem Zugriff konfiguriert werden. Lesen und Schreiben von Werten geschieht in der Regel mittels Register. Die Datei `reg_stm32f4xx.h` definiert Strukturen und Makros für diese Registerzugriffe.

```
Application  →  reg_stm32f4xx.h  →  Registers  →  GPIO A / GPIO B
```

In diesem Praktikum realisieren wir die Konfiguration und die Ein- und Ausgabe mittels Registerzugriff für die GPIO der Prozessorfamilie STM32F4xx.

## 2. Lernziele

- GPIO mit Hilfe von technischen Dokumentationen initialisieren
- GPIO zur Ein- und Ausgabe verwenden
- Wissen zu den verschiedenen Konfigurationsoptionen festigen
- Strukturen des untersten Hardware Abstraction Layers (HAL) einsetzen: Basisadressen, Macros und Structs in `reg_stm32f4xx.h` zur Konfiguration von Registern

## 3. Material

- 1x CT Board
- 3x Verbindungsdrähte (female-female)
- 2x Schutzkappen

## 4. Vorbereitungsfragen

### Frage 1: Wo sind `GPIOB` und `MODER` definiert?

> Ein Registerzugriff erfolgt beispielsweise über `GPIOB->MODER`. Dabei gibt `GPIOB` die Basisadresse an und `MODER` den Offset. Wo sind diese zwei Symbole definiert?

**Antwort:**

Beide sind in `reg_stm32f4xx.h` definiert:
- **`GPIOB`** als Makro: `#define GPIOB ((reg_gpio_t *) 0x40020400)`
- **`MODER`** als Feld in der Struktur `reg_gpio_t`: `volatile uint32_t MODER;`

---

### Frage 2: Was macht das Macro `#define GPIOB ((reg_gpio_t *) 0x40020400)`?

**Antwort:**

Das Makro castet die hexadezimale Speicheradresse `0x40020400` (Basisadresse von GPIO Port B im STM32F429ZI) zu einem Pointer auf die Struktur `reg_gpio_t`. Dadurch kann man über `GPIOB->MODER`, `GPIOB->ODR` etc. direkt auf die memory-mapped Register von GPIO Port B zugreifen, als wäre es eine gewöhnliche C-Struktur.

---

### Frage 3: Was enthält `struct reg_gpio_t`?

**Antwort:**

Die Struktur `reg_gpio_t` bildet alle GPIO-Konfigurationsregister und Datenregister eines GPIO-Ports ab (MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFRL, AFRH). Die Felder sind als `volatile uint32_t` deklariert und entsprechen in ihrer Reihenfolge und ihrem Offset dem tatsächlichen Registerlayout im Speicher des STM32F4xx.

---

### Frage 4: Wie funktioniert `GPIOB->MODER = 0x00000280;`?

**Antwort:**

1. `GPIOB` wird durch das Makro zu `((reg_gpio_t *) 0x40020400)` expandiert — ein Pointer auf die GPIO-B-Registerstruktur
2. `->MODER` greift auf das Feld `MODER` innerhalb der Struktur zu. Da `MODER` das erste Feld ist, liegt es bei Offset `0x00`, also an Adresse $0\text{x}40020400 + 0\text{x}00 = 0\text{x}40020400$
3. Der Wert `0x00000280` wird direkt an diese Speicheradresse geschrieben
4. Da das Feld `volatile` ist, wird der Compiler den Zugriff nicht wegoptimieren

Der Wert `0x00000280` = `0b...0010_1000_0000` setzt:
- Bits [5:4] = `10` → PB2 als Alternate Function
- Bits [7:6] = `10` → PB3 als Alternate Function

(Dies ist der Reset-Wert von GPIOB, der PB3 für JTDO/TRACESWO konfiguriert.)

## 5. Applikation und Testaufbau

GPIO A und B sind direkt auf die Ports P5 und P6 des CT-Boards herausgeführt.

**Pin-Mapping (P5 = GPIOA, P6 = GPIOB):**

| Pin-Nr | P5 (GPIOA) | P6 (GPIOB) |
|--------|-----------|-----------|
| 1 | PA0 | PB0 |
| 2 | PA1 | PB1 |
| 3 | PA2 | PB2 |
| … | … | … |
| 12 | PA11 | PB11 |
| 13 | +3V3 | +3V3 |
| 14–15 | GND | GND |
| 16 | +5V0 | +5V0 |

**Signalfluss:**

```
DIP Switches S10..S8  →  GPIOB (PB2..PB0, Output)  →  Drähte  →  GPIOA (PA2..PA0, Input)  →  LED18..LED16
        ↓                        ↓
    LED10..LED8              LED2..LED0
   (Debug: Switch-Werte)   (Debug: ODR Rücklesen)
```

> **Warnung:** Stellen Sie noch keine Verbindung zwischen den Pins auf dem CT-Board her, bevor die GPIO korrekt konfiguriert sind!

## 6. Aufgaben

**GPIO-Konfigurationsübersicht:**

| GPIO | Mode/Type | Pull-up/Pull-down | Speed | Observe on |
|------|-----------|-------------------|-------|-----------|
| PB0 | Push-Pull Output | No Pull | Low Speed | LED0 |
| PB1 | Open Drain Output | No Pull | Medium Speed | LED1 |
| PB2 | Open Drain Output | Pull-up | High Speed | LED2 |
| PA0 | Input | Pull-down | — | LED16 |
| PA1 | Input | Pull-up | — | LED17 |
| PA2 | Input | No Pull | — | LED18 |

---

### 6.1 Konfiguration der Eingänge: PA2..PA0

**Aufgabe:** Überlegen, welche Werte für die Konfiguration der Eingänge in die Konfigurationsregister von GPIOA geschrieben werden müssen. Tabelle ausfüllen, implementieren, testen.

#### Konfigurationstabelle GPIOA (Inputs)

**Direction / Mode (MODER) — 2 Bit pro Pin:**

| Port | Konfig | Bits | Binär | Clear-Maske (shift) | Clear-Maske (hex) | Set-Maske (shift) | Set-Maske (hex) |
|------|--------|------|-------|---------------------|-------------------|-------------------|-----------------|
| PA0 | Input | 1..0 | `00` | `~(0x03 << 0)` | `0xFFFFFFFC` | `0x00 << 0` | `0x00000000` |
| PA1 | Input | 3..2 | `00` | `~(0x03 << 2)` | `0xFFFFFFF3` | `0x00 << 2` | `0x00000000` |
| PA2 | Input | 5..4 | `00` | `~(0x03 << 4)` | `0xFFFFFFCF` | `0x00 << 4` | `0x00000000` |

MODER-Werte: `00` = Input, `01` = GP Output, `10` = Alternate Function, `11` = Analog

**Pull-up / Pull-down (PUPDR) — 2 Bit pro Pin:**

| Port | Konfig | Bits | Binär | Clear-Maske (shift) | Clear-Maske (hex) | Set-Maske (shift) | Set-Maske (hex) |
|------|--------|------|-------|---------------------|-------------------|-------------------|-----------------|
| PA0 | Pull-down | 1..0 | `10` | `~(0x03 << 0)` | `0xFFFFFFFC` | `0x02 << 0` | `0x00000002` |
| PA1 | Pull-up | 3..2 | `01` | `~(0x03 << 2)` | `0xFFFFFFF3` | `0x01 << 2` | `0x00000004` |
| PA2 | No Pull | 5..4 | `00` | `~(0x03 << 4)` | `0xFFFFFFCF` | `0x00 << 4` | `0x00000000` |

PUPDR-Werte: `00` = No Pull, `01` = Pull-up, `10` = Pull-down, `11` = Reserved

**Lösung** (`init_GPIOA()`):

```c
// 1. Set Mode to Input (00) for pins 0, 1, 2
GPIOA->MODER &= ~(0x3F << 0); // Clear bits 0-5

// 2. Configure Pull-up/down
GPIOA->PUPDR &= ~(0x3F << 0); // Clear PUPDR bits 0-5
GPIOA->PUPDR |= (0x02 << 0);  // PA0: Pull-down (10)
GPIOA->PUPDR |= (0x01 << 2);  // PA1: Pull-up (01)
// PA2 remains 00 (No pull)
```

---

#### Frage 6.1a: Welche LEDs erwarten Sie hell, welche dunkel?

> Bevor Sie das Programm ausführen: Welche LEDs erwarten sie bei der Ausführung hell, welche dunkel?

**Antwort:**

Ohne externe Verdrahtung (Pins sind floating bzw. nur durch Pull-Widerstände definiert):

| Hell | Dunkel | Undefiniert |
|------|--------|-------------|
| LED17 (PA1, Pull-up → 1) | LED16 (PA0, Pull-down → 0) | LED18 (PA2, No Pull → floating) |

---

#### Frage 6.1b: Entspricht das Ergebnis Ihren Erwartungen?

**Antwort:** Ja, LED16 ist dunkel (Pull-down → 0), LED17 leuchtet (Pull-up → 1), und LED18 flackert möglicherweise oder hat einen zufälligen Zustand (floating).

---

#### Frage 6.1c: Fingerprobe über P5 — Erklärung des Verhaltens?

> Streichen Sie mit dem Finger über die Pins von P5 und beobachten Sie das Verhalten der LEDs. Wie erklären Sie sich dieses Verhalten?

**Antwort:**

Der menschliche Körper wirkt als Antenne und koppelt elektromagnetische Störungen (z.B. 50 Hz Netzfrequenz) auf die Pins ein:

- **PA0 (Pull-down):** Bleibt meistens dunkel — der Pull-down-Widerstand hält den Pin stabil auf Low
- **PA1 (Pull-up):** Bleibt meistens hell — der Pull-up-Widerstand hält den Pin stabil auf High
- **PA2 (No Pull):** Reagiert am stärksten und flackert deutlich — kein Pull-Widerstand stabilisiert den Zustand, der Pin ist hochohmig (floating) und extrem empfindlich gegenüber Störungen

> Dies zeigt die Wichtigkeit von Pull-up/Pull-down-Widerständen bei unbeschalteten Eingängen.

---

### 6.2 Konfiguration der Ausgänge: PB2..PB0

**Aufgabe:** Registerzugriffe für eine Output-Konfiguration implementieren. Registerbits zuerst löschen und dann setzen.

#### Konfigurationstabelle GPIOB (Outputs)

**Direction / Mode (MODER) — 2 Bit pro Pin:**

| Port | Konfig | Bits | Binär | Clear-Maske (hex) | Set-Maske (hex) |
|------|--------|------|-------|-------------------|-----------------|
| PB0 | Gen. Purpose Output | 1..0 | `01` | `0xFFFFFFFC` | `0x00000001` |
| PB1 | Gen. Purpose Output | 3..2 | `01` | `0xFFFFFFF3` | `0x00000004` |
| PB2 | Gen. Purpose Output | 5..4 | `01` | `0xFFFFFFCF` | `0x00000010` |

**Type (OTYPER) — 1 Bit pro Pin:**

| Port | Konfig | Bit | Binär | Clear-Maske (hex) | Set-Maske (hex) |
|------|--------|-----|-------|-------------------|-----------------|
| PB0 | Push-Pull | 0 | `0` | `0xFFFFFFFE` | `0x00000000` |
| PB1 | Open Drain | 1 | `1` | `0xFFFFFFFD` | `0x00000002` |
| PB2 | Open Drain | 2 | `1` | `0xFFFFFFFB` | `0x00000004` |

> **Achtung:** OTYPER hat nur **1 Bit pro Pin** (nicht 2 wie die anderen Register).

**Pull-up / Pull-down (PUPDR) — 2 Bit pro Pin:**

| Port | Konfig | Bits | Binär | Clear-Maske (hex) | Set-Maske (hex) |
|------|--------|------|-------|-------------------|-----------------|
| PB0 | No Pull | 1..0 | `00` | `0xFFFFFFFC` | `0x00000000` |
| PB1 | No Pull | 3..2 | `00` | `0xFFFFFFF3` | `0x00000000` |
| PB2 | Pull-up | 5..4 | `01` | `0xFFFFFFCF` | `0x00000010` |

**Speed (OSPEEDR) — 2 Bit pro Pin:**

| Port | Konfig | Bits | Binär | Clear-Maske (hex) | Set-Maske (hex) |
|------|--------|------|-------|-------------------|-----------------|
| PB0 | Low | 1..0 | `00` | `0xFFFFFFFC` | `0x00000000` |
| PB1 | Medium | 3..2 | `01` | `0xFFFFFFF3` | `0x00000004` |
| PB2 | High | 5..4 | `10` | `0xFFFFFFCF` | `0x00000020` |

OSPEEDR-Werte: `00` = Low, `01` = Medium, `10` = High, `11` = Very High

**Lösung** (`init_GPIOB()`):

```c
// 1. Configure GPIOB Pins 0, 1, 2 as General Purpose Output (Mode 01)
GPIOB->MODER &= ~(0x0000003F);
GPIOB->MODER |=  (0x00000015);  // 01 01 01

// 2. Configure Output Type (OTYPER)
// PB0: Push-Pull (0), PB1: Open Drain (1), PB2: Open Drain (1)
GPIOB->OTYPER &= ~(0x00000007);
GPIOB->OTYPER |=  (0x00000006);  // 110

// 3. Configure Output Speed (OSPEEDR)
// PB0: Low (00), PB1: Medium (01), PB2: High (10)
GPIOB->OSPEEDR &= ~(0x0000003F);
GPIOB->OSPEEDR |=  (0x00000024);  // 10 01 00

// 4. Configure Pull-up/Pull-down (PUPDR)
// PB0: No Pull (00), PB1: No Pull (00), PB2: Pull-up (01)
GPIOB->PUPDR &= ~(0x0000003F);
GPIOB->PUPDR |=  (0x00000010);  // 01 00 00
```

---

### 6.3 Ausgabe auf PB2..PB0

**Aufgabe:** Endlosschleife implementieren: DIP Switches S10..S8 einlesen, auf LED10..LED8 und in ODR ausgeben, ODR zurücklesen und auf LED2..LED0 anzeigen, GPIOA Input auf LED18..LED16 anzeigen.

**Lösung** (Endlosschleife in `main()`):

```c
while (1) {
    // 1. Read DIP switches S10..S8
    data_dip_switch = (uint8_t)(CT_DIPSW->BYTE.S15_8 & MASK_3_BITS);

    // 2. Display DIP switches on LED10..LED8 for debugging
    CT_LED->BYTE.LED15_8 = data_dip_switch;

    // 3. Write the switch values to the GPIOB Output Data Register
    GPIOB->ODR = (uint32_t)data_dip_switch;

    // 4. Read back from GPIOB ODR and show on LED2..LED0
    CT_LED->BYTE.LED7_0 = (uint8_t)(GPIOB->ODR & MASK_3_BITS);

    // 5. Read input from GPIOA and show on LED18..LED16
    data_gpio_in = (uint16_t)(GPIOA->IDR & MASK_3_BITS);
    CT_LED->BYTE.LED23_16 = (uint8_t)data_gpio_in;
}
```

---

### 6.4 Vervollständigen des Aufbaus

> **Warnung:** CT-Board von der Versorgungsspannung trennen, bevor Drähte angebracht werden!

Verbindungen herstellen: PA0↔PB0, PA1↔PB1, PA2↔PB2.

**Frage:** Stimmen die Anzeigen auf allen LEDs miteinander überein?

**Antwort:** Ja.

---

### 6.5 Untersuchung verschiedener I/O Kombinationen

**Aufgabe:** Durch Umstecken der Verbindungsdrähte alle Kombinationen von Aus- und Eingängen verschaffen. Alle Kombinationen prüfen und Ergebnisse eintragen.

#### Ergebnistabelle

|  | **PA0 / LED16: Pull-Down** | **PA1 / LED17: Pull-Up** | **PA2 / LED18: No Pull** |
|--|---------------------------|-------------------------|-------------------------|
| **PB0 / S8: Push-Pull, No Pull** | 0: **0**, 1: **1** | 0: **0**, 1: **1** | 0: **0**, 1: **1** |
| **PB1 / S9: Open Drain, No Pull** | 0: **0**, 1: **0** | 0: **0**, 1: **1** | 0: **0**, 1: **undef** |
| **PB2 / S10: Open Drain, Pull-up** | 0: **0**, 1: **1** | 0: **0**, 1: **1** | 0: **0**, 1: **1** |

#### Erklärung der Ergebnisse

- **Push-Pull (PB0):** Kann aktiv High und Low treiben. Funktioniert zuverlässig mit allen Eingangs-Konfigurationen, da der Ausgang immer einen definierten Pegel liefert.

- **Open Drain ohne Pull-up (PB1):** Kann nur aktiv Low treiben. Bei High wird der Ausgang hochohmig (floating):
  - Mit **Pull-down** (PA0): Bei "1" wird 0 gelesen, da der Pull-down den floating Pin auf Low zieht
  - Mit **Pull-up** (PA1): Bei "1" wird 1 gelesen, da der Pull-up den floating Pin auf High zieht
  - Mit **No Pull** (PA2): Bei "1" ist der Zustand undefiniert (floating + floating)

- **Open Drain mit Pull-up (PB2):** Der externe Pull-up am Ausgang zieht den Pin bei "1" auf High. Funktioniert daher zuverlässig mit allen Eingangs-Konfigurationen.

#### Frage: Gibt es kritische Kombinationen?

**Antwort:**

Ja, die Kombination **Open Drain ohne Pull-up (PB1)** mit **No Pull Input (PA2)** ist kritisch. Wenn der Open-Drain-Ausgang High ausgibt (also hochohmig ist) und der Eingang keinen Pull-Widerstand hat, sind beide Seiten floating. Der gelesene Wert ist undefiniert und störungsanfällig. In der Praxis muss bei Open-Drain-Ausgängen immer ein Pull-up-Widerstand vorhanden sein (entweder am Ausgang oder am Eingang), damit ein definierter High-Pegel entsteht.

---

### 7. Bewertung

| Bewertungskriterien | Gewichtung |
|---------------------|-----------|
| GPIO Inputs gemäss Aufgabe implementiert, Fragen erklärt | 1/4 |
| Output-Konfiguration implementiert und getestet | 1/4 |
| GPIO Outputs gemäss Anforderungen getestet | 1/4 |
| I/O Kombinationen komplett untersucht, auffällige Ergebnisse erklärt | 1/4 |

## 8. Anhang

### 8.1 Rücksetzen der Debugger Pins (Software Methode)

Ganz am Anfang des Programms werden die Taster 0 bis 3 abgefragt, ob diese gedrückt werden. Sollte jemand aus Versehen die Debugger Pins umkonfiguriert haben, kann kurz nach einem Reset einer der Taster 0 bis 3 gedrückt werden, um das Programm zu stoppen bevor der Studentencode ausgeführt wird.

### 8.2 Rücksetzen der Debugger Pins (Hardware Methode)

> **Nur verwenden, falls die GPIO Pins des Debuggers durch einen fehlerhaften Praktikumscode falsch konfiguriert wurden und kein Code mehr aufs Board geladen werden kann.**

Bei GPIOA und GPIOB sind nur die Pins 11-0 direkt abgreifbar. Die Pins 15-12 werden intern vom Discovery Board verwendet (z.B. Verbindung zum Debugger). Falls diese umkonfiguriert werden, kann nicht mehr über die Debug-Schnittstelle zugegriffen werden. Lösung: Aus dem System Memory booten (BOOT0 → +3V3), dann Reset, dann Verbindung trennen.
