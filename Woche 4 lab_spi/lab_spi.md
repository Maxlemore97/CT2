# CT 2 — Praktikum: Ansteuerung eines TFT-Displays über SPI — Teil 1

## 1 Einleitung

In diesem Praktikum werden auf dem CT System die Grundfunktionen für das Senden und Empfangen eines Bytes über SPI implementiert. Die SPI Grundfunktionen werden im nachfolgenden Praktikum für die Ansteuerung eines intelligenten Displays verwendet. Das Display wird erst im nächsten Praktikum angeschlossen.

> Abbildung 1: TFT-Display (EA eDIPTFT43-ATP) mit SPI Schnittstelle auf dem CT-Board.

## 2 Lernziele

- Sie verstehen die verschiedenen Timing Optionen einer SPI Schnittstelle und können diese auf dem STM32F4 konfigurieren.
- Sie können aus Ihrer Software heraus einzelne Bytes über SPI versenden und empfangen.
- Sie können die übertragenen SPI Daten mithilfe des Oszilloskops analysieren.
- Sie vertiefen Ihre Programmierkenntnisse in C.

## 3 Material

- CT-Board
- Logic Analyzer mit Flachbandkabeln **und Messklemmen**
- Drahtbrücken mit «Abgrif». Siehe Abbildung 4.

## 4 Schnittstelle

Die SPI-Schnittstelle wird verwendet, um mit einem intelligenten Display zu kommunizieren. Die SPI Signale werden auf dem CT Board über den Port 5 angesteuert.

### PORT / Signal Belegung (Abbildung 2)

| Port 5 Pin | Signal | Richtung | Funktion |
|------------|--------|----------|----------|
| P5.5 | $\overline{SS}$ | → (Output) | Slave Select (active low) |
| P5.6 | SCK | → (Output) | Serial Clock |
| P5.7 | MISO | ← (Input) | Master In, Slave Out |
| P5.8 | MOSI | → (Output) | Master Out, Slave In |
| P5.9 | SBUF | ← (Input) | Buffer-Status vom Display |

Eine Beschreibung des Port 5 finden Sie im InES CT Board Wiki (*www.ennis.zhaw.ch*).

Mit einem tiefen Pegel am Signal $\overline{SBUF}$ zeigt das Display an, dass im internen Buffer des Displays Daten zur Abholung durch den Microcontroller bereit stehen. Dieses Signal wird erst im nächsten Praktikum verwendet.

### SPI-Timing (Abbildung 3)

Für die Ansteuerung des Displays muss das SPI-Timing gemäss **SPI Mode 0** verwendet werden:

- **CPOL = 0**: CLK idle state LOW
- **CPHA = 0**: Data valid at first edge (steigende Flanke)
- **DORD = 0**: MSB = send Bit 7 first

```
     SS  ‾‾‾\_______________________/‾‾‾\_____
    CLK  ____/‾\_/‾\_/‾\_/‾\_/‾\_/‾\_/‾\_/‾\____/‾\_
   MOSI  ----< 7 >< 6 >< 5 >< 4 >< 3 >< 2 >< 1 >< 0 >----< 7 >
   MISO  ----< 7 >< 6 >< 5 >< 4 >< 3 >< 2 >< 1 >< 0 >----< 7 >
```

Das Display erlaubt bei einer pausenlosen Übertragung von Bytes eine Clockfrequenz von bis zu **200 kHz**. Die SPI-Funktionseinheit ist im Rahmenprogramm auf einen Clock von 42 MHz eingestellt. D.h. $f_{PCLK}$ gemäss Reference Manual ist gleich 42 MHz. Sie müssen daher in der folgenden Aufgabe den Prescaler der SPI-Schnittstelle entsprechend konfigurieren.

## 5 Aufgaben

Wir stellen Ihnen ein Testprogramm `test.c` sowie die Files `hal_spi.c` (Programmrahmen) und `hal_spi.h` (Header File) für die Implementation des SPI zur Verfügung.

### 5.1 Initialisierung

Die Funktion `void hal_spi_init(void)` zur Initialisierung der SPI Schnittstelle ist bereits teilweise implementiert. Die Konfiguration der GPIOs wurde bereits vorgenommen.

**Aufgabe:** Überlegen Sie sich, welche Bits im Kontrollregister `SPI1->CR1` gesetzt werden müssen, damit das geforderte Timing des SPI-Displays eingehalten wird und vervollständigen Sie dann den Code.

#### Hinweise zur Initialisierung der SPI-Schnittstelle

