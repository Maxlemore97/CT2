# Woche 6 — Timer und PWM (Pulse Width Modulation)

## Überblick

In dieser Woche wurden die General-Purpose Timer TIM3 und TIM4 des STM32F429ZI eingesetzt, um RGB-LEDs über PWM-Signale anzusteuern und einen periodischen Interrupt im Sekundentakt zu erzeugen. TIM4 arbeitet als Downcounter mit Update-Interrupt, TIM3 erzeugt drei synchrone PWM-Signale mit individuellen Duty Cycles für die Farben Rot, Grün und Blau.

## Lernziele

- Mit einem Timer ein periodisches Interrupt erzeugen und eine ISR implementieren
- Synchrone PWM-Signale mit unterschiedlichen Duty Cycles erzeugen
- Funktion von Reload-Register (ARR) und Compare-Registern (CCRx) erklären

## Kernkonzepte

### Timer-Grundlagen

Die General-Purpose Timer TIM2–TIM5 des STM32F4xx sind 16-Bit-Zähler (TIM3/TIM4) bzw. 32-Bit-Zähler (TIM2/TIM5), die vom internen Clock CK_INT (84 MHz am APB1-Bus) getaktet werden.

**Zählkette:**

```
CK_INT (84 MHz) → Prescaler (PSC) → CK_CNT → Counter (CNT) → Overflow/Underflow → Update Event (UIF)
```

**Zählfrequenz:**

$$f_{CNT} = \frac{f_{CK\_INT}}{PSC + 1}$$

**Periodendauer:**

$$T_{period} = \frac{(ARR + 1)}{f_{CNT}} = \frac{(PSC + 1) \cdot (ARR + 1)}{f_{CK\_INT}}$$

> **Wichtig:** Der Prescaler-Wert im Register ist um 1 kleiner als der tatsächliche Teiler. `PSC = 0` bedeutet Teiler 1 (kein Teilen), `PSC = 8399` bedeutet Teiler 8400.

### Timer als Interrupt-Quelle (TIM4)

Ein Timer kann bei jedem Overflow (Upcounter) oder Underflow (Downcounter) ein **Update Event** auslösen, das im Status-Register als **UIF** (Update Interrupt Flag) gesetzt wird.

**Konfigurationsschritte:**

1. Peripheral Clock aktivieren (`RCC->APB1ENR`)
2. Prescaler setzen (`TIMx->PSC`)
3. Auto-Reload-Wert setzen (`TIMx->ARR`)
4. Zählrichtung wählen (`TIMx->CR1`, Bit DIR)
5. Update-Interrupt aktivieren (`TIMx->DIER`, Bit UIE)
6. Timer starten (`TIMx->CR1`, Bit CEN)
7. Interrupt im NVIC freischalten (`NVIC->ISER0`)

**ISR-Pflicht:** In der Interrupt Service Routine muss das UIF-Bit **manuell gelöscht** werden, da es von der Hardware nur gesetzt, nicht automatisch zurückgesetzt wird:

```c
TIM4->SR &= ~(0x00000001);  // UIF löschen
```

### PWM-Erzeugung (TIM3)

PWM (Pulse Width Modulation) erzeugt ein Rechtecksignal mit einstellbarem **Tastgrad** (Duty Cycle). Die Periodendauer wird durch ARR bestimmt, der Duty Cycle durch die Compare-Register CCRx.

**PWM Mode 1 (Upcounting):**
- Output ist **HIGH** solange `CNT < CCRx`
- Output ist **LOW** solange `CNT >= CCRx`

```
     ┌──────────┐                    ┌──────────┐
     │          │                    │          │
─────┘          └────────────────────┘          └────
     |← CCRx →|←   ARR - CCRx     →|
     |←         ARR + 1            →|

     Duty Cycle = CCRx / (ARR + 1)
```

