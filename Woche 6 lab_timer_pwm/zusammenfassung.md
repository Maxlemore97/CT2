# Woche 6 — Timer und PWM (Pulse Width Modulation)

## Überblick

In dieser Woche geht es darum, LEDs in verschiedenen Farben leuchten zu lassen. Dazu brauchen wir zwei eingebaute "Stoppuhren" (Timer) im Mikrocontroller:

- **TIM4** zählt jede Sekunde herunter und löst dann einen Alarm (Interrupt) aus → LEDs blinken
- **TIM3** erzeugt drei schnelle An/Aus-Signale (PWM), die bestimmen, wie hell Rot, Grün und Blau leuchten → daraus entsteht jede beliebige Mischfarbe

## Lernziele

- Mit einem Timer ein periodisches Interrupt erzeugen und eine ISR implementieren
- Synchrone PWM-Signale mit unterschiedlichen Duty Cycles erzeugen
- Funktion von Reload-Register (ARR) und Compare-Registern (CCRx) erklären

## Kernkonzepte

### Was ist ein Timer?

Ein Timer ist im Grunde ein **Zähler**, der automatisch im Takt hochzählt (oder runterzählt). Man kann einstellen:
- **Wie schnell** er zählt (Prescaler)
- **Bis wohin** er zählt (Auto-Reload-Wert)
- **Was passiert**, wenn er am Ende ankommt (z.B. Interrupt auslösen oder Signal umschalten)

**Analogie:** Stell dir eine Küchenuhr vor. Du stellst ein, wie schnell die Sekunden ticken (Prescaler) und bei welcher Zahl der Wecker klingelt (Auto-Reload). Wenn der Wecker klingelt, passiert etwas (z.B. LED umschalten) — das ist der Interrupt.

### Die Zählkette — Vom Takt zum Ereignis

Der Mikrocontroller hat einen internen Takt von **84 MHz** (84 Millionen Takte pro Sekunde). Das ist viel zu schnell, um direkt damit zu arbeiten. Deshalb wird der Takt schrittweise verlangsamt:

```
84 MHz Takt → [Prescaler: verlangsamt] → Zähltakt → [Counter: zählt] → Überlauf → Ereignis!
```

**Zählfrequenz** (wie schnell der Zähler tickt):

$$f_{CNT} = \frac{84\,\text{MHz}}{PSC + 1}$$

**Periodendauer** (Zeit bis zum Überlauf):

$$T_{period} = \frac{(PSC + 1) \cdot (ARR + 1)}{84\,\text{MHz}}$$

### Prescaler (PSC) — Der Verlangsamer

Der Prescaler **teilt** den schnellen Eingangstakt herunter. Er lässt nur jeden x-ten Takt durch.

**Wichtige Eigenheit:** Im Register steht immer der Teilerwert **minus 1**:
- `PSC = 0` → teilt durch 1 (kein Teilen, volle Geschwindigkeit)
- `PSC = 1` → teilt durch 2 (halb so schnell)
- `PSC = 8399` → teilt durch 8400

**Warum minus 1?** So kann man mit dem Wert 0 im Register trotzdem "durch 1 teilen" darstellen. Das ist eine gängige Konvention in der Hardware-Welt.

### ARR (Auto-Reload Register) — Das Zählziel

Der ARR bestimmt, **bis wohin** der Zähler zählt, bevor er von vorne beginnt. Auch hier gilt: der Zähler durchläuft `ARR + 1` Schritte (weil er bei 0 anfängt).

**Beispiel:** ARR = 9999 bedeutet: Der Zähler zählt 10'000 Schritte (0 bis 9999), dann startet er neu.

### Timer als Wecker — TIM4 und Interrupts

Ein **Interrupt** ist wie ein Wecker: Wenn der Timer überläuft (alle Schritte durchgezählt hat), unterbricht er das Hauptprogramm und führt eine spezielle Funktion aus — die **ISR** (Interrupt Service Routine). Danach macht das Hauptprogramm weiter, wo es aufgehört hat.

**Konfigurationsschritte (in dieser Reihenfolge):**

