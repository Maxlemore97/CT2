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

### Woche 3 - GPIO (General Purpose Input/Output)
Konfiguration und Verwendung von GPIO-Pins zur Ein- und Ausgabe mittels Registerzugriff.

**Themen aus der Lektion:**
- GPIO-Konfiguration über Register: MODER, OTYPER, OSPEEDR, PUPDR
- Registerzugriff über HAL-Strukturen (`reg_stm32f4xx.h`): Basisadressen, Macros, Structs
- Output-Typen: Push-Pull vs. Open Drain
- Pull-up / Pull-down Widerstände und deren Einfluss auf Eingänge
- Unterschiedliche Output-Geschwindigkeiten (Low, Medium, High, Very High)

**Praktikum:**
- GPIOA (PA0–PA2) als Eingänge konfigurieren mit verschiedenen Pull-Konfigurationen
- GPIOB (PB0–PB2) als Ausgänge konfigurieren (Push-Pull, Open Drain, verschiedene Speeds)
- DIP-Switches einlesen, über GPIO ausgeben und über Verbindungsdrähte zurücklesen
- Untersuchung aller Output/Input-Kombinationen (Push-Pull vs. Open Drain, Pull-up/down/none)

### Woche 4 - SPI (Serial Peripheral Interface)
Ansteuerung eines TFT-Displays über SPI — Teil 1: Initialisierung und Sende-/Empfangsfunktion.

**Themen aus der Lektion:**
- SPI-Protokoll: synchrone, serielle Vollduplex-Schnittstelle (MOSI, MISO, SCK, SS)
- SPI-Modi (CPOL/CPHA): 4 Timing-Varianten, Display erfordert Mode 0
- CR1-Register: Konfiguration von Prescaler, Master/Slave, Software Slave Select, Enable
- Prescaler-Berechnung: $f_{SPI} = f_{PCLK} / Prescaler$
- Status-Flags (TXE, RXNE, BSY) für Polling-basierte Kommunikation

**Praktikum:**
- SPI1 als Master initialisieren (Mode 0, 164 kHz, Software Slave Select)
- Sende-/Empfangsfunktion implementieren (Full-Duplex, Polling)
- Loopback-Test: MOSI↔MISO verbinden, gesendete = empfangene Daten verifizieren
- SPI-Signale mit Oszilloskop/Logic Analyzer analysieren

### Woche 5 - SPI Part 2: Übertragungsprotokoll und TFT-Display
Ansteuerung eines TFT-Displays über SPI — Teil 2: Übertragungsprotokoll mit Prüfsumme und Touch-Eingaben.

**Themen aus der Lektion:**
- Übertragungsprotokoll: Befehle in Rahmen verpacken (DC1, len, ESC, Daten, bcc)
- Prüfsumme (BCC): Summe aller Bytes modulo 256
- Empfangsbestätigung: ACK/NAK-Handshake
- Schichtenarchitektur: Hauptprogramm → Befehle → Protokoll → HAL
- Mocked Hardware: Software-Attrappe für nicht verfügbare Hardware

**Praktikum:**
- `write_cmd_to_display()` implementieren: Befehle mit Protokollrahmen an Display senden
- `read_display_buffer()` implementieren: Touch-Events vom Display empfangen
- SBUF-Signal auswerten (Datenbereitschaft des Displays)
- Touch-Button-Events auf LEDs L7–L0 anzeigen

### Woche 6 - Timer und PWM
Ansteuerung von RGB-LEDs über PWM-Signale mit den Timern TIM3 und TIM4.

**Themen aus der Lektion:**
- General-Purpose Timer TIM2–TIM5: Prescaler, Auto-Reload, Zählrichtung
- Interrupt-Erzeugung: Update Event (UIF), ISR, NVIC-Freischaltung
- PWM-Erzeugung: Output Compare Mode 1, Duty Cycle über Compare-Register (CCRx)
- Prescaler/Reload-Berechnung: $f_{PWM} = f_{CK\_INT} / ((PSC+1) \cdot (ARR+1))$
- GPIO Alternate Function für Timer-Ausgänge

**Praktikum:**
- TIM4 als Downcounter mit 1-Sekunden-Interrupt konfigurieren (LEDs blinken)
- TIM3 als PWM-Generator für 3 Kanäle (200 Hz, RGB) konfigurieren
- Duty Cycles über DIP Switches (4-Bit, Skalierung × 4000) einstellen
- Automatischer Farbwechsel im Sekundentakt (ISR-basierter Zähler)

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
├── Woche 3 lab_gpio/
│   ├── project/app/       # GPIO Applikation
│   └── lab_GPIO_fragen.md # Vorbereitungsfragen & Antworten
├── Woche 4 lab_spi/
│   ├── project/app/       # SPI Applikation (hal_spi.c, test.c)
│   ├── lab_SPI_part1_fragen.md # Vorbereitungsfragen & Antworten
│   └── SPI_zusammenfassung.md  # Reference Manual Zusammenfassung
```

## Toolchain

- Keil µVision (ARM Compiler)
- CT-Board (STM32F429ZI, Modus 3)
- Tektronix MDO3014 Oszilloskop (Logic Analyzer)