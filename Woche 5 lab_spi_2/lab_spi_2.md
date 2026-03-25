# CT2 Praktikum: Ansteuerung eines TFT Displays über SPI — Teil 2

## 1. Einleitung

In diesem Praktikum wird das Übertragungsprotokoll für ein TFT-Display (EA eDIPTFT43-ATP) über SPI implementiert. Das Protokoll ermöglicht es, Befehle an das Display zu senden (mit Prüfsumme und ACK/NAK) und Touch-Eingaben vom Display zu empfangen. Alternativ kann ein Mocked Display (Software-Attrappe) verwendet werden, falls kein physisches Display verfügbar ist.

## 2. Lernziele

- Überliegendes Protokoll auf Byte-orientiertem SPI implementieren (Zeichenfolgen mit Fehlererkennung und Empfangsbestätigung)
- C-Programm in Schichten strukturieren (Hauptprogramm → Befehle → Protokoll → HAL)

## 3. Programmrahmen

### Schichtenübersicht

```
main.c                    →  Hauptprogramm (Befehle aufrufen)
  ↓
cmd_lcd.c / cmd_touch.c   →  Display-/Touch-Befehle (Zeichenfolgen bereitstellen)
  ↓
lcd_io.c                  →  Übertragungsprotokoll (Einpacken/Entpacken)
  ↓
hal_spi.c / hal_sbuf.c    →  SPI HAL (Byte-Transfer, SBUF-Signal)
```

### Konfiguration: TFT vs. Mocked Display

- **TFT Display angeschlossen:** `hal_spi.c` und `hal_spi.h` aus Woche 4 verwenden. Preprocessor-Symbol `MOCKED_SPI_DISPLAY` **nicht** definieren.
- **Kein TFT Display:** `hal_mocked.c` und `hal_mocked.h` verwenden. In Keil unter C/C++ → Preprocessor Symbols: `MOCKED_SPI_DISPLAY` definieren.

## 4. Aufgabe 1: Befehle an Display senden (`write_cmd_to_display`)

### Protokoll

```
CT-Board → Display:  [DC1 0x11] [len] [ESC 0x1B] [Befehlsdaten] [bcc]
Display → CT-Board:  [ACK 0x06] oder [NAK 0x15]
```

- `len` = Anzahl Nutzdaten inkl. ESC, ohne DC1 und bcc
- `bcc` = Summe aller gesendeten Bytes (inkl. DC1, len) modulo 256
- ACK muss vom CT-Board eingelesen werden, damit der Befehl ausgeführt wird

### Lösung (`lcd_io.c`)

```c
uint8_t write_cmd_to_display(const uint8_t *cmdBuffer, uint8_t length)
{
    uint8_t bcc = 0;
    uint8_t rec_byte;
    uint8_t i;

    // Länge muss um 1 erhöht werden, da ESC mitzählt
    uint8_t total_length = length + 1;

    // 1. DC1 senden
    hal_spi_read_write(DC1_CHAR);
    bcc += DC1_CHAR;

    // 2. Länge senden
    hal_spi_read_write(total_length);
    bcc += total_length;

    // 3. ESC senden (Befehlseinleitung)
    hal_spi_read_write(ESC_CHAR);
    bcc += ESC_CHAR;

    // 4. Befehlsdaten senden
    for (i = 0; i < length; i++) {
        hal_spi_read_write(cmdBuffer[i]);
        bcc += cmdBuffer[i];
    }

    // 5. Prüfsumme senden
    hal_spi_read_write(bcc);

    // 6. ACK/NAK empfangen (Dummy-Byte 0x00 senden)
    rec_byte = hal_spi_read_write(0x00);

    // 7. Ergebnis auswerten
    if (rec_byte == ACK_CHAR) {
        return SUCCESS;     // 0
    } else {
        return ERRORCODE;   // 1
    }
}
```

### Beispiel: ClearDisplay ("DL")

| Schritt | Byte | Beschreibung |
|---------|------|-------------|
| 1 | `0x11` | DC1 (Start) |
| 2 | `0x03` | len = 2 (Befehl) + 1 (ESC) = 3 |
| 3 | `0x1B` | ESC (Befehlseinleitung) |
| 4 | `0x44` | 'D' |
| 5 | `0x4C` | 'L' |
| 6 | `0xBF` | bcc = (0x11+0x03+0x1B+0x44+0x4C) mod 256 |

## 5. Aufgabe 2: Daten aus Display auslesen (`read_display_buffer`)

### Protokoll (3 Schritte)

**Schritt 1 — Leseanfrage:**
```
CT-Board → Display:  [DC2 0x12] [0x01] ['S' 0x53] [bcc 0x66]
```

**Schritt 2 — Bestätigung:**
```
Display → CT-Board:  [ACK 0x06] oder [NAK 0x15]
```

