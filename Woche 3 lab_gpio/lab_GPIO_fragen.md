# CT2 Praktikum - GPIO: Fragen & Antworten

## 4 Vorbereitungsfragen

### Frage 1: Wo sind `GPIOB` und `MODER` definiert?

> Ein Registerzugriff erfolgt beispielsweise über `GPIOB->MODER`. Dabei gibt `GPIOB` die Basisadresse an und `MODER` den Offset. Wo sind diese zwei Symbole definiert (Dateiname und Zeile)?

**Antwort:**

Beide sind in der Datei `reg_stm32f4xx.h` definiert (Teil des CTBoard14_DFP, Pfad: `C:\Arm\InES\CTBoard14_DFP\4.0.4\Device\Include\reg_stm32f4xx.h`):

- **`GPIOB`** ist als Makro definiert: `#define GPIOB ((reg_gpio_t *) 0x40020400)`
- **`MODER`** ist ein Feld innerhalb der Struktur `reg_gpio_t` (als `volatile uint32_t MODER;`)

---

### Frage 2: Was macht das Macro `#define GPIOB ((reg_gpio_t *) 0x40020400)`?

**Antwort:**

Das Makro castet die hexadezimale Speicheradresse `0x40020400` (Basisadresse von GPIO Port B im STM32F429ZI) zu einem Pointer auf die Struktur `reg_gpio_t`. Dadurch kann man über `GPIOB->MODER`, `GPIOB->ODR` etc. direkt auf die memory-mapped Register von GPIO Port B zugreifen, als wäre es eine gewoehnliche C-Struktur.

---

### Frage 3: Was enthält `struct reg_gpio_t`?

> Bitte beschreiben Sie in ein bis zwei Saetzen, was `struct reg_gpio_t` enthaelt.

**Antwort:**

Die Struktur `reg_gpio_t` bildet alle GPIO-Konfigurationsregister und Datenregister eines GPIO-Ports ab (MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFRL, AFRH). Die Felder sind als `volatile uint32_t` deklariert und entsprechen in ihrer Reihenfolge und ihrem Offset dem tatsaechlichen Registerlayout im Speicher des STM32F4xx.

---

### Frage 4: Wie funktioniert `GPIOB->MODER = 0x00000280;`?

> Erklaeren Sie, wie das C-Statement funktioniert. Wie erfolgt der Zugriff auf das Register?

**Antwort:**

1. `GPIOB` wird durch das Makro zu `((reg_gpio_t *) 0x40020400)` expandiert - ein Pointer auf die GPIO-B-Registerstruktur.
2. `->MODER` greift auf das Feld `MODER` innerhalb der Struktur zu. Da `MODER` das erste Feld ist, liegt es bei Offset `0x00`, also an Adresse `0x40020400 + 0x00 = 0x40020400`.
3. Der Wert `0x00000280` wird direkt an diese Speicheradresse geschrieben.
4. Da das Feld `volatile` ist, wird der Compiler den Zugriff nicht wegoptimieren - der Schreibvorgang findet garantiert auf der Hardware statt.

Der Wert `0x00000280 = 0b...0010_1000_0000` setzt:
- Bits [5:4] = `10` → PB2 als Alternate Function
- Bits [7:6] = `10` → PB3 als Alternate Function
(Dies ist der Reset-Wert von GPIOB, der PB3 fuer JTDO/TRACESWO konfiguriert.)

---

## 6.1 Konfiguration der Eingaenge: PA2..PA0

### Konfigurationstabelle GPIOA (Inputs)

#### Direction / Mode (MODER) - 2 Bit pro Pin

| Port | Konfig | Bits | Binaer | Clear-Maske (shift) | Clear-Maske (hex) | Set-Maske (shift) | Set-Maske (hex) |
|------|--------|------|--------|---------------------|-------------------|-------------------|-----------------|
| PA0  | Input  | 1..0 | 00     | `~(0x03 << 0)`     | `0xFFFFFFFC`      | `0x00 << 0`      | `0x00000000`    |
| PA1  | Input  | 3..2 | 00     | `~(0x03 << 2)`     | `0xFFFFFFF3`      | `0x00 << 2`      | `0x00000000`    |
| PA2  | Input  | 5..4 | 00     | `~(0x03 << 4)`     | `0xFFFFFFCF`      | `0x00 << 4`      | `0x00000000`    |

#### Pull-up / Pull-down (PUPDR) - 2 Bit pro Pin

