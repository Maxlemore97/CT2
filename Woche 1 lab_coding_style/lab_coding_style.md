# P01 Coding Style Structure

## 1. Einleitung

In diesem Praktikum realisieren wir einen elektronischen Würfel auf dem CT-Board. Das C-Programm wird in verschiedene Module mit unterschiedlichen Aufgaben gegliedert.

## 2. Lernziele

- Vorteile eines modularen Source Codes erklären können
- Die vier vorgestellten Leitlinien zur Strukturierung von Programmen kennen
- Eigene C-Programme in Module gliedern können
- Wissen, wie ein Modul in C auf Header File (`.h`) und C-File (`.c`) aufgeteilt wird
- Struktur eines Gesamtprogramms und Abhängigkeiten zwischen Modulen grafisch darstellen können

## 3. Von einem „All-In-One" Source File zu modularem Source Code

### 3.1. Ausgangslage

Das erste Programm ist oft ein einzelnes Source Code File mit allem drin. Dies ist angebracht für einfachste Programme. Sobald Programme komplexer werden, leidet die Übersichtlichkeit. Fehlersuche und Anpassungen werden schwierig.

### 3.2. Zielsetzung

Source Code auf mehrere praktisch handhabbare Teile aufteilen. Dies ermöglicht die Komplexität zu reduzieren und erlaubt es, in sich abgeschlossene Komponenten zu entwickeln, die mehrfach verwendet werden können.

### 3.3. Umsetzung in C

In C wird die **Schnittstelle (Interface)** eines Moduls im Header File (`.h`) codiert. Die **Implementation** wird im `.c` File codiert.

#### 3.3.1. Header File — Interface

Das Header File beschreibt, **was** das Modul kann. Es enthält **ausschliesslich** Informationen, die für einen Anwender des Moduls notwendig sind. Es enthält keine Informationen über die innere Struktur des Moduls (**Information Hiding**).

**Fragen zum `statistics` Header File:**

- **Welche Funktionen werden angeboten?**
  > `statistics_add_throw`, `statistics_read`

- **Welche Parameter werden übergeben?**
  > `throw_value` und `dice_number` vom Typ `uint8_t`

- **Welche Datentypen müssen zwischen Anwender und Modul bekannt sein?**
  > Eingabetyp der Methoden und Return-Wert (`uint8_t`)

- **Welche Macros (`#defines`) werden sowohl durch den Anwender als auch durch das Modul verwendet?**
  > `ERROR_VALUE`

**Regeln für Header Files:**
- Enthält Deklarationen (Prototypen) der öffentlichen Funktionen
- Keine Deklarationen modulinterner Funktionen
- Keine Funktionsdefinitionen
- Keine Variablendefinitionen (kein Speicher alloziert)

> **Info:** Kolleg:innen sollten das Modul allein mit den Informationen im Header File einsetzen können. Ein Blick ins C-File sollte nicht nötig sein.

**Rahmen für ein Header File** (`my_module.h`):

```c
/* re-definition guard */
#ifndef _MY_MODULE_H_
#define _MY_MODULE_H_

/* includes, but only if required to compile this header file */
#include <stdint.h>

/* module declarations with decent descriptions */
...

#endif
```

#### 3.3.2. C-File — Implementation

Im C-File werden die Header Files aller verwendeten Module über `#include` eingebunden. Das eigene Header File wird immer ebenfalls eingebunden.

**Rahmen für ein C-File** (`my_module.c`):

```c
/* standard includes */
#include <...>        // standard includes go in <...>

/* user includes */
#include "..."        // user header files go in "..."
#include "my_module.h" // include your own interface

/* variables visible within the whole module */
...

/* function definitions */
...
```

Der Mechanismus wie die aus den einzelnen C-Files generierten Object Files zu einem ausführbaren Executable zusammengefasst werden, wurde in CT1 im Kapitel „Modulare Codierung / Linking" erklärt.

## 4. Anwendung

Die Anwendung realisiert einen **elektronischen Würfel**:
- Bei jedem Druck der Taste **T0** wird eine Zufallszahl ($1 \ldots 6$) auf der **7-Segmentanzeige** angezeigt
- Die totale Anzahl Würfe und die Anzahl pro Wert ($1 \ldots 6$) werden auf dem **LCD** als zweistellige Dezimalzahl angezeigt

*Abbildung 1: CT-Board mit 7-Segment (gewürfelter Wert), LCD Zeile 1 (Statistik pro Wert), LCD Zeile 2 (Total Würfe), Taste T0 (Auslöser).*

## 5. Aufgaben

