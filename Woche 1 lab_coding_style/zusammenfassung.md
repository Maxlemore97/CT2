# Woche 1 — Coding Style & Modulare Struktur

## Überblick

In dieser Woche wurde ein elektronischer Würfel auf dem CT-Board implementiert. Der Fokus lag auf der modularen Strukturierung von C-Programmen: Aufteilung in Header Files (Interface) und C-Files (Implementation), Information Hiding und die Darstellung von Modulabhängigkeiten in UML.

## Lernziele

- Vorteile von modularem Source Code verstehen und erklären
- C-Programme in Module mit klarer Trennung von Interface und Implementation gliedern
- Header Files korrekt aufbauen (nur Deklarationen, Macros, Re-definition Guards)
- Modulabhängigkeiten in UML-ähnlichen Diagrammen darstellen
- Memory-mapped I/O für LCD und 7-Segment-Anzeige auf dem CT-Board verwenden

## Kernkonzepte

### Modulare Programmierung in C

Ein Modul besteht aus zwei Dateien:
- **Header File (`.h`)** — das **Interface**: Was kann das Modul? Enthält ausschliesslich Deklarationen (Prototypen), Macros und Re-definition Guards. Keine Funktionsdefinitionen, keine Variablenallokation.
- **C-File (`.c`)** — die **Implementation**: Wie macht das Modul es? Enthält die Funktionsdefinitionen und modulinterne Variablen.

**Prinzip:** Ein Anwender soll das Modul allein anhand des Header Files nutzen können, ohne ins C-File schauen zu müssen.

### Information Hiding mit `static`

Das Schlüsselwort `static` auf Modul-Ebene beschränkt die Sichtbarkeit einer Variable auf das aktuelle C-File:

```c
static uint8_t dice_counter = 1;  // nur innerhalb counter.c sichtbar
```

Andere Module können nicht direkt auf `dice_counter` zugreifen — nur über die öffentlichen Funktionen `counter_read()` und `counter_increment()`. Dies ist das C-Äquivalent zu **private** in objektorientierten Sprachen.

### Re-definition Guards

Verhindern mehrfaches Einbinden desselben Header Files:

```c
#ifndef _MODULE_H
#define _MODULE_H
/* ... Deklarationen ... */
#endif
```

Ohne Guard würde ein Header, der von mehreren C-Files inkludiert wird, zu Compiler-Fehlern wegen doppelter Definitionen führen.

### Include-Konventionen

```c
#include <stdint.h>      // Standard-Bibliotheken in spitzen Klammern
#include "my_module.h"   // Eigene Module in Anführungszeichen
```

Das eigene Header File wird im zugehörigen C-File **immer** eingebunden — so prüft der Compiler, ob Deklaration und Definition übereinstimmen.

### Memory-Mapped I/O auf dem CT-Board

Peripherie wird über Speicheradressen angesprochen:

```c
CT_LCD->ASCII[5] = 'q';       // Zeichen an LCD-Position schreiben
CT_LCD->BG.GREEN = 65535u;     // Hintergrundfarbe setzen
hal_ct_seg7_bin_write(value);  // 7-Segment-Anzeige beschreiben
```

Das Modul `reg_ctboard` stellt die nötigen Strukturen (`CT_LCD`, `CT_BUTTON`) als memory-mapped Pointer bereit.

## Aufgaben und Umsetzung

### Aufgabe 1: UML-Diagramm der Modulstruktur

- **Ziel:** Alle Module mit öffentlichen/privaten Funktionen und `uses`-Abhängigkeiten darstellen
- **Umsetzung:** PlantUML-Diagramm in `uml_diagram.puml`
- **Ergebnis:** 6 Module identifiziert (`main`, `counter`, `statistics`, `display`, `button_state`, `dice`) plus HAL-Schicht (`hal_ct_seg7`, `reg_ctboard`)

### Aufgabe 2: Modul `counter` — Würfel