- Im beigelegten Reference Manual finden Sie auf Seite 896 eine Beschreibung des Konfigurationsregisters CR1.
- Das Register CR1 kann über das bereits vordefinierte Macro `SPI1->CR1` angesprochen werden.
- Die Verdrahtung in Abbildung 2 erfordert den von ST als '2-line unidirectional data mode' bezeichneten Modus. Dabei muss full-duplex eingestellt werden, d.h. RXONLY = 0. Das Kontrollbit BIDIOE ist bedeutungslos und kann auf Null gesetzt werden (BIDIOE = 0).
- Das Display unterstützt keinen CRC (Cyclic Redundancy Check). Setzen Sie daher CRCEN = CRCNEXT = 0.
- Im vorliegenden Fall wird das Slave Select Signal durch die Software über den GPIO PinA.5 auf dem Stecker P5.5 kontrolliert. Setzen Sie deshalb SSM = SSI = 1.
- Erst wenn alle Settings im Register CR1 gemacht sind, darf (und muss) das SPE Bit gesetzt werden. Ab dann sind alle Settings „scharf" gestellt und der Daten Transfer kann beginnen.
- Das Register CR2 wird im vorgegebenen Rahmen auf 0x0000 initialisiert und muss nicht verändert werden. Dadurch sind alle Interrupts und die DMA (Direct Memory Access) disabled.

#### Lösung: CR1 Bit-Konfiguration

| Bit | Name | Wert | Begründung |
|-----|------|------|------------|
| 15 | BIDIMODE | 0 | 2-line unidirectional data mode |
| 14 | BIDIOE | 0 | Bedeutungslos im 2-line mode |
| 13 | CRCEN | 0 | Kein CRC (Display unterstützt kein CRC) |
| 12 | CRCNEXT | 0 | Kein CRC |
| 11 | DFF | 0 | 8-bit Datenformat |
| 10 | RXONLY | 0 | Full-duplex (Transmit and Receive) |
| 9 | SSM | 1 | Software slave management (SS via GPIO) |
| 8 | SSI | 1 | Internal slave select high (verhindert MODF-Fehler) |
| 7 | LSBFIRST | 0 | MSB wird zuerst übertragen |
| 6 | SPE | 1 | SPI Enable (erst nach allen anderen Settings setzen!) |
| 5:3 | BR[2:0] | 111 | $f_{PCLK}/256 = 42\,\text{MHz} / 256 = 164\,\text{kHz}$ (< 200 kHz) |
| 2 | MSTR | 1 | Master-Konfiguration |
| 1 | CPOL | 0 | Clock idle state = LOW (SPI Mode 0) |
| 0 | CPHA | 0 | Daten gültig bei erster Taktflanke (SPI Mode 0) |

**Prescaler-Berechnung:**

$$f_{SPI} = \frac{f_{PCLK}}{Prescaler} = \frac{42\,\text{MHz}}{256} = 164{,}0625\,\text{kHz} < 200\,\text{kHz} \checkmark$$

Benötigter Teiler: $42\,000\,000 / 200\,000 = 210$ → nächster verfügbarer Prescaler: 256 → BR[2:0] = `111`

Warum nicht /128 (328 kHz)? → Würde die 200 kHz Grenze des Displays überschreiten.

**Resultierende CR1-Werte:**
- Vor SPE: `0x033C` (MSTR + BR=111 + SSM + SSI)
- Nach SPE: `0x037C` (+ SPE)

#### Code-Lösung (`hal_spi.c`):

```c
/// STUDENTS: To be programmed

SPI1->CR1 |= (0x7 << 3);   // BR[2:0] = 111 --> fPCLK/256 = 164 kHz
SPI1->CR1 |= (0x1 << 2);   // MSTR = 1     --> Master mode
SPI1->CR1 |= (0x1 << 9);   // SSM = 1      --> Software slave management
SPI1->CR1 |= (0x1 << 8);   // SSI = 1      --> Internal slave select high
SPI1->CR1 |= (0x1 << 6);   // SPE = 1      --> SPI enable (set last)

/// END: To be programmed
```

### 5.2 Senden/Empfangen

Implementieren Sie das Senden von Daten in der unten stehenden Sende-/Empfangsfunktion, die in `hal_spi.h` als Funktionsprototyp und in `hal_spi.c` als Rahmen vordefiniert ist:

Die Funktion `uint8_t hal_spi_read_write(uint8_t send_byte)` soll das übergebene Byte versenden und das empfangene Byte zurückgeben.

#### Hinweise zur Verwendung der SPI-Schnittstelle

- Die Funktionen zur Steuerung des $\overline{SS}$ Signals `static void set_ss_pin_low()` und `static void set_ss_pin_high()` sind gegeben.
- Zusätzlich ist auch die Funktion `static void wait_10_us()` gegeben. Verwenden Sie diese um sicherzustellen, dass das $\overline{SS}$ Signal erst nach der letzten Clock-Flanke auf High gesetzt wird. Dies können Sie mit dem Oszilloskop überprüfen.
- In den Vorlesungsunterlagen oder im beigelegten Reference Manual auf Seite 861 finden Sie, wie zum Senden und Empfangen vorgegangen werden muss. Das Datenregister `SPI1->DR` und das Kontrollregister `SPI1->SR` sind bereits vordefiniert. Eine Beschreibung des Kontrollregisters und der Flags finden Sie im Reference Manual auf den Seiten 872 und 900.

#### Lösung: Ablauf (gemäss Reference Manual S.861)