> **Hinweis:** Im vorgegebenen Programmrahmen sind sämtliche Header Files plus einige Implementationen gegeben. Da verschiedene Implementationen fehlen, gibt es beim Build zunächst Compiler-Warnungen (unbenutzte Variablen) und Linker-Fehler (fehlende Object Codes).

### 5.1. Einarbeitung — UML-Diagramm

**Aufgabe:** Programmrahmen verstehen, ausgehend von `main()`. Alle Module mit öffentlichen und privaten Funktionen in einem UML-ähnlichen Diagramm darstellen. Abhängigkeiten mit `uses`-Pfeilen zeigen.

**Lösung:** Siehe `uml_diagram.puml` — vollständiges UML-Diagramm der Modulstruktur:

```
┌─────────────────────────────────────────────┐
│                    main                      │
│  + main() : int                              │
└──┬──────────┬──────────┬──────────┬─────────┘
   │uses      │uses      │uses      │uses
   ▼          ▼          ▼          ▼
┌────────┐ ┌────────┐ ┌──────────┐ ┌────────────┐
│counter │ │display │ │statistics│ │button_state│
├────────┤ ├────────┤ ├──────────┤ ├────────────┤
│-dice_  │ │-LCD_   │ │-nr_of_  │ │-last_      │
│counter │ │ADDR_.. │ │throws[] │ │button_state│
├────────┤ ├────────┤ ├──────────┤ ├────────────┤
│+counter│ │+display│ │+stats_  │ │+button_    │
│_incre- │ │_clear()│ │add_     │ │state_eval()│
│ment()  │ │+display│ │throw()  │ └─────┬──────┘
│+counter│ │_write_ │ │+stats_  │       │uses
│_read() │ │throw() │ │read()   │       ▼
└───┬────┘ │+display│ └────┬────┘  ┌─────────┐
    │      │_write_ │      │       │reg_     │
    │      │value() │      │       │ctboard  │
    │      │+display│      │       │CT_BUTTON│
    │      │_write_ │      │       └─────────┘
    │      │total() │      │
    │      └──┬──┬──┘      │
    │uses     │  │uses     │uses
    ▼         ▼  ▼         ▼
┌──────┐ ┌────────────┐ ┌──────┐
│dice  │ │hal_ct_seg7 │ │dice  │
│NR_OF_│ │reg_ctboard │ │      │
│DICE_ │ │CT_LCD      │ │      │
│VALUES│ └────────────┘ └──────┘
└──────┘
```

**Module und ihre Rollen:**

| Modul | Public Functions | Private Variables | Abhängigkeiten |
|-------|-----------------|-------------------|----------------|
| `main` | `main()` | — | counter, display, statistics, button_state, dice |
| `counter` | `counter_increment()`, `counter_read()` | `dice_counter` | dice |
| `statistics` | `statistics_add_throw()`, `statistics_read()` | `nr_of_throws[]` | dice |
| `display` | `display_clear()`, `display_write_throw()`, `display_write_value()`, `display_write_total()` | Macros: `LCD_ADDR_LINE1/2`, `NR_OF_CHAR_PER_LINE` | hal_ct_seg7, reg_ctboard |
| `button_state` | `button_state_eval()` | `last_button_state` | reg_ctboard |
| `dice` | — (nur Konstante) | `NR_OF_DICE_VALUES = 6` | — |

---

### 5.2. Würfel — Modul `counter`

**Aufgabe:** Modul `counter` implementieren. Die Variable `dice_counter` ist als `static` vorgegeben (Sichtbarkeit auf Modul eingeschränkt). Für den Test zuerst statistics/display auskommentieren.

**Lösung** (`counter.c`):

```c
static uint8_t dice_counter = 1;

uint8_t counter_read(void) {
    return rand() % 6 + 1;
}

void counter_increment(void) {
    ++dice_counter;
}
```

> **Erklärung:** `counter_read()` nutzt `rand()` aus `<stdlib.h>` für eine Pseudo-Zufallszahl im Bereich $[1, 6]$. Der Modulo-Operator `% 6` ergibt Werte $0 \ldots 5$, `+ 1` verschiebt auf $1 \ldots 6$. Die Funktion `counter_increment()` erhöht den internen Zähler — dieser dient als Seed-ähnlicher Mechanismus, da `main()` ihn in jeder Schleifeniteration aufruft.

---

### 5.3. Statistik — Modul `statistics`

**Aufgabe:** Modul `statistics` implementieren. Der Array `nr_of_throws[]` ist vorgegeben.

**Lösung** (`statistics.c`):

