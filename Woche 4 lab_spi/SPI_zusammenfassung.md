# SPI Reference Manual Zusammenfassung (STM32F4)

Relevante Auszuege aus dem RM0090 Reference Manual fuer das Lab SPI Part 1.

---

## 1. Was ist SPI?

SPI (Serial Peripheral Interface) ist eine **synchrone, serielle Vollduplex-Schnittstelle**.
Der STM32F4 kann als Master oder Slave konfiguriert werden.

**4 Leitungen** verbinden Master und Slave:

| Signal | Richtung (Master) | Funktion                            |
|--------|--------------------|-------------------------------------|
| MOSI   | Output             | Master Out, Slave In (Daten senden) |
| MISO   | Input              | Master In, Slave Out (Daten empfangen) |
| SCK    | Output             | Serial Clock (Takt vom Master)      |
| NSS    | Output (SW)        | Slave Select (aktiv LOW)            |

**Vollduplex** bedeutet: Senden und Empfangen geschieht **gleichzeitig** ueber MOSI und MISO.
Fuer jedes gesendete Byte wird automatisch auch ein Byte empfangen.

---

## 2. SPI-Modi (CPOL / CPHA)

Die Kombination von **CPOL** (Clock Polarity) und **CPHA** (Clock Phase) definiert den SPI-Modus.
Es gibt 4 moegliche Modi:

| Modus | CPOL | CPHA | CLK Idle | Daten gueltig bei      |
|-------|------|------|----------|------------------------|
| 0     | 0    | 0    | LOW      | 1. Flanke (steigend)   |
| 1     | 0    | 1    | LOW      | 2. Flanke (fallend)    |
| 2     | 1    | 0    | HIGH     | 1. Flanke (fallend)    |
| 3     | 1    | 1    | HIGH     | 2. Flanke (steigend)   |

### CPOL (Bit 1 in CR1) - Clock Polarity
- **CPOL = 0**: Clock ist im Ruhezustand LOW
- **CPOL = 1**: Clock ist im Ruhezustand HIGH

### CPHA (Bit 0 in CR1) - Clock Phase
- **CPHA = 0**: Daten werden bei der **ersten** Taktflanke uebernommen (Capture Strobe)
- **CPHA = 1**: Daten werden bei der **zweiten** Taktflanke uebernommen

### Fuer unser Display: **SPI Mode 0** (CPOL=0, CPHA=0)
- Clock ruht auf LOW
- Daten werden bei der **steigenden Flanke** gelesen
- MSB wird zuerst uebertragen

---

## 3. SPI Control Register 1 (SPI_CR1) - Bit fuer Bit

Adress-Offset: 0x00, Reset-Wert: 0x0000

```
Bit:  15    14    13    12    11   10    9    8    7     6    5  4  3    2     1     0
    BIDI  BIDI  CRC   CRC   DFF  RX   SSM  SSI  LSB   SPE  BR[2:0]  MSTR  CPOL  CPHA
    MODE   OE    EN   NEXT        ONLY            FIRST
```

### Bit 15: BIDIMODE - Bidirektionaler Datenmodus
- **0**: 2-line unidirectional (Standard: MOSI + MISO als separate Leitungen)
- 1: 1-line bidirectional (nur eine Datenleitung)

**Wir verwenden 0** → Zwei separate Datenleitungen (MOSI und MISO)

### Bit 14: BIDIOE - Output Enable im bidirektionalen Modus
- Nur relevant wenn BIDIMODE=1
- **Wir setzen 0** (irrelevant bei BIDIMODE=0)

### Bit 13: CRCEN - CRC Berechnung
- **0**: CRC deaktiviert
- 1: CRC aktiviert

**Wir verwenden 0** → Display unterstuetzt kein CRC

### Bit 12: CRCNEXT - Naechster Transfer ist CRC
- **Wir setzen 0** (kein CRC)

### Bit 11: DFF - Datenrahmenformat
- **0**: 8-Bit Datenrahmen
- 1: 16-Bit Datenrahmen