**Daraus folgt:**
- `CCRx = 0` → 0% Duty Cycle (immer LOW)
- `CCRx = ARR + 1` → 100% Duty Cycle (immer HIGH)
- `CCRx = (ARR + 1) / 2` → 50% Duty Cycle

### Output Compare Mode

Jeder Timer-Kanal hat ein Capture/Compare Mode Register (CCMR), das den Output-Modus festlegt:

| OCxM[2:0] | Modus | Beschreibung |
|-----------|-------|-------------|
| `000` | Frozen | Kein Einfluss auf Output |
| `001` | Active on match | HIGH bei CNT = CCRx |
| `010` | Inactive on match | LOW bei CNT = CCRx |
| `011` | Toggle | Umschalten bei CNT = CCRx |
| `110` | **PWM Mode 1** | HIGH wenn CNT < CCRx (Upcounting) |
| `111` | PWM Mode 2 | LOW wenn CNT < CCRx (invertiert) |

Für PWM-Erzeugung wird **Mode 1 (`110`)** verwendet.

### Register-Übersicht TIM2–TIM5

| Register | Offset | Funktion |
|----------|--------|----------|
| `CR1` | `0x00` | Control: CEN, DIR, CMS, ARPE, CKD |
| `CR2` | `0x04` | Control 2: Master mode (MMS), TI1S |
| `SMCR` | `0x08` | Slave mode: Externe Clock, Trigger |
| `DIER` | `0x0C` | Interrupt/DMA enable: UIE, CCxIE |
| `SR` | `0x10` | Status: UIF, CCxIF |
| `EGR` | `0x14` | Event generation: UG, CCxG |
| `CCMR1` | `0x18` | Capture/Compare mode CH1+CH2: OCxM, CCxS |
| `CCMR2` | `0x1C` | Capture/Compare mode CH3+CH4: OCxM, CCxS |
| `CCER` | `0x20` | Capture/Compare enable: CCxE, CCxP |
| `CNT` | `0x24` | Counter value |
| `PSC` | `0x28` | Prescaler value (Teiler = PSC + 1) |
| `ARR` | `0x2C` | Auto-reload value |
| `CCR1` | `0x34` | Capture/Compare register Channel 1 |
| `CCR2` | `0x38` | Capture/Compare register Channel 2 |
| `CCR3` | `0x3C` | Capture/Compare register Channel 3 |
| `CCR4` | `0x40` | Capture/Compare register Channel 4 |

### CR1-Register im Detail

```
Bit: 15-10  9:8   7    6:5  4    3    2    1    0
     Res.  CKD  ARPE  CMS  DIR  OPM  URS  UDIS CEN
```

| Bit | Name | Verwendung in diesem Lab |
|-----|------|--------------------------|
| 4 | DIR | 0 = Upcounter (TIM3), 1 = Downcounter (TIM4) |
| 0 | CEN | 1 = Counter enabled (zuletzt setzen!) |

### DIER-Register — Interrupt Enable

| Bit | Name | Funktion |
|-----|------|----------|
| 0 | UIE | Update Interrupt Enable (Overflow/Underflow) |
| 1–4 | CC1IE–CC4IE | Capture/Compare Interrupt Enable |

### CCER-Register — Output Enable

| Bit | Name | Funktion |
|-----|------|----------|
| 0 | CC1E | Channel 1 Output Enable |
| 4 | CC2E | Channel 2 Output Enable |
| 8 | CC3E | Channel 3 Output Enable |
| 12 | CC4E | Channel 4 Output Enable |

### Prescaler- und Reload-Berechnung

**TIM4 — 1-Sekunden-Interrupt:**

$$f_{CNT} = \frac{84\,\text{MHz}}{PSC + 1} = 10\,\text{kHz} \quad \Rightarrow \quad PSC = 8399$$

$$T = \frac{ARR + 1}{f_{CNT}} = \frac{10000}{10\,\text{kHz}} = 1\,\text{s} \quad \Rightarrow \quad ARR = 9999$$

