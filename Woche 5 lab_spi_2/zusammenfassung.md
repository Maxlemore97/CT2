# Woche 5 — SPI Part 2: Übertragungsprotokoll und TFT-Display-Ansteuerung

## Überblick

Aufbauend auf der SPI-Initialisierung und Byte-Transfer-Funktion aus Woche 4 wurde in dieser Woche das Übertragungsprotokoll für das TFT-Display (EA eDIPTFT43-ATP) implementiert. Das Protokoll ermöglicht das Senden von Befehlen mit Prüfsumme und Empfangsbestätigung sowie das Auslesen von Touch-Events. Falls kein physisches Display verfügbar ist, kann eine Software-Attrappe (Mocked Display) verwendet werden.

## Lernziele

- Ein überliegendes Protokoll in Software realisieren, um auf dem Byte-orientierten SPI die Übertragung von Zeichenfolgen mit Fehlererkennung und Empfangsbestätigung zu ermöglichen
- Programmierkenntnisse in C vertiefen — insbesondere die Strukturierung eines Programms in mehrere Schichten

## Kernkonzepte

### Schichtenarchitektur

Das Programm ist in vier Schichten aufgeteilt, von oben nach unten:

| Schicht | Files | Funktion |
|---------|-------|----------|
| **Hauptprogramm** | `main.c` | Ruft Display-Befehle auf (z.B. "HELLO SPI" anzeigen, Touch-Button definieren) |
| **Display-Befehle** | `cmd_lcd.c/h`, `cmd_touch.c/h` | Stellt Zeichenfolgen für einzelne Display-Befehle bereit (z.B. ClearDisplay, SetFont, DefineButton) |
| **Übertragungsprotokoll** | `lcd_io.c/h` | Packt Befehle in den Protokollrahmen (DC1, len, ESC, Daten, bcc) und entpackt empfangene Daten |
| **SPI HAL** | `hal_spi.c/h`, `hal_sbuf.c/h`, `hal_mocked.c/h` | Byte-Transfer über SPI (aus Woche 4) und SBUF-Signal-Handling |

Diese Schichtung sorgt dafür, dass die obere Schicht nichts über die Details der darunterliegenden Schicht wissen muss. Das Mocked Display ersetzt nur die unterste Schicht — alle anderen Schichten bleiben unverändert.

### Protokoll: Befehle senden (CT-Board → Display)

Jeder Befehl wird in einen Protokollrahmen verpackt:

```
CT-Board sendet:  [DC1] [len] [ESC] [Befehlszeichenfolge] [bcc]
Display antwortet: [ACK] oder [NAK]
```

| Feld | Wert | Beschreibung |
|------|------|--------------|
| DC1 | `0x11` | Start-Steuerzeichen |
| len | variabel | Anzahl der Nutzdaten in Byte (inkl. ESC, ohne DC1 und bcc) |
| ESC | `0x1B` | Befehlseinleitung — zählt zur Länge |
| Befehlszeichenfolge | variabel | Die eigentlichen Befehlsdaten |
| bcc | berechnet | Prüfsumme: Summe aller Bytes (inkl. DC1 und len) modulo 256 |

**Antwort des Displays:**
- **ACK** (`0x06`): Empfang erfolgreich — bestätigt nur korrekte Übertragung, kein Syntax-Check
- **NAK** (`0x15`): Fehlerhafte Prüfsumme oder Empfangspufferüberlauf — Paket muss erneut gesendet werden

**Beispiel — ClearDisplay ("DL"):**
```
Senden: 0x11  0x03  0x1B  0x44  0x4C  0xBF
         DC1   len   ESC   'D'   'L'   bcc

bcc = (0x11 + 0x03 + 0x1B + 0x44 + 0x4C) mod 256 = 0xBF
```

### Protokoll: Daten empfangen (Display → CT-Board)

Das Auslesen des Display-Sendepuffers (z.B. für Touch-Events) erfolgt in drei Schritten:

**Schritt 1 — Leseanfrage senden:**
```
CT-Board sendet: [DC2] [0x01] ['S'] [bcc]
```
- DC2 = `0x12`, 'S' = `0x53` (Sendepuffer entleeren)
- bcc = (0x12 + 0x01 + 0x53) mod 256 = `0x66`