1. **Clock einschalten** (`RCC->APB1ENR`) — Den Timer mit Strom versorgen
2. **Prescaler setzen** (`TIMx->PSC`) — Wie schnell soll gezählt werden?
3. **Zählziel setzen** (`TIMx->ARR`) — Bis wohin soll gezählt werden?
4. **Zählrichtung wählen** (`TIMx->CR1`, Bit DIR) — Hoch- oder runterzählen?
5. **Interrupt aktivieren** (`TIMx->DIER`, Bit UIE) — Soll bei Überlauf ein Alarm ausgelöst werden?
6. **Timer starten** (`TIMx->CR1`, Bit CEN) — **Immer zuletzt!** Erst alles einstellen, dann starten
7. **Interrupt im NVIC freischalten** (`NVIC->ISER0`) — Die zentrale Interrupt-Verwaltung muss auch Bescheid wissen

**Wichtig — UIF manuell löschen:** Wenn der Timer überläuft, setzt die Hardware ein Flag (UIF = Update Interrupt Flag). Dieses Flag muss man in der ISR **selbst zurücksetzen**. Vergisst man das, denkt der Controller "der Interrupt ist ja noch da" und ruft die ISR sofort nochmal auf — eine Endlosschleife.

```c
TIM4->SR &= ~(0x00000001);  // UIF-Flag löschen
```

### PWM — Wie man Helligkeit steuert

**PWM (Pulse Width Modulation)** ist ein Trick: Man schaltet eine LED sehr schnell an und aus. Das menschliche Auge sieht das nicht als Blinken, sondern nimmt eine **reduzierte Helligkeit** wahr. Je länger die LED pro Zyklus an ist, desto heller erscheint sie.

Der **Duty Cycle** (Tastgrad) gibt an, wie viel Prozent der Zeit das Signal HIGH (an) ist:

```
     ┌──────────┐                    ┌──────────┐
     |          |                    |          |
─────┘          └────────────────────┘          └────
     |← AN  →|←      AUS        →|
     |←      eine Periode        →|

     Duty Cycle = AN-Zeit / Gesamtperiode
```

**Beispiele:**
- 0% Duty Cycle → LED immer aus
- 50% Duty Cycle → LED halb so hell
- 100% Duty Cycle → LED voll an

### PWM Mode 1 — Wie der Timer das Signal erzeugt

Im **PWM Mode 1** vergleicht der Timer seinen aktuellen Zählerstand (CNT) mit dem Wert im Compare-Register (CCRx):

- Solange `CNT < CCRx` → Ausgang HIGH (LED an)
- Sobald `CNT >= CCRx` → Ausgang LOW (LED aus)

**Daraus folgt:**
- `CCRx = 0` → 0% Duty Cycle (sofort LOW, nie HIGH)
- `CCRx = ARR + 1` → 100% Duty Cycle (immer HIGH, nie LOW)
- `CCRx = (ARR + 1) / 2` → 50% Duty Cycle

**Je grösser der CCR-Wert, desto heller die LED.**

### Output Compare Mode — Verschiedene Betriebsarten

Jeder Timer-Kanal kann in verschiedenen Modi betrieben werden. Das wird über die Bits OCxM im CCMR-Register eingestellt:

| OCxM[2:0] | Modus | Was passiert |
|-----------|-------|-------------|
| `000` | Frozen | Ausgang wird nicht beeinflusst |
| `001` | Active on match | HIGH wenn Zähler = Vergleichswert |
| `010` | Inactive on match | LOW wenn Zähler = Vergleichswert |
| `011` | Toggle | Umschalten wenn Zähler = Vergleichswert |
| `110` | **PWM Mode 1** | HIGH solange Zähler < Vergleichswert |
| `111` | PWM Mode 2 | Invertiert zu Mode 1 |

Wir verwenden **Mode 1 (`110`)** für die PWM-Erzeugung.

### Die wichtigsten Register auf einen Blick

| Register | Was es tut | Analogie |
|----------|-----------|----------|
| `CR1` | Steuerung: Starten, Richtung | Ein/Aus-Schalter und Richtungshebel |
| `PSC` | Prescaler: Taktteiler | Gangschaltung (langsamer/schneller) |
| `ARR` | Auto-Reload: Zählziel | Wecker-Einstellung (wann klingeln?) |
| `DIER` | Interrupt ein/aus | Wecker-Ton an oder stumm |
| `SR` | Status-Flags (z.B. UIF) | Anzeige "Wecker hat geklingelt" |
| `CCMR1/2` | Modus der Ausgangskanäle | Betriebsmodus (PWM, Toggle, etc.) |
| `CCER` | Ausgänge ein/aus | Schalter für jeden einzelnen Ausgang |
| `CCR1–4` | Compare-Werte (Duty Cycle) | Helligkeitsregler pro Farbe |
| `CNT` | Aktueller Zählerstand | Aktuelle Position der Stoppuhr |