**Wir verwenden 0** → 8-Bit pro Transfer

### Bit 10: RXONLY - Nur-Empfangsmodus
- **0**: Full-Duplex (Senden und Empfangen gleichzeitig)
- 1: Nur Empfangen (Transmitter deaktiviert)

**Wir verwenden 0** → Full-Duplex

### Bit 9: SSM - Software Slave Management
- 0: Hardware-NSS (NSS-Pin steuert Slave-Auswahl)
- **1**: Software-NSS (SSI-Bit ersetzt den NSS-Pin intern)

**Wir verwenden 1** → Wir steuern SS manuell ueber GPIO PA4

### Bit 8: SSI - Internal Slave Select
- Nur wirksam wenn SSM=1
- Dieser Wert wird intern als NSS-Pegel verwendet
- **1**: NSS intern HIGH → kein MODF (Master Mode Fault)

**Wir verwenden 1** → Verhindert MODF-Fehler. Wenn SSI=0 waere, wuerde der
SPI-Controller denken, ein anderer Master zieht NSS low → Master Mode Fault!

### Bit 7: LSBFIRST - Bit-Reihenfolge
- **0**: MSB zuerst (Most Significant Bit)
- 1: LSB zuerst (Least Significant Bit)

**Wir verwenden 0** → MSB first (wie vom Display gefordert, DORD=0)

### Bit 6: SPE - SPI Enable
- 0: SPI deaktiviert
- **1**: SPI aktiviert

**WICHTIG**: SPE muss als **letztes** gesetzt werden, nachdem alle anderen Bits konfiguriert sind!
Ab dem Moment wo SPE=1 ist, sind alle Settings "scharf".

### Bits 5:3: BR[2:0] - Baud Rate Prescaler

| BR[2:0] | Teiler | Bei fPCLK=42 MHz |
|---------|--------|-------------------|
| 000     | /2     | 21 MHz            |
| 001     | /4     | 10.5 MHz          |
| 010     | /8     | 5.25 MHz          |
| 011     | /16    | 2.625 MHz         |
| 100     | /32    | 1.3125 MHz        |
| 101     | /64    | 656.25 kHz        |
| 110     | /128   | 328.125 kHz       |
| **111** | **/256** | **164.0625 kHz** |

**Wir verwenden 111** → 42 MHz / 256 = 164 kHz < 200 kHz (Display-Limit)

Warum nicht /128 (328 kHz)? → Wuerde die 200 kHz Grenze des Displays ueberschreiten!

### Bit 2: MSTR - Master/Slave Auswahl
- 0: Slave
- **1**: Master

**Wir verwenden 1** → Der STM32 ist der Master, das Display ist der Slave

### Bit 1: CPOL - Clock Polarity
**Wir verwenden 0** → Clock idle = LOW (SPI Mode 0)

### Bit 0: CPHA - Clock Phase
**Wir verwenden 0** → Daten gueltig bei 1. Flanke (SPI Mode 0)

### Zusammenfassung CR1-Wert

```
Bit: 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
      0  0  0  0  0  0  1  1  0  1  1  1  1  1  0  0
                        SSM SSI   SPE  BR=111  MSTR
```

- Vor SPE: **0x033C** = MSTR + BR=111 + SSM + SSI
- Mit SPE: **0x037C** = 0x033C + SPE

---

## 4. SPI Status Register (SPI_SR)

Adress-Offset: 0x08, Reset-Wert: 0x0002

```
Bit:  7     6     5     4     3      2     1     0
     BSY   OVR   MODF  CRC   UDR   CHSIDE TXE   RXNE
                        ERR
```

### Die drei fuer uns relevanten Flags:

