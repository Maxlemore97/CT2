# CT2 - Computertechnik 2

Repository für das Modul Computertechnik 2 (CT2) an der ZHAW (Zürcher Hochschule für Angewandte Wissenschaften).

## Inhalt

### Woche 1 - Coding Style Structure
Elektronische Würfel-Applikation (Electronic Dice) für das CT-Board.

**Themen aus der Lektion:**
- Vorteile von modularem Source Code gegenüber "All-In-One" Ansatz
- Aufteilung in Header File (.h, Interface) und C-File (.c, Implementation)
- Header File: Funktionsdeklarationen, Macros, Re-Definition Guards (`#ifndef`)
- C-File: Includes, modulweite Variablen (`static`), Funktionsdefinitionen
- Vier Leitlinien zur Strukturierung von C-Programmen in Module
- UML-Moduldiagramme zur Darstellung von Abhängigkeiten

**Praktikum:**
- Elektronischer Würfel mit Modulen: `counter`, `display`, `statistics`, `button_state`
- UML-Moduldiagramm der Applikationsarchitektur (PlantUML)

### Woche 2 - Buszyklen
Analyse von Speicherbus-Zugriffen auf dem CT-Board mit Logic Analyzer.

**Themen aus der Lektion:**
- Synchroner externer Speicherbus (16-Bit Datenbus) des STM32F4xx
- Aufbau des CT-Boards: CPU (Cortex-M4), FMC, CPLD, externer Bus
- Bussignale: CLK, NE (Chip Enable), NOE (Output Enable), NWE (Write Enable), NBL0/NBL1 (Byte Lanes)
- Byte-, Halfword- und Word-Zugriffe auf gerade und ungerade Adressen
- Analyse der Buszyklen mit dem Tektronix Oszilloskop / Logic Analyzer

**Praktikum:**
- DIP-Switches lesen und an LEDs schreiben (8-Bit, 16-Bit, 32-Bit Zugriffe)
- Halfword-Zugriff auf ungerade Adressen (S23…8 → LEDs 23…8)
- Logic-Analyzer-Messungen der verschiedenen Zugriffsarten

## Projektstruktur

```
CT2/
├── Woche 1 lab_coding_style/
│   ├── app/               # Applikationscode (main, Module)
│   ├── RTE/               # Runtime Environment (HAL, Device)
│   ├── build/             # Build-Konfiguration
│   └── uml_diagram.puml  # UML-Diagramm (PlantUML)
├── Woche 2 lab_bus_cycle/
│   ├── project/app/       # Bus-Cycle Applikation
│   └── *.jpeg             # Logic-Analyzer Screenshots
```

## Toolchain

- Keil µVision (ARM Compiler)
- CT-Board (STM32F429ZI, Modus 3)
- Tektronix MDO3014 Oszilloskop (Logic Analyzer)