### CR1-Register im Detail

```
Bit: 15-10  9:8   7    6:5  4    3    2    1    0
     Res.  CKD  ARPE  CMS  DIR  OPM  URS  UDIS CEN
```

Für dieses Lab sind nur zwei Bits relevant:

| Bit | Name | Was es tut |
|-----|------|-----------|
| 4 | DIR | Zählrichtung: 0 = hoch (TIM3), 1 = runter (TIM4) |
| 0 | CEN | Counter Enable: 1 = Timer läuft. **Immer zuletzt setzen!** |

### CCMR1/2 — Kanal-Modus einstellen

Diese Register steuern je **zwei Kanäle** gleichzeitig:

- **CCMR1** kontrolliert Kanal 1 (Bits 6:4 = OC1M) und Kanal 2 (Bits 14:12 = OC2M)
- **CCMR2** kontrolliert Kanal 3 (Bits 6:4 = OC3M) und Kanal 4 (Bits 14:12 = OC4M)

Für PWM Mode 1 (`110`) auf allen drei Kanälen:
- `CCMR1 = 0x6060` → OC1M = 110, OC2M = 110 (Kanal 1 + 2 auf PWM Mode 1)
- `CCMR2 = 0x0060` → OC3M = 110 (Kanal 3 auf PWM Mode 1)

### CCER — Ausgänge aktivieren

Jeder Kanal hat ein Enable-Bit. Erst wenn dieses gesetzt ist, wird das PWM-Signal tatsächlich am Pin ausgegeben:

| Bit | Name | Funktion |
|-----|------|----------|
| 0 | CC1E | Kanal 1 Ausgang ein |
| 4 | CC2E | Kanal 2 Ausgang ein |
| 8 | CC3E | Kanal 3 Ausgang ein |
| 12 | CC4E | Kanal 4 Ausgang ein |

`CCER = 0x0111` schaltet Kanal 1, 2 und 3 ein.

### DIER — Interrupt einschalten

| Bit | Name | Funktion |
|-----|------|----------|
| 0 | UIE | Update Interrupt Enable — Interrupt bei Über-/Unterlauf |
| 1–4 | CC1IE–CC4IE | Interrupt wenn Compare-Wert erreicht (hier nicht verwendet) |

## Konkrete Berechnungen

### TIM4 — 1-Sekunden-Interrupt

**Ziel:** Alle 1 Sekunde soll ein Interrupt ausgelöst werden.

**Schritt 1 — Prescaler wählen:** Wir wollen 100 µs pro Zählschritt (= 10 kHz Zähltakt):

$$PSC = \frac{84\,\text{MHz}}{10\,\text{kHz}} - 1 = 8400 - 1 = 8399$$

**Schritt 2 — Zählziel wählen:** Für 1 Sekunde brauchen wir 10'000 Schritte à 100 µs:

$$ARR = 10{,}000 - 1 = 9999$$

**Kontrolle:** $\frac{8400 \times 10{,}000}{84{,}000{,}000} = 1{,}0\,\text{s}$

### TIM3 — 200 Hz PWM

**Ziel:** PWM-Signal mit 200 Hz (5 ms Periode), möglichst hohe Auflösung.

**Schritt 1 — Gesamtteiler berechnen:**

$$(PSC + 1) \times (ARR + 1) = \frac{84\,\text{MHz}}{200\,\text{Hz}} = 420{,}000$$

