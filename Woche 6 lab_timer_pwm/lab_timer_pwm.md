# CT2 Praktikum: Timer und PWM

## 1. Einleitung

In diesem Praktikum werden die 16-Bit Timer TIM3 und TIM4 des STM32F429ZI eingesetzt, um dreifarbige RGB-LEDs auf einem Zusatzboard anzusteuern. TIM4 erzeugt im Sekundentakt einen Interrupt, TIM3 produziert drei synchrone PWM-Signale für die Farben Rot, Grün und Blau. Über DIP Switches werden die Duty Cycles der einzelnen Farben eingestellt.

## 2. Lernziele

- Periodisches Interrupt mit Timer erzeugen und ISR implementieren
- Synchrone PWM-Signale mit unterschiedlichen Duty Cycles erzeugen
- Funktion von Reload- und Compare-Registern erklären

## 3. Programmrahmen

```
main.c     →  Hauptprogramm: Init, Endlosschleife, ISR
timer.c/h  →  Timer-Modul: tim3_init(), tim4_init(), tim3_set_compare_register()
```

**Vorgegeben:** GPIO-Konfiguration für PWM-Pins (PB0, PB4, PB5 als AF2), Peripheral Clocks, einige Timer-Register-Defaults, NVIC-Freischaltung.

**Zu implementieren:** Timer-Konfiguration (PSC, ARR, CR1, DIER, CCMR, CCER), ISR, Main-Loop.

## 4. Aufgabe 1: TIM4 — Interrupt im Sekundentakt

### Prescaler- und Reload-Berechnung

- CK_INT = 84 MHz
- Geforderte Zählschritte: 100 µs (10 kHz)
- Geforderte Periode: 1 s

**Prescaler:**
```
f_CNT = 84 MHz / (PSC + 1) = 10 kHz
PSC + 1 = 8400
PSC = 8399
```

**Auto-Reload (Downcounter):**
```
T = (ARR + 1) / f_CNT = 1 s
ARR + 1 = 10000
ARR = 9999
```

### Lösung: `tim4_init()` (timer.c)

```c
// Prescaler: 84 MHz / 8400 = 10 kHz (100 µs Schritte)
TIM4->PSC = 8399;

// Auto-Reload: 10000 Schritte = 1 Sekunde
TIM4->ARR = 9999;

// CR1: Default 0x0000, zusätzlich DIR = 1 (Downcounter)
TIM4->CR1 = 0x0010;

// DIER: UIE = 1 (Update Interrupt Enable), alles andere 0
TIM4->DIER = 0x0001;

// Timer starten: CEN = 1
TIM4->CR1 |= 0x0001;
```

### Lösung: `TIM4_IRQHandler()` (main.c)

```c
void TIM4_IRQHandler(void)
{
    static uint8_t led_state = 0;

    // UIF löschen
    tim4_reset_uif();

    // LEDs toggeln
    if (led_state == 0) {
        CT_LED->BYTE.LED31_24 = 0xFF;
        led_state = 1;
    } else {
        CT_LED->BYTE.LED31_24 = 0x00;
        led_state = 0;
    }

    // Zyklischen 4-Bit-Zähler inkrementieren (für Aufgabe 3)
    cycle_counter_4bit = (cycle_counter_4bit + 1) & 0x0F;
}
```

---

## 5. Aufgabe 2: TIM3 — PWM für RGB-LEDs

### Prescaler- und Reload-Berechnung

- Geforderte PWM-Frequenz: 200 Hz (5 ms Periode)
- Maximale Ausnutzung des 16-Bit-Zählbereiches

```
(PSC + 1) * (ARR + 1) = 84 MHz / 200 Hz = 420000

Wahl: PSC = 6 (Teiler 7), ARR = 59999
Kontrolle: 7 * 60000 = 420000 ✓
f_PWM = 84 MHz / 420000 = 200 Hz ✓
```

### Inline-Fragen und Antworten

**Welcher Wert muss für einen Duty Cycle von 100% ins Compare Register geladen werden?**
> CCRx = ARR + 1 = 60000. Im PWM Mode 1 (Upcounting) ist der Output HIGH solange CNT < CCRx. Da CNT von 0 bis 59999 zählt, ist bei CCRx = 60000 die Bedingung immer erfüllt → 100% Duty Cycle.

**Welcher Wert muss für einen Duty Cycle von 0% ins Compare Register geladen werden?**
> CCRx = 0. CNT ist nie kleiner als 0, daher ist der Output immer LOW → 0% Duty Cycle.

**Wie gross ist die Schrittweite (Skalierungswert)?**
> Schrittweite = 60000 / 15 = 4000. Die 4-Bit-Werte (0–15) werden mit 4000 multipliziert: `CCRx = switch_value * 4000`.

### DIP-Switch-Zuordnung

| Switches | Kanal | Pin | Farbe |
|----------|-------|-----|-------|
| S3..S0 | CH1 | PB4 (P6.4) | Rot |
| S11..S8 | CH2 | PB5 (P6.5) | Grün |
| S19..S16 | CH3 | PB0 (P6.0) | Blau |

### Lösung: `tim3_init()` (timer.c)