| Flag | Bit | Bedeutung | Wann gesetzt? | Wann geloescht? |
|------|-----|-----------|---------------|-----------------|
| **TXE** | 1 | Tx Buffer Empty | Hardware: Daten vom Tx-Buffer ins Shift-Register kopiert | Software: Schreiben in SPI_DR |
| **RXNE** | 0 | Rx Buffer Not Empty | Hardware: Empfangene Daten vom Shift-Register in Rx-Buffer | Software: Lesen von SPI_DR |
| **BSY** | 7 | SPI Busy | Hardware: Transfer laeuft | Hardware: Transfer abgeschlossen |

### Vordefinierte Bitmasks im Code:
```c
#define BIT_TXE  (uint32_t)0x00000002   // Bit 1
#define BIT_RXNE (uint32_t)0x00000001   // Bit 0
#define BIT_BSY  (uint32_t)0x00000080   // Bit 7
```

**Wichtig**: TXE ist nach dem Reset = 1 (Buffer ist leer), RXNE = 0 (nichts empfangen).

---

## 5. SPI Data Register (SPI_DR)

- **Schreiben** in SPI_DR → Daten gehen in den Tx-Buffer → werden gesendet
- **Lesen** von SPI_DR → Daten kommen aus dem Rx-Buffer → empfangenes Byte

Dasselbe Register fuer Senden UND Empfangen (intern getrennte Buffer).

---

## 6. Master-Konfiguration: Schritt fuer Schritt (RM S.861)

Laut Reference Manual muss der SPI Master so konfiguriert werden:

1. **BR[2:0]** setzen → Baud Rate / Prescaler waehlen
2. **CPOL und CPHA** setzen → Timing-Modus definieren
3. **DFF** setzen → 8-Bit oder 16-Bit Datenformat
4. **LSBFIRST** konfigurieren → MSB oder LSB first
5. **SSM und SSI** setzen (bei Software-NSS) → NSS intern hochhalten
6. **MSTR** setzen → Master-Modus
7. **SPE** setzen → SPI aktivieren (als Letztes!)

---

## 7. Sende-/Empfangsablauf Full-Duplex Master (RM S.864-866)

### Einzelbyte-Transfer (unser Fall):

```
1. SS low setzen          (Slave aktivieren via GPIO)
2. Warten bis TXE = 1    (Tx-Buffer bereit)
3. Byte in SPI_DR         (startet automatisch die Clock-Erzeugung)
4. Warten bis RXNE = 1   (Empfang abgeschlossen)
5. SPI_DR lesen           (empfangenes Byte holen, loescht RXNE)
6. Warten bis BSY = 0    (Transfer komplett abgeschlossen)
7. 10 us warten           (SS erst nach letzter Clock-Flanke hochsetzen)
8. SS high setzen         (Slave deaktivieren)
```

### Was passiert intern bei Schritt 3 (Byte schreiben)?
1. Das Byte wird in den **Tx-Buffer** geschrieben
2. Der Tx-Buffer wird ins **Shift-Register** geladen (TXE wird wieder 1)
3. Das Shift-Register schiebt die Bits seriell auf **MOSI** heraus
4. **Gleichzeitig** werden Bits von **MISO** ins Shift-Register hineingeschoben
5. Nach 8 Taktzyklen: Shift-Register → **Rx-Buffer** (RXNE wird 1)

### Warum warten wir auf BSY=0?
BSY zeigt an, dass die SPI-Hardware noch aktiv ist. Erst wenn BSY=0, ist
die letzte Clock-Flanke vorbei und der Transfer wirklich beendet.

### Warum wait_10_us() vor SS high?
Das Display braucht eine kurze Verzoegerung zwischen der letzten Clock-Flanke
und dem Hochsetzen von SS. Ohne dieses Warten koennte SS zu frueh hochgehen
und das letzte Bit wuerde vom Display nicht korrekt uebernommen.

---

## 8. NSS / Slave Select Management

Zwei Optionen:

### Hardware NSS (SSM=0)
- NSS-Pin wird automatisch gesteuert
- Geht LOW wenn SPI aktiv, HIGH wenn SPI inaktiv