- **Ziel:** Pseudo-Zufallszahlen $1 \ldots 6$ generieren
- **Umsetzung:** `rand() % 6 + 1` für Zufallswert, `static uint8_t dice_counter` als modulinterne Variable
- **Schlüssel-Code:**
  ```c
  uint8_t counter_read(void) {
      return rand() % 6 + 1;
  }
  ```

### Aufgabe 3: Modul `statistics` — Wurfstatistik

- **Ziel:** Anzahl Würfe total und pro Augenzahl zählen
- **Umsetzung:** Array `nr_of_throws[7]` — Index 0 für Total, Index $1 \ldots 6$ pro Wert
- **Schlüssel-Code:**
  ```c
  static uint8_t nr_of_throws[NR_OF_DICE_VALUES + 1] = {0};

  void statistics_add_throw(uint8_t throw_value) {
      nr_of_throws[0]++;
      nr_of_throws[throw_value]++;
  }
  ```

### Aufgabe 4: Modul `display` — LCD-Ausgabe

- **Ziel:** Würfelwert auf 7-Segment, Statistik auf LCD (Zeile 1: pro Wert, Zeile 2: Total)
- **Umsetzung:** Zeichenweiser Zugriff auf `CT_LCD->ASCII[]`, `snprintf()` für formatierte Ausgabe, manuelle ASCII-Konvertierung für zweistellige Zahlen
- **Schlüssel-Code:**
  ```c
  void display_write_total(uint8_t total_value) {
      char buffer[NR_OF_CHAR_PER_LINE] = {' '};
      snprintf(buffer, sizeof(buffer), "total throws %u", total_value);
      for (uint8_t i = 0; i <= NR_OF_CHAR_PER_LINE; i++) {
          CT_LCD->ASCII[LCD_ADDR_LINE2 + i] = buffer[i];
      }
  }
  ```

## Wichtige Register / Hardware

Diese Woche hat keine direkten Register-Zugriffe — stattdessen die Module und deren Hardware-Anbindung:

| Register | Adresse/Pfad | Relevante Bits | Funktion |
|----------|-------------|----------------|----------|
| `CT_LCD->ASCII[]` | Memory-mapped via `reg_ctboard` | Byte-Array (40 Zeichen) | LCD-Zeichen schreiben (Zeile 1: Index 0–19, Zeile 2: Index 20–39) |
| `CT_LCD->BG.GREEN` | Memory-mapped via `reg_ctboard` | 16-Bit (0–65535) | LCD-Hintergrund Grünintensität |
| `CT_BUTTON` | Memory-mapped via `reg_ctboard` | Bit 0: T0 | Tastenstatus lesen |
| `hal_ct_seg7` | HAL-Funktion | 8-Bit Wert | 7-Segment-Anzeige beschreiben |

## Schlussfolgerungen

- **Modulare Struktur** ist essentiell für wartbaren Code: Jedes Modul hat eine klar definierte Aufgabe und ein sauberes Interface
- **Information Hiding** via `static` verhindert unbeabsichtigte Zugriffe auf interne Daten — die einzige Kommunikation läuft über die öffentlichen Funktionen im Header File
- **Header Files** dienen als Vertrag: Sie definieren *was* ein Modul kann, ohne zu verraten *wie*
- **Re-definition Guards** sind Pflicht in jedem Header File, um Mehrfach-Inklusion zu vermeiden
- **Flankenauswertung** im `button_state`-Modul (`~last & current`) ist ein wichtiges Embedded-Pattern: Nur der Übergang von "nicht gedrückt" zu "gedrückt" wird erkannt, nicht das dauerhafte Halten

## Verwandte Themen

- **Woche 2 (Bus Cycles):** Vertieft den Hardware-Zugriff via Memory-Mapped I/O und den synchronen Datenbus
- **Woche 3 (GPIO):** Direkter Register-Zugriff auf GPIO-Pins — baut auf dem Verständnis von Modulstruktur und Volatile-Zugriffen auf