| Port | Konfig     | Bits | Binaer | Clear-Maske (shift) | Clear-Maske (hex) | Set-Maske (shift) | Set-Maske (hex) |
|------|------------|------|--------|---------------------|-------------------|-------------------|-----------------|
| PA0  | Pull-down  | 1..0 | 10     | `~(0x03 << 0)`     | `0xFFFFFFFC`      | `0x02 << 0`      | `0x00000002`    |
| PA1  | Pull-up    | 3..2 | 01     | `~(0x03 << 2)`     | `0xFFFFFFF3`      | `0x01 << 2`      | `0x00000004`    |
| PA2  | No Pull    | 5..4 | 00     | `~(0x03 << 4)`     | `0xFFFFFFCF`      | `0x00 << 4`      | `0x00000000`    |

MODER-Werte: `00` = Input, `01` = GP Output, `10` = Alternate Function, `11` = Analog

PUPDR-Werte: `00` = No Pull, `01` = Pull-up, `10` = Pull-down, `11` = Reserved

---

### Frage 6.1a: Welche LEDs erwarten Sie hell, welche dunkel?

> Bevor Sie das Programm ausfuehren: Welche LEDs erwarten Sie bei der Ausfuehrung hell, welche dunkel?

**Antwort:**

Ohne externe Verdrahtung (Pins sind floating bzw. nur durch Pull-Widerstaende definiert):

- **LED16 (PA0, Pull-down): Dunkel** - Der Pull-down-Widerstand zieht den Eingang auf 0.
- **LED17 (PA1, Pull-up): Hell** - Der Pull-up-Widerstand zieht den Eingang auf 1.
- **LED18 (PA2, No Pull): Undefiniert** - Ohne Pull-Widerstand ist der Zustand des floating Eingangs nicht vorhersagbar.

| Hell | Dunkel | Undefiniert |
|------|--------|-------------|
| LED17 | LED16 | LED18 |

---

### Frage 6.1b: Entspricht das Ergebnis Ihren Erwartungen?

**Antwort:**

Ja, LED16 ist dunkel (Pull-down → 0), LED17 leuchtet (Pull-up → 1), und LED18 flackert moeglicherweise oder hat einen zufaelligen Zustand (floating).

---

### Frage 6.1c: Fingerprobe ueber P5 - Erklaerung des Verhaltens?

> Streichen Sie mit dem Finger ueber die Pins von P5 und beobachten Sie das Verhalten der LEDs. Wie erklaeren Sie sich dieses Verhalten?

**Antwort:**

Der menschliche Koerper wirkt als Antenne und koppelt elektromagnetische Stoerungen (z.B. 50Hz Netzfrequenz) auf die Pins ein. Die Auswirkungen:

- **PA0 (Pull-down):** Bleibt meistens dunkel, da der Pull-down-Widerstand den Pin stabil auf Low haelt. Nur bei starker Einkopplung koennte kurzes Flackern auftreten.
- **PA1 (Pull-up):** Bleibt meistens hell, da der Pull-up-Widerstand den Pin stabil auf High haelt.
- **PA2 (No Pull):** Reagiert am staerksten auf die Beruehrung und flackert deutlich, da kein Pull-Widerstand den Zustand stabilisiert. Der Pin ist hochohmig (floating) und extrem empfindlich gegenueber Stoerungen.

Dies zeigt die Wichtigkeit von Pull-up/Pull-down-Widerstaenden bei unbeschalteten Eingaengen.

---

## 6.2 Konfiguration der Ausgaenge: PB2..PB0

### Konfigurationstabelle GPIOB (Outputs)

#### Direction / Mode (MODER) - 2 Bit pro Pin

| Port | Konfig             | Bits | Binaer | Clear-Maske (shift) | Clear-Maske (hex) | Set-Maske (shift) | Set-Maske (hex) |
|------|--------------------|------|--------|---------------------|-------------------|-------------------|-----------------|
| PB0  | Gen. Purpose Output | 1..0 | 01    | `~(0x03 << 0)`     | `0xFFFFFFFC`      | `0x01 << 0`      | `0x00000001`    |
| PB1  | Gen. Purpose Output | 3..2 | 01    | `~(0x03 << 2)`     | `0xFFFFFFF3`      | `0x01 << 2`      | `0x00000004`    |
| PB2  | Gen. Purpose Output | 5..4 | 01    | `~(0x03 << 4)`     | `0xFFFFFFCF`      | `0x01 << 4`      | `0x00000010`    |

#### Type (OTYPER) - 1 Bit pro Pin

| Port | Konfig     | Bit | Binaer | Clear-Maske (shift) | Clear-Maske (hex) | Set-Maske (shift) | Set-Maske (hex) |
|------|------------|-----|--------|---------------------|-------------------|-------------------|-----------------|
| PB0  | Push-Pull  | 0   | 0      | `~(0x01 << 0)`     | `0xFFFFFFFE`      | `0x00 << 0`      | `0x00000000`    |
| PB1  | Open Drain | 1   | 1      | `~(0x01 << 1)`     | `0xFFFFFFFD`      | `0x01 << 1`      | `0x00000002`    |
| PB2  | Open Drain | 2   | 1      | `~(0x01 << 2)`     | `0xFFFFFFFB`      | `0x01 << 2`      | `0x00000004`    |