### Software NSS (SSM=1) ← **Unser Fall**
- Der NSS-Pin ist frei verfuegbar (wird als GPIO genutzt)
- Das **SSI-Bit** ersetzt intern den NSS-Pin-Wert
- **SSI muss 1 sein**, damit kein Master Mode Fault (MODF) entsteht
- Wir steuern SS manuell ueber PA4 mit `set_ss_pin_low()` / `set_ss_pin_high()`

Die Funktionen verwenden das BSRR-Register (Bit Set/Reset Register) des GPIO:
- `GPIOA->BSRR = 0x00100000` → Bit 4 im Reset-Teil → PA4 = LOW (SS aktiv)
- `GPIOA->BSRR = 0x00000010` → Bit 4 im Set-Teil → PA4 = HIGH (SS inaktiv)

---

## 9. GPIO-Konfiguration fuer SPI (bereits im Code gegeben)

| Port-Pin | CT-Board | SPI-Funktion | GPIO-Modus          |
|----------|----------|--------------|---------------------|
| PA4      | P5.5     | NSS (SS)     | Output (manuell)    |
| PA5      | P5.6     | SCK          | Alternate Function 5|
| PA6      | P5.7     | MISO         | Alternate Function 5|
| PA7      | P5.8     | MOSI         | Alternate Function 5|
| PA8      | P5.9     | SBUF         | Input               |

- **AF5** = SPI1 auf dem STM32F429 (laut Datasheet)
- PA4 wird NICHT als AF konfiguriert, sondern als normaler GPIO Output,
  weil wir SS per Software steuern (SSM=1)

---

## 10. Clock-Konfiguration

```
                   APB2 Bus (max 84 MHz)
System Clock ──→ AHB ──→ APB2 Prescaler ──→ SPI1
  168 MHz                    /2              42 MHz (fPCLK)

SPI Clock = fPCLK / Prescaler = 42 MHz / 256 = 164 kHz
```

- **RCC->APB2ENR |= 0x00001000**: Aktiviert den Clock fuer SPI1 (Bit 12 = SPI1EN)
- **RCC->AHB1ENR |= 0x00000001**: Aktiviert den Clock fuer GPIOA (Bit 0 = GPIOAEN)

Ohne diese Clock-Aktivierung wuerden die Register von SPI1/GPIOA nicht reagieren.

---

## 11. Loopback-Test (Aufgabe 5.3)

Fuer den Test wird **MOSI direkt mit MISO** verbunden (Drahtbruecke Pin 7 ↔ Pin 8 auf P5).

**Erwartetes Verhalten**: Alles was gesendet wird, kommt sofort wieder zurueck.
→ LED7-LED0 (gesendeter Wert) und LED23-LED16 (empfangener Wert) muessen identisch sein.

**Achtung**: Display darf beim Loopback-Test NICHT angeschlossen sein (Kurzschluss-Gefahr).

---

## 12. Zusammenfassung: Warum jedes Bit so gesetzt wird

| Einstellung     | Wert | Grund                                          |
|-----------------|------|-------------------------------------------------|
| CPOL=0, CPHA=0  | 0, 0 | Display erfordert SPI Mode 0                    |
| MSTR=1          | 1    | STM32 ist Master, Display ist Slave             |
| BR=111          | 111  | 164 kHz < 200 kHz Display-Maximum               |
| DFF=0           | 0    | 8-Bit Uebertragung (1 Byte pro Transfer)        |
| LSBFIRST=0      | 0    | MSB first (Display-Anforderung)                 |
| SSM=1           | 1    | SS wird per Software/GPIO gesteuert             |
| SSI=1           | 1    | Internes NSS=HIGH, verhindert MODF              |
| BIDIMODE=0      | 0    | Zwei separate Datenleitungen (MOSI+MISO)        |
| RXONLY=0        | 0    | Full-Duplex (nicht nur Empfang)                 |
| CRCEN=0         | 0    | Kein CRC (Display unterstuetzt es nicht)        |
| SPE=1           | 1    | SPI aktivieren (immer als Letztes!)             |
