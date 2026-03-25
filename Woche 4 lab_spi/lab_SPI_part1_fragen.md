# Lab SPI Part 1 - Fragen und Antworten

## 5.1 Initialisierung - Welche Bits im CR1 Register?

### Frage: Welche Bits im Kontrollregister SPI1->CR1 muessen gesetzt werden, damit das geforderte Timing des SPI-Displays eingehalten wird?

**Anforderungen aus dem Lab-Sheet:**
- SPI Mode 0: CPOL=0, CPHA=0 (CLK idle LOW, Data valid at first edge)
- MSB first (DORD=0)
- 2-line unidirectional, full-duplex (BIDIMODE=0, RXONLY=0)
- Kein CRC (CRCEN=0, CRCNEXT=0)
- Software Slave Select (SSM=1, SSI=1)
- Master Mode (MSTR=1)
- 8-bit Datenformat (DFF=0)
- Clock-Frequenz max. 200 kHz (Display-Limit)

**Antwort - CR1 Bit-Konfiguration:**

| Bit | Name     | Wert | Begruendung                                              |
|-----|----------|------|----------------------------------------------------------|
| 15  | BIDIMODE | 0    | 2-line unidirectional data mode                          |
| 14  | BIDIOE   | 0    | Bedeutungslos im 2-line mode                             |
| 13  | CRCEN    | 0    | Kein CRC (Display unterstuetzt kein CRC)                 |
| 12  | CRCNEXT  | 0    | Kein CRC                                                 |
| 11  | DFF      | 0    | 8-bit Datenformat                                        |
| 10  | RXONLY   | 0    | Full-duplex (Transmit and Receive)                       |
| 9   | SSM      | 1    | Software slave management (SS via GPIO)                  |
| 8   | SSI      | 1    | Internal slave select high (verhindert MODF-Fehler)      |
| 7   | LSBFIRST | 0    | MSB wird zuerst uebertragen                              |
| 6   | SPE      | 1    | SPI Enable (erst nach allen anderen Settings setzen!)    |
| 5:3 | BR[2:0]  | 111  | fPCLK/256 = 42 MHz / 256 = 164 kHz (< 200 kHz)         |
| 2   | MSTR     | 1    | Master-Konfiguration                                     |
| 1   | CPOL     | 0    | Clock idle state = LOW (SPI Mode 0)                      |
| 0   | CPHA     | 0    | Daten gueltig bei erster Taktflanke (SPI Mode 0)         |

**Prescaler-Berechnung:**
- fPCLK = 42 MHz
- Max. erlaubte SPI-Frequenz: 200 kHz
- Benoetigter Teiler: 42'000'000 / 200'000 = 210 -> naechster verfuegbarer Prescaler: 256
- BR[2:0] = 111 -> fPCLK/256 = 42 MHz / 256 = 164.0625 kHz (erfuellt < 200 kHz)

**Resultierende CR1-Werte:**
- Vor SPE: `0x033C` (MSTR + BR=111 + SSM + SSI)
- Nach SPE: `0x037C` (+ SPE)

## 5.2 Senden/Empfangen - Ablauf hal_spi_read_write()

### Frage: Wie muss zum Senden und Empfangen vorgegangen werden?

**Antwort - Ablauf (gemaess Reference Manual S.861):**

1. **SS low setzen** (`set_ss_pin_low()`) - Slave aktivieren
2. **Warten auf TXE=1** - Tx-Buffer ist leer und bereit fuer neue Daten
3. **Byte in SPI1->DR schreiben** - Startet die SPI-Uebertragung
4. **Warten auf RXNE=1** - Empfangene Daten im Rx-Buffer bereit
5. **SPI1->DR lesen** - Empfangenes Byte auslesen (loescht RXNE-Flag)
6. **Warten auf BSY=0** - Uebertragung vollstaendig abgeschlossen
7. **10 us warten** (`wait_10_us()`) - Sicherstellen, dass SS erst nach letzter Clock-Flanke hochgeht
8. **SS high setzen** (`set_ss_pin_high()`) - Slave deaktivieren
9. **Empfangenes Byte zurueckgeben**

### Relevante Status-Flags (SPI1->SR):
| Flag | Bit | Bedeutung                          |
|------|-----|------------------------------------|
| TXE  | 1   | Transmit buffer empty              |
| RXNE | 0   | Receive buffer not empty           |
| BSY  | 7   | SPI busy (Transfer am Laufen)      |

## 5.3 Testen - Loopback-Test

### Frage: Wie wird die Funktion getestet?

**Antwort:**
- MOSI (Pin 8 auf P5) und MISO (Pin 7 auf P5) mit Drahtbruecke verbinden (Loopback)
- Display darf NICHT angeschlossen sein
- Das Testprogramm (`test.c`) liest DIP-Switches S7-S0, zeigt den Wert auf LED7-LED0, sendet ihn via SPI und zeigt den empfangenen Wert auf LED23-LED16
- Bei korrektem Loopback: LED7-LED0 und LED23-LED16 muessen identisch sein
- Zusaetzlich: SPI-Signale (MOSI, CLK, SS) mit Oszilloskop/Logic Analyzer am Port 5 messen