```c
// CR1: Upcounter (DIR = 0), alle anderen Bits 0
TIM3->CR1 = 0x0000;

// Prescaler: 84 MHz / 7 = 12 MHz
TIM3->PSC = 6;

// Auto-Reload: 12 MHz / 60000 = 200 Hz
TIM3->ARR = 59999;

// CCMR1: OC1M = 110 (PWM Mode 1) für CH1, OC2M = 110 für CH2
TIM3->CCMR1 = 0x6060;

// CCMR2: OC3M = 110 (PWM Mode 1) für CH3
TIM3->CCMR2 = 0x0060;

// CCER: CC1E, CC2E, CC3E aktivieren (Bit 0, 4, 8)
TIM3->CCER = 0x0111;

// Timer starten: CEN = 1
TIM3->CR1 |= 0x0001;
```

### Lösung: `tim3_set_compare_register()` (timer.c)

```c
void tim3_set_compare_register(pwm_channel_t channel, uint16_t value)
{
    switch (channel) {
        case PWM_CH1:
            TIM3->CCR1 = value;
            break;
        case PWM_CH2:
            TIM3->CCR2 = value;
            break;
        case PWM_CH3:
            TIM3->CCR3 = value;
            break;
        case PWM_CH4:
            TIM3->CCR4 = value;
            break;
    }
}
```

### Makros (main.c / timer.c)

```c
// Skalierungsfaktor für 4-Bit-Werte → CCR-Werte
#define STEP_SIZE        4000
#define MASK_4BIT        0x0F
```

---

## 6. Aufgabe 3: Periodisches Wechseln von Farben

### Funktionsweise

- Über S31 wird zwischen Aufgabe 2 (manuell) und Aufgabe 3 (automatisch) umgeschaltet
- Die ISR inkrementiert `cycle_counter_4bit` (0x0–0xF) im Sekundentakt
- Im Hauptprogramm:
  - CH1 (Rot): Duty Cycle = Zählerwert (zählt hoch)
  - CH2 (Grün): Duty Cycle = 0xF - Zählerwert (zählt runter)
  - CH3 (Blau): weiterhin über S19..S16

### Lösung: `main()` (main.c)

```c
int main(void)
{
    uint16_t red, green, blue;

    tim4_init();
    tim3_init();

    while (1) {
        if (CT_DIPSW->BYTE.S31_24 & 0x80) {
            // Aufgabe 3: Automatischer Farbwechsel
            red   = cycle_counter_4bit * STEP_SIZE;
            green = (0x0F - cycle_counter_4bit) * STEP_SIZE;
            blue  = (CT_DIPSW->BYTE.S23_16 & MASK_4BIT) * STEP_SIZE;
        } else {
            // Aufgabe 2: Manuelle Farbeinstellung
            red   = (CT_DIPSW->BYTE.S7_0   & MASK_4BIT) * STEP_SIZE;
            green = (CT_DIPSW->BYTE.S15_8  & MASK_4BIT) * STEP_SIZE;
            blue  = (CT_DIPSW->BYTE.S23_16 & MASK_4BIT) * STEP_SIZE;
        }

        tim3_set_compare_register(PWM_CH1, red);
        tim3_set_compare_register(PWM_CH2, green);
        tim3_set_compare_register(PWM_CH3, blue);
    }
}
```

---

## 7. Bewertung

| Bewertungskriterien | Gewichtung |
|---------------------|-----------|
| TIM4 funktioniert wie gefordert (1s Interrupt, LEDs blinken) | 1/2 |
| RGB-Wert setzen funktioniert, Logic-Analyzer-Bild wird präsentiert | 1/4 |
| Periodisches Wechseln der Farben implementiert | 1/4 |

## 8. Schlussfolgerungen

- **PSC und ARR bestimmen die Periodendauer gemeinsam**: Die Formel `T = (PSC+1) * (ARR+1) / f_CK_INT` verknüpft beide Werte. Für 200 Hz PWM gibt es mehrere gültige Kombinationen — die optimale nutzt den Zählbereich maximal aus (ARR = 59999 bei PSC = 6).
- **UIF in der ISR löschen ist Pflicht**: Ohne Löschen wird die ISR endlos aufgerufen, da das Interrupt-Flag gesetzt bleibt. Die Funktion `tim4_reset_uif()` kapselt dies sauber.
- **PWM Mode 1 im Upcounter**: Output HIGH wenn CNT < CCRx. Grösserer CCR-Wert = grösserer Duty Cycle. CCR = 0 → 0%, CCR > ARR → 100%.
- **Ganzzahlige Skalierung**: Die Wahl von ARR = 59999 (Bereich 60000) ermöglicht eine glatte Schrittweite von 4000 für die 15 Stufen der 4-Bit-DIP-Switches. Andere ARR-Werte würden Rundungsfehler erzeugen.
- **ISR-Variablen müssen `static` oder global sein**: Der `cycle_counter_4bit` wird in der ISR geschrieben und im Main-Loop gelesen. Da die ISR asynchron aufgerufen wird, muss die Variable über Funktionsaufrufe hinweg persistieren.
- **Separates Timer-Modul**: Die Trennung in `main.c` (Ablaufsteuerung) und `timer.c` (Hardware-Zugriffe) folgt dem Schichtenprinzip und ermöglicht Wiederverwendung der Timer-Funktionen.