**Schritt 3 — Daten empfangen:**
```
Display → CT-Board:  [DC1 0x11] [len] [Nutzdaten...] [bcc]
```

### Voraussetzung: SBUF-Signal

Vor dem Lesen muss geprüft werden, ob Daten bereitliegen:

```c
uint8_t hal_sbuf_get_state(void);  // Gibt 1 zurück wenn Daten vorhanden
```

Falls `hal_sbuf_get_state()` 0 zurückgibt, soll `read_display_buffer()` sofort 0 zurückgeben ohne eine Leseanfrage zu starten.

### Lösung: `send_read_display_buffer_request()` (Schritt 1+2)

```c
static uint8_t send_read_display_buffer_request(void)
{
    uint8_t bcc = 0;
    uint8_t rec_byte;

    // DC2 senden
    hal_spi_read_write(DC2_CHAR);
    bcc += DC2_CHAR;

    // Länge senden (0x01)
    hal_spi_read_write(ONE_CHAR);
    bcc += ONE_CHAR;

    // Kommando 'S' (0x53) senden
    hal_spi_read_write(0x53);
    bcc += 0x53;

    // Prüfsumme senden
    hal_spi_read_write(bcc);

    // ACK empfangen
    rec_byte = hal_spi_read_write(0x00);

    if (rec_byte == ACK_CHAR) {
        return SUCCESS;
    } else {
        return ERRORCODE;
    }
}
```

### Lösung: `read_display_buffer()` (vollständig)

```c
uint8_t read_display_buffer(uint8_t *readBuffer)
{
    uint8_t rec_bytes = 0;
    uint8_t len;
    uint8_t i;

    // Leseanfrage senden und ACK prüfen
    if (send_read_display_buffer_request() != SUCCESS) {
        return 0;
    }

    // Warten bis Display Daten bereit hat (SBUF = 1)
    while (hal_sbuf_get_state() == 0) {
        /* warten */
    }

    // Antwortpaket empfangen: DC1, len, Nutzdaten..., bcc
    hal_spi_read_write(0x00);           // DC1 lesen (verwerfen)
    len = hal_spi_read_write(0x00);     // Länge lesen

    // Nutzdaten lesen
    for (i = 0; i < len; i++) {
        readBuffer[i] = hal_spi_read_write(0x00);
        rec_bytes++;
    }

    hal_spi_read_write(0x00);           // bcc lesen (verwerfen)

    return rec_bytes;
}
```

### Touch-Event-Datenformat

| Empfangene Nutzdaten | Bedeutung |
|---------------------|-----------|
| `ESC 'A' 0x01 0x01` | Button gedrückt (DOWN) |
| `ESC 'A' 0x01 0x02` | Button losgelassen (UP) |

### Main-Loop: Touch-Events auf LEDs anzeigen

In der `while(1)`-Schleife in `main.c` werden Touch-Events eingelesen und auf den LEDs L7–L0 ausgegeben:

```c
while (1) {
    nrOfChars = read_display_buffer(readBuffer);
    if (nrOfChars >= MIN_LENGTH_BUFFER_MESSAGE) {
        if (readBuffer[3] == BUTTON1_DOWN) {
            CT_LED->BYTE.LED7_0 = LED_ON;
        } else if (readBuffer[3] == BUTTON1_UP) {
            CT_LED->BYTE.LED7_0 = LED_OFF;
        }
    }
}
```

## 6. Bewertung

| Bewertungskriterien | Gewichtung |
|---------------------|-----------|
| Befehle können an das Display übertragen werden | 1/2 |
| Touch-Events können ausgelesen und an LEDs L7–L0 angezeigt werden | 1/2 |

## 7. Schlussfolgerungen

- **len zählt ESC mit**: Die Längenfeldberechnung muss das ESC-Byte (`0x1B`) einschliessen, daher `len = cmdBuffer_length + 1`. Dies ist ein häufiger Fehler.
- **bcc schliesst DC1/DC2 und len ein**: Die Prüfsumme wird über alle Bytes berechnet — inklusive Start-Steuerzeichen und Längenfeld, aber exklusive der bcc selbst.
- **ACK muss aktiv gelesen werden**: Das Display führt einen Befehl erst aus, nachdem das CT-Board das ACK eingelesen hat. Ein vergessenes `hal_spi_read_write(0x00)` nach dem Senden führt dazu, dass der Befehl nicht ausgeführt wird.
- **SBUF prüfen vor Lesen**: Ohne Prüfung des SBUF-Signals würde eine Leseanfrage ins Leere gehen. Die SBUF-Leitung zeigt aktiv an, ob im Display Daten zum Senden bereitliegen.
- **Dummy-Byte beim Empfang**: Um Daten vom SPI-Slave zu empfangen, muss der Master trotzdem ein Byte senden (SPI ist immer Full-Duplex). Convention: `0x00` als Dummy senden.