```c
static uint8_t nr_of_throws[NR_OF_DICE_VALUES + 1] = {0};
// Index 0:       totale Anzahl Würfe
// Index 1 bis 6: Anzahl Würfe pro Augenzahl

void statistics_add_throw(uint8_t throw_value) {
    nr_of_throws[0]++;           // Total erhöhen
    nr_of_throws[throw_value]++; // Zähler für diesen Wert erhöhen
}

uint8_t statistics_read(uint8_t dice_number) {
    if (dice_number > NR_OF_DICE_VALUES) {
        return ERROR_VALUE;      // 0xFF bei ungültigem Index
    }
    return nr_of_throws[dice_number];
}
```

> **Erklärung:** Der Array hat 7 Elemente (Index $0 \ldots 6$). Index 0 speichert die Gesamtzahl, Index $1 \ldots 6$ die Anzahl pro Würfelwert. `statistics_read(0)` gibt das Total zurück. Bei ungültigem Index wird `ERROR_VALUE` (`0xFF` = 255) zurückgegeben.

---

### 5.4. Display — Modul `display`

**Aufgabe:** Modul `display` implementieren. Zugriff über die ASCII-Schnittstelle des CT-Board LCD via `CT_LCD->ASCII[]`.

**Hinweise aus dem Lab-Sheet:**
- Zeichen schreiben: `CT_LCD->ASCII[position] = 'zeichen';`
- String schreiben: Zeichenweise in einer Schleife
- `snprintf()` aus `<stdio.h>` für formatierte Ausgabe in einen Buffer
- Display löschen: Leerzeichen schreiben
- Hintergrundfarbe: `CT_LCD->BG.GREEN = 65535;` für maximales Grün

**Lösung** (`display.c`):

```c
#define LCD_ADDR_LINE1      0u
#define LCD_ADDR_LINE2      20u
#define NR_OF_CHAR_PER_LINE 20u
#define LCD_CLEAR           "                    "

void display_write_throw(uint8_t dice_number) {
    hal_ct_seg7_bin_write(dice_number);
}

void display_write_value(uint8_t slot_nr, uint8_t value) {
    uint8_t start;
    if (slot_nr == 1) start = 0;
    if (slot_nr == 2) start = 3;
    if (slot_nr == 3) start = 6;
    if (slot_nr == 4) start = 9;
    if (slot_nr == 5) start = 12;
    if (slot_nr == 6) start = 15;

    char first = ' ';
    char second = '0';
    if (value >= 10)
        first = value / 10 + '0';
    second += value % 10;

    CT_LCD->ASCII[start] = first;
    CT_LCD->ASCII[start + 1] = second;
}

void display_clear(void) {
    for (uint8_t i = 0; i <= NR_OF_CHAR_PER_LINE; i++) {
        CT_LCD->ASCII[i + LCD_ADDR_LINE1] = ' ';
    }
    for (uint8_t i = 0; i <= NR_OF_CHAR_PER_LINE; i++) {
        CT_LCD->ASCII[i + LCD_ADDR_LINE2] = ' ';
    }
    CT_LCD->BG.GREEN = 65535u;  // Maximale Grünintensität
}

void display_write_total(uint8_t total_value) {
    char buffer[NR_OF_CHAR_PER_LINE] = {' '};
    snprintf(buffer, sizeof(buffer), "total throws %u", total_value);
    for (uint8_t i = 0; i <= NR_OF_CHAR_PER_LINE; i++) {
        CT_LCD->ASCII[LCD_ADDR_LINE2 + i] = buffer[i];
    }
}
```

> **Erklärung:**
> - `display_write_value()`: Jeder Slot belegt 3 Zeichen (2 Ziffern + 1 Leerzeichen Abstand). Die zweistellige Zahl wird manuell in ASCII umgewandelt: `value / 10 + '0'` für die Zehnerstelle, `value % 10 + '0'` für die Einerstelle. Bei Werten $< 10$ wird die Zehnerstelle als Leerzeichen dargestellt.
> - `display_clear()`: Überschreibt beide LCD-Zeilen mit Leerzeichen und setzt den Hintergrund auf Grün.
> - `display_write_total()`: Nutzt `snprintf()` für formatierte Ausgabe und kopiert den Buffer zeichenweise ins LCD.

---

### 5.5. Test

**Aufgabe:** Gesamte Anwendung mit originaler `main()` testen. Produziert der Würfel eine gleichmässige Verteilung?

**Antwort:** Da `counter_read()` die Standard-`rand()`-Funktion verwendet, ist die Verteilung pseudo-zufällig und bei genügend Würfen annähernd gleichmässig ($\approx \frac{1}{6}$ pro Wert).

---

### 5.6. Bewertung

| Bewertung | Gewichtung |
|-----------|-----------|
| Darstellung der Programmstruktur in UML-ähnlicher Form | 1/4 |
| Programm erfüllt geforderte Funktionalität (Würfel, Statistik, LCD, Test) | 3/4 |