1. **SS low setzen** (`set_ss_pin_low()`) — Slave aktivieren
2. **Warten auf TXE=1** — Tx-Buffer ist leer und bereit für neue Daten
3. **Byte in `SPI1->DR` schreiben** — Startet die SPI-Übertragung
4. **Warten auf RXNE=1** — Empfangene Daten im Rx-Buffer bereit
5. **`SPI1->DR` lesen** — Empfangenes Byte auslesen (löscht RXNE-Flag)
6. **Warten auf BSY=0** — Übertragung vollständig abgeschlossen
7. **10 µs warten** (`wait_10_us()`) — Sicherstellen, dass SS erst nach letzter Clock-Flanke hochgeht
8. **SS high setzen** (`set_ss_pin_high()`) — Slave deaktivieren
9. **Empfangenes Byte zurückgeben**

**Relevante Status-Flags (`SPI1->SR`):**

| Flag | Bit | Bitmask | Bedeutung |
|------|-----|---------|-----------|
| TXE | 1 | `0x00000002` | Transmit buffer empty |
| RXNE | 0 | `0x00000001` | Receive buffer not empty |
| BSY | 7 | `0x00000080` | SPI busy (Transfer am Laufen) |

#### Code-Lösung (`hal_spi.c`):

```c
/// STUDENTS: To be programmed

uint8_t rec_byte;

set_ss_pin_low();

while (!(SPI1->SR & BIT_TXE)) {}    // wait until Tx buffer empty
SPI1->DR = send_byte;                // write byte to data register

while (!(SPI1->SR & BIT_RXNE)) {}   // wait until Rx buffer not empty
rec_byte = SPI1->DR;                 // read received byte

while (SPI1->SR & BIT_BSY) {}        // wait until SPI not busy

wait_10_us();
set_ss_pin_high();

return rec_byte;

/// END: To be programmed
```

### 5.3 Testen der Funktion

Verifizieren Sie Ihre Sende/Empfangsfunktion mit Hilfe des Testprogrammes `test.c`. Das Testprogramm liest ein Byte von S7 bis S0 ein, gibt den Wert auf Led7 bis Led0 aus und sendet ihn via SPI. Der von der SPI-Schnittstelle empfangene Wert wird auf Led23 bis Led16 ausgegeben.

**Loopback-Test:** Verbinden Sie für den Test die MOSI und MISO (Pin 7 und 8) mit einer Drahtbrücke auf dem Port 5.

> Abbildung 4: P5 Pin-Belegung — Pin 7 (MISO) und Pin 8 (MOSI) werden mit Drahtbrücke verbunden.

**Messen** Sie zusätzlich die Signale MOSI, CLK und $\overline{SS}$ mit der SPI-Analysefunktion des Oszilloskops am Port 5.

> Platzhalter: Screenshot Logic Analyzer — zeigt die SPI-Signale (CLK, MOSI/MISO, SS) beim Loopback-Test.

**Achtung: Für diesen Test darf das Display nicht angeschlossen werden → Kurzschluss**

#### Hinweise zur SPI-Analysefunktion des Oszilloskops

- Die SPI Pins können über die analogen oder über die digitalen Kanäle angeschlossen werden.
- Definieren Sie den Bus über die blaue Taste „Bus B1". Wählen Sie „SPI", definieren Sie die Eingänge, kontrollieren Sie die Schwellenwerte und konfigurieren Sie Polarität und Phase des SPI unter „SPI Einstellungen".
- Haben Sie Spikes auf den Signalen, dann überprüfen sie den Thresholdwert und korrigieren ihn gegebenenfalls auf 2.5 V.

#### Lösung: Loopback-Test

Das Testprogramm (`test.c`) implementiert folgenden Ablauf:

```c
int32_t main(void)
{
    uint8_t send_byte;
    uint8_t rec_byte;

    hal_spi_init();

    while (1) {
        send_byte = CT_DIPSW->BYTE.S7_0;       // DIP Switches lesen
        CT_LED->BYTE.LED7_0 = send_byte;        // Gesendeter Wert auf LEDs
        rec_byte = hal_spi_read_write(send_byte); // Senden & Empfangen
        CT_LED->BYTE.LED23_16 = rec_byte;       // Empfangener Wert auf LEDs
    }
}
```

**Erwartetes Ergebnis:** Bei korrektem Loopback (MOSI→MISO) müssen LED7–LED0 und LED23–LED16 **identisch** sein, da das gesendete Byte direkt zurückkommt.

### 5.4 Bewertung

Die lauffähigen Programme müssen präsentiert werden. Die einzelnen Studierenden müssen die Lösungen und den Quellcode verstanden haben und erklären können.

| Bewertungskriterien | Gewichtung |
|---------------------|------------|
| Die SPI-Schnittstelle wird wie gefordert initialisiert | 1/4 |
| Das Programm erfüllt die geforderte Funktionalität | 1/4 |
| Das Programm wurde entsprechend getestet und funktioniert korrekt. Ein Screenshot des Oszilloskops wird gezeigt und erklärt | 2/4 |

## 6 Anhang: MISO / MOSI Abbildung 1–3

Verwenden Sie für die Überbrückung einen Kurzschlussdraht.

> Abbildung: Nahaufnahme des Port 5 Steckers auf dem CT-Board. Pin 7 (MISO) und Pin 8 (MOSI) sind mit einem blauen Kurzschlussdraht verbunden.