**Schritt 2 — Bestätigung empfangen:**
```
Display antwortet: [ACK] oder [NAK]
```

**Schritt 3 — Daten empfangen:**
```
Display sendet: [DC1] [len] [Nutzdaten] [bcc]
```

**Voraussetzung:** Die SBUF-Leitung (Send Buffer) wird vom Display auf GND gezogen, wenn Daten bereitliegen. Erst dann soll die Leseanfrage gestartet werden.

### Touch-Event-Datenformat

Die empfangenen Nutzdaten bei Touch-Events:

| Byte-Folge | Bedeutung |
|------------|-----------|
| `ESC 'A' 0x01 0x01` | Touch-Button wurde gedrückt (DOWN) |
| `ESC 'A' 0x01 0x02` | Touch-Button wurde losgelassen (UP) |

### Mocked Display

Falls kein physisches TFT-Display verfügbar ist, kann das Mocked Display verwendet werden:

- Aktivierung über Preprocessor-Symbol `MOCKED_SPI_DISPLAY` in den Keil-Projekteinstellungen (C/C++ → Preprocessor Symbols → Define)
- Die Mocked-Implementierung (`hal_mocked.c/h`) ersetzt `hal_spi.c/h` und simuliert das Display-Verhalten aus Protokoll-Sicht
- Im Mocked-Modus wird der blaue Taster T0 auf dem CT-Board als Touch-Button verwendet
- Die Ausgabe erfolgt auf dem LC-Display des CT-Boards statt auf dem TFT

### Prüfsumme (BCC)

Die Block Check Character (BCC) Prüfsumme ist bewusst einfach gehalten:

```c
bcc = (Summe aller Bytes inkl. DC1/DC2 und len) % 256
```

Da `uint8_t` automatisch bei 256 überläuft, reicht es in C, einfach alle Bytes zu addieren — der Overflow ergibt automatisch das Modulo 256.

## Wichtige Steuerzeichen

| Zeichen | Hex-Wert | Funktion |
|---------|----------|----------|
| DC1 | `0x11` | Start eines Sende- oder Empfangspakets |
| DC2 | `0x12` | Start einer Leseanfrage |
| ESC | `0x1B` | Befehlseinleitung innerhalb eines Pakets |
| ACK | `0x06` | Positive Empfangsbestätigung |
| NAK | `0x15` | Negative Empfangsbestätigung (Fehler) |

## Schlussfolgerungen

- **Schichtenarchitektur entkoppelt Hardware und Logik**: Die Display-Befehle wissen nichts über SPI oder das Protokoll. Das Protokoll weiss nichts über die konkrete Hardware. Dies ermöglicht den nahtlosen Austausch von echtem Display und Mocked Display.
- **Prüfsumme sichert Übertragungsintegrität**: Die BCC-Prüfsumme ist eine einfache Summe modulo 256 — nicht kryptographisch sicher, aber ausreichend für kurze Distanzen über SPI.
- **ACK-Empfang ist essentiell**: Das Display führt einen Befehl erst aus, nachdem das CT-Board das ACK-Byte eingelesen hat. Ohne Lesen des ACK wird der Befehl nicht ausgeführt.
- **SBUF-Signal verhindert unnötiges Polling**: Statt blind Daten vom Display abzufragen, signalisiert die SBUF-Leitung aktiv, dass Daten bereitliegen. Dies spart SPI-Bandbreite und verhindert fehlerhafte Lesevorgänge.
- **Mocked Hardware ist ein wichtiges Entwicklungsmuster**: Wenn die Ziel-Hardware noch nicht verfügbar ist, können Software-Attrappen die Hardware-Schicht ersetzen. Die Schnittstelle bleibt identisch — nur die Implementierung ändert sich.

## Verwandte Themen

- **Woche 4 (SPI Part 1):** SPI-Initialisierung und `hal_spi_read_write()` — die Grundlage für die Byte-Übertragung in diesem Praktikum
- **Woche 3 (GPIO):** GPIO-Konfiguration — PA8 wird als Input für das SBUF-Signal verwendet
- **Woche 6 (Timer/PWM):** Timer und Pulse Width Modulation — nächstes Thema