**TIM3 — 200 Hz PWM:**

$$\frac{84\,\text{MHz}}{(PSC + 1) \cdot (ARR + 1)} = 200\,\text{Hz} \quad \Rightarrow \quad (PSC + 1) \cdot (ARR + 1) = 420{,}000$$

Für maximale Auflösung: $PSC = 6$ (Teiler 7), $ARR = 59{,}999$

$$f_{PWM} = \frac{84\,\text{MHz}}{7 \cdot 60{,}000} = 200\,\text{Hz} \quad \checkmark$$

### Duty-Cycle-Skalierung

Die DIP Switches liefern 4-Bit-Werte (0x0–0xF). Diese müssen auf den vollen CCR-Bereich skaliert werden:

- 0x0 → 0% → CCRx = 0
- 0xF → 100% → CCRx = ARR + 1 = 60000

**Schrittweite:** $\frac{60{,}000}{15} = 4{,}000$

**Skalierung:** `CCRx = switch_value * 4000`

### NVIC — Nested Vectored Interrupt Controller

Der NVIC verwaltet alle Interrupts des Cortex-M4. Jeder Interrupt hat eine eindeutige **IRQ-Nummer** und muss im NVIC freigeschaltet werden:

```c
NVIC->ISER0 |= (1 << 30);  // TIM4 = IRQ #30
```

Die ISR muss den **exakten Funktionsnamen** haben, der im Startup-File definiert ist (z.B. `TIM4_IRQHandler`). Ein falscher Name führt dazu, dass der Default Handler aufgerufen wird.

### GPIO Alternate Function für PWM

Die Timer-Ausgänge werden über GPIO-Pins im Alternate Function Modus ausgegeben. TIM3 verwendet:

| Timer-Kanal | GPIO-Pin | CT-Board Port | AF |
|-------------|----------|---------------|-----|
| TIM3_CH1 | PB4 | P6.4 | AF2 |
| TIM3_CH2 | PB5 | P6.5 | AF2 |
| TIM3_CH3 | PB0 | P6.0 | AF2 |

## Schlussfolgerungen

- **PSC + 1 ist der tatsächliche Teiler**: Das Register enthält den Teilerwert minus 1. PSC = 0 teilt nicht, PSC = 8399 teilt durch 8400. Gleiches gilt für ARR bei der Periodenberechnung.
- **CEN zuletzt setzen**: Wie bei SPI-SPE gilt auch hier: Alle Konfiguration abschliessen, bevor der Timer gestartet wird.
- **UIF manuell löschen**: Im Gegensatz zu manchen Peripherien löscht die Hardware das UIF-Flag nicht automatisch. Vergisst man das Löschen in der ISR, wird der Interrupt sofort wieder ausgelöst (endlose ISR-Aufrufe).
- **PWM Mode 1 im Upcounting**: `OCxREF = 1` wenn `CNT < CCRx`. Das bedeutet: Je grösser der CCR-Wert, desto grösser der Duty Cycle. CCR = 0 → 0%, CCR > ARR → 100%.
- **Skalierung über ganzzahlige Multiplikation**: Die 4-Bit DIP-Switch-Werte (0–15) werden mit der Schrittweite (ARR+1)/15 multipliziert. Bei ARR = 59999 ergibt sich die Schrittweite 4000 — eine glatte Zahl dank geeigneter Prescaler/ARR-Wahl.

## Verwandte Themen

- **Woche 3 (GPIO):** GPIO Alternate Function Mode — die PWM-Pins werden als AF2 für TIM3 konfiguriert
- **Woche 4/5 (SPI):** Polling vs. Interrupt — SPI nutzt Polling (busy waiting), Timer nutzt Interrupts (CPU kann anderes tun)
- **Woche 7 (Interrupts):** Vertiefung des NVIC und der Interrupt-Priorisierung