**Schritt 2 — PSC und ARR aufteilen:** Wir wollen ARR möglichst gross (= hohe Auflösung bei der Helligkeitsstufe), aber es muss in 16 Bit passen (max. 65'535):

| PSC | Teiler | ARR + 1 | ARR | Passt in 16 Bit? |
|-----|--------|---------|-----|-----------------|
| 5 | 6 | 70'000 | 69'999 | Nein (> 65'535) |
| **6** | **7** | **60'000** | **59'999** | **Ja** |
| 7 | 8 | 52'500 | 52'499 | Ja (aber weniger Auflösung) |

**Gewählt:** PSC = 6, ARR = 59'999

**Kontrolle:** $\frac{84\,\text{MHz}}{7 \times 60{,}000} = 200\,\text{Hz}$

### Duty-Cycle-Skalierung — Von 4-Bit-Schaltern zu Helligkeit

Die DIP-Switches liefern 4-Bit-Werte (0x0 bis 0xF, also 0 bis 15). Diese müssen auf den Bereich des Compare-Registers abgebildet werden:

- Schalterwert 0x0 → 0% Duty Cycle → CCRx = 0
- Schalterwert 0xF → 100% Duty Cycle → CCRx = ARR + 1 = 60'000

Dazwischen liegen **15 gleich grosse Schritte**:

$$\text{Schrittweite} = \frac{60{,}000}{15} = 4{,}000$$

**Formel:** `CCRx = Schalterwert * 4000`

| Schalterwert | Rechnung | CCRx | Duty Cycle |
|:------------:|----------|-----:|-----------:|
| 0x0 (0) | 0 × 4000 | 0 | 0% |
| 0x1 (1) | 1 × 4000 | 4'000 | 6.7% |
| 0x3 (3) | 3 × 4000 | 12'000 | 20% |
| 0x8 (8) | 8 × 4000 | 32'000 | 53.3% |
| 0xF (15) | 15 × 4000 | 60'000 | 100% |

**Warum 15 und nicht 16?** Weil 0xF = 15 der höchste 4-Bit-Wert ist. Wir haben 16 Stufen (0 bis 15), aber 15 **Schritte** dazwischen. Würden wir durch 16 teilen, käme Schalterwert 0xF nur auf 93.75% statt auf volle 100%.

### NVIC — Die zentrale Interrupt-Verwaltung

Der **NVIC** (Nested Vectored Interrupt Controller) ist die Zentrale, die alle Interrupts im Cortex-M4 verwaltet. Jede Interrupt-Quelle hat eine feste **IRQ-Nummer**. Um einen Interrupt zu nutzen, muss er im NVIC freigeschaltet werden:

```c
NVIC->ISER0 |= (1 << 30);  // TIM4 hat IRQ-Nummer 30
```

**Wichtig:** Die ISR muss **exakt den richtigen Namen** haben (z.B. `TIM4_IRQHandler`). Dieser Name ist im Startup-File festgelegt. Schreibt man ihn falsch, findet der Controller die Funktion nicht und springt stattdessen in den Default Handler — das Programm hängt.

### GPIO Alternate Function — Timer-Signale an Pins ausgeben

Die PWM-Signale des Timers müssen physisch an Pins ausgegeben werden. Dazu werden die GPIO-Pins in den **Alternate Function Modus** geschaltet. Jeder Pin kann verschiedene Funktionen haben — für TIM3 braucht man **AF2**:

| Timer-Kanal | GPIO-Pin | CT-Board Port | Farbe |
|-------------|----------|---------------|-------|
| TIM3_CH1 | PB4 | P6.4 | Rot |
| TIM3_CH2 | PB5 | P6.5 | Grün |
| TIM3_CH3 | PB0 | P6.0 | Blau |

Die GPIO-Konfiguration war im Projektrahmen bereits vorgegeben.

## Schlussfolgerungen

- **PSC + 1 ist der tatsächliche Teiler**: Im Register steht immer eins weniger, als man eigentlich teilen will. PSC = 0 teilt nicht, PSC = 8399 teilt durch 8400. Gleiches gilt für ARR.
- **CEN zuletzt setzen**: Erst alles konfigurieren, dann den Timer starten. Sonst läuft der Timer kurzzeitig mit falschen Einstellungen.
- **UIF manuell löschen**: Die Hardware setzt das Flag, löscht es aber nicht. Vergisst man das Löschen in der ISR → Endlosschleife, weil der Interrupt sofort wieder ausgelöst wird.
- **PWM Mode 1**: Je grösser der CCR-Wert, desto grösser der Duty Cycle, desto heller die LED.
- **Skalierungswert 4000 ist kein Zufall**: Die Wahl von PSC = 6 und ARR = 59999 ergibt ARR + 1 = 60000, das glatt durch 15 teilbar ist. Das ermöglicht eine saubere ganzzahlige Skalierung ohne Rundungsfehler.
- **15 Schritte, nicht 16**: 4-Bit-Werte gehen von 0 bis 15. Das sind 16 Stufen, aber nur 15 Abstände. Teilt man durch 15, landet man exakt bei 0% und 100%.

## Verwandte Themen

- **Woche 3 (GPIO):** GPIO Alternate Function Mode — die PWM-Pins werden als AF2 für TIM3 konfiguriert
- **Woche 4/5 (SPI):** Polling vs. Interrupt — SPI nutzt Polling (busy waiting), Timer nutzt Interrupts (CPU kann anderes tun)
- **Woche 7 (Interrupts):** Vertiefung des NVIC und der Interrupt-Priorisierung