**Achtung:** OTYPER hat nur 1 Bit pro Pin (nicht 2 wie die anderen Register).

#### Pull-up / Pull-down (PUPDR) - 2 Bit pro Pin

| Port | Konfig          | Bits | Binaer | Clear-Maske (shift) | Clear-Maske (hex) | Set-Maske (shift) | Set-Maske (hex) |
|------|-----------------|------|--------|---------------------|-------------------|-------------------|-----------------|
| PB0  | No Pull-up/down | 1..0 | 00     | `~(0x03 << 0)`     | `0xFFFFFFFC`      | `0x00 << 0`      | `0x00000000`    |
| PB1  | No Pull-up/down | 3..2 | 00     | `~(0x03 << 2)`     | `0xFFFFFFF3`      | `0x00 << 2`      | `0x00000000`    |
| PB2  | Pull-up         | 5..4 | 01     | `~(0x03 << 4)`     | `0xFFFFFFCF`      | `0x01 << 4`      | `0x00000010`    |

#### Speed (OSPEEDR) - 2 Bit pro Pin

| Port | Konfig  | Bits | Binaer | Clear-Maske (shift) | Clear-Maske (hex) | Set-Maske (shift) | Set-Maske (hex) |
|------|---------|------|--------|---------------------|-------------------|-------------------|-----------------|
| PB0  | Low     | 1..0 | 00     | `~(0x03 << 0)`     | `0xFFFFFFFC`      | `0x00 << 0`      | `0x00000000`    |
| PB1  | Medium  | 3..2 | 01     | `~(0x03 << 2)`     | `0xFFFFFFF3`      | `0x01 << 2`      | `0x00000004`    |
| PB2  | High    | 5..4 | 10     | `~(0x03 << 4)`     | `0xFFFFFFCF`      | `0x02 << 4`      | `0x00000020`    |

OSPEEDR-Werte: `00` = Low, `01` = Medium, `10` = High, `11` = Very High

---

## 6.5 Untersuchung verschiedener I/O Kombinationen

### Ergebnistabelle

|  | **PA0 / LED16: Pull-Down** | **PA1 / LED17: Pull-Up** | **PA2 / LED18: No Pull** |
|--|---------------------------|-------------------------|-------------------------|
| **PB0 / S8: Push-Pull, No Pull** | 0: **0**, 1: **1** | 0: **0**, 1: **1** | 0: **0**, 1: **1** |
| **PB1 / S9: Open Drain, No Pull** | 0: **0**, 1: **0** | 0: **0**, 1: **1** | 0: **0**, 1: **undefiniert** |
| **PB2 / S10: Open Drain, Pull-up** | 0: **0**, 1: **1** | 0: **0**, 1: **1** | 0: **0**, 1: **1** |

### Erklaerung der Ergebnisse

- **Push-Pull (PB0):** Kann aktiv High und Low treiben. Funktioniert zuverlaessig mit allen Eingangs-Konfigurationen, da der Ausgang immer einen definierten Pegel liefert.

- **Open Drain ohne Pull-up (PB1):** Kann nur aktiv Low treiben. Bei High wird der Ausgang hochohmig (floating):
  - Mit **Pull-down** (PA0): Bei "1" wird 0 gelesen, da der Pull-down den floating Pin auf Low zieht.
  - Mit **Pull-up** (PA1): Bei "1" wird 1 gelesen, da der Pull-up den floating Pin auf High zieht.
  - Mit **No Pull** (PA2): Bei "1" ist der Zustand undefiniert (floating + floating).

- **Open Drain mit Pull-up (PB2):** Der externe Pull-up am Ausgang zieht den Pin bei "1" auf High. Funktioniert daher zuverlaessig mit allen Eingangs-Konfigurationen.

---

### Frage: Gibt es kritische Kombinationen?

**Antwort:**

Ja, die Kombination **Open Drain ohne Pull-up (PB1)** mit **No Pull Input (PA2)** ist kritisch. Wenn der Open-Drain-Ausgang High ausgibt (also hochohmig ist) und der Eingang keinen Pull-Widerstand hat, sind beide Seiten floating. Der gelesene Wert ist undefiniert und stoerungsanfaellig. In der Praxis muss bei Open-Drain-Ausgaengen immer ein Pull-up-Widerstand vorhanden sein (entweder am Ausgang oder am Eingang), damit ein definierter High-Pegel entsteht.

Generell ist auch die Kombination **Open Drain ohne Pull-up** mit **Pull-down Input** problematisch, da der Pull-down den gewuenschten High-Pegel verhindert.
