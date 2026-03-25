# Woche 4 — SPI (Serial Peripheral Interface)

## Überblick

In dieser Woche wurde die SPI-Schnittstelle des STM32F429ZI konfiguriert und zur Kommunikation mit einem TFT-Display (EA eDIPTFT43-ATP) vorbereitet. Es wurden die Initialisierung des SPI1-Moduls über das CR1-Register, die Implementierung einer Sende-/Empfangsfunktion mit Polling der Status-Flags (TXE, RXNE, BSY) und ein Loopback-Test zur Verifikation umgesetzt. Das Display selbst wird erst im nächsten Praktikum angeschlossen.

## Lernziele

- SPI-Timing-Optionen (CPOL, CPHA, Prescaler) verstehen und auf dem STM32F4 konfigurieren
- Einzelne Bytes über SPI senden und empfangen (Full-Duplex)
- Übertragene SPI-Daten mithilfe des Oszilloskops / Logic Analyzers analysieren
- Zusammenspiel von GPIO (Alternate Function, Software Slave Select) und SPI-Peripherie verstehen

## Kernkonzepte

### SPI-Protokoll

SPI (Serial Peripheral Interface) ist eine **synchrone, serielle Vollduplex-Schnittstelle**. Der Master erzeugt den Takt und steuert die Kommunikation.

**Vier Leitungen:**

| Signal | Richtung (Master) | Funktion |
|--------|--------------------|----------|
| MOSI | Output | Master Out, Slave In — Daten senden |
| MISO | Input | Master In, Slave Out — Daten empfangen |
| SCK | Output | Serial Clock — Takt vom Master |
| $\overline{SS}$ (NSS) | Output | Slave Select — aktiv LOW |

**Vollduplex** bedeutet: Senden und Empfangen geschehen **gleichzeitig** über MOSI und MISO. Für jedes gesendete Byte wird automatisch auch ein Byte empfangen.

### SPI-Modi (CPOL / CPHA)

Die Kombination von **CPOL** (Clock Polarity) und **CPHA** (Clock Phase) definiert den SPI-Modus:

| Modus | CPOL | CPHA | CLK Idle | Daten gültig bei |
|-------|------|------|----------|------------------|
| 0 | 0 | 0 | LOW | 1. Flanke (steigend) |
| 1 | 0 | 1 | LOW | 2. Flanke (fallend) |
| 2 | 1 | 0 | HIGH | 1. Flanke (fallend) |
| 3 | 1 | 1 | HIGH | 2. Flanke (steigend) |

Das Display erfordert **SPI Mode 0** (CPOL=0, CPHA=0): Clock ruht auf LOW, Daten werden bei der **steigenden Flanke** gelesen, MSB wird zuerst übertragen.

### CR1-Register — Konfiguration im Detail

Das Kontrollregister `SPI1->CR1` steuert sämtliche SPI-Parameter. Für das Display-Interface:

```
Bit: 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
      0  0  0  0  0  0  1  1  0  1  1  1  1  1  0  0
                       SSM SSI   SPE  BR=111  MSTR
```

- Vor SPE: **`0x033C`** (MSTR + BR=111 + SSM + SSI)
- Mit SPE: **`0x037C`** (+ SPE)

**Wichtig:** SPE muss als **letztes** gesetzt werden — ab diesem Moment sind alle Settings „scharf".

### Prescaler-Berechnung

Die SPI-Taktfrequenz wird aus dem Peripheral Clock abgeleitet:

$$f_{SPI} = \frac{f_{PCLK}}{Prescaler}$$

SPI1 hängt am APB2-Bus ($f_{PCLK} = 42\,\text{MHz}$). Das Display erlaubt maximal 200 kHz:

$$\frac{42\,\text{MHz}}{200\,\text{kHz}} = 210 \quad \Rightarrow \quad \text{nächster Prescaler: } 256 \quad \Rightarrow \quad f_{SPI} = \frac{42\,\text{MHz}}{256} = 164\,\text{kHz}$$

Der Prescaler /128 (328 kHz) würde die 200 kHz Grenze überschreiten, daher BR[2:0] = `111` (/256).

### Software Slave Select (SSM/SSI)

Statt den NSS-Pin per Hardware steuern zu lassen, wird SS **manuell per GPIO** (PA4) kontrolliert:

- **SSM = 1**: Software Slave Management — der physische NSS-Pin wird intern ignoriert
- **SSI = 1**: Internal Slave Select auf HIGH — verhindert einen Master Mode Fault (MODF)
- **GPIO PA4**: Als normaler Output konfiguriert (nicht AF), gesteuert über `GPIOA->BSRR`

```c
// SS low (Slave aktivieren):  Bit 4 im Reset-Teil des BSRR
GPIOA->BSRR = 0x00100000;

// SS high (Slave deaktivieren): Bit 4 im Set-Teil des BSRR
GPIOA->BSRR = 0x00000010;
```

### Sende-/Empfangsablauf (Polling)

Der Full-Duplex-Transfer eines einzelnen Bytes folgt einem festen Ablauf mit Polling der Status-Flags im `SPI1->SR`:

1. $\overline{SS}$ low setzen → Slave aktivieren
2. Warten auf **TXE = 1** → Tx-Buffer ist leer
3. Byte in `SPI1->DR` schreiben → startet automatisch die Clock-Erzeugung
4. Warten auf **RXNE = 1** → Empfangsdaten bereit
5. `SPI1->DR` lesen → empfangenes Byte holen (löscht RXNE)
6. Warten auf **BSY = 0** → Transfer komplett abgeschlossen
7. 10 µs warten → SS erst nach letzter Clock-Flanke hochsetzen
8. $\overline{SS}$ high setzen → Slave deaktivieren

**Was passiert intern beim Schreiben in DR?**
1. Byte → Tx-Buffer → Shift-Register (TXE wird wieder 1)
2. Shift-Register schiebt Bits seriell auf MOSI heraus
3. **Gleichzeitig** werden Bits von MISO ins Shift-Register hineingeschoben
4. Nach 8 Taktzyklen: Shift-Register → Rx-Buffer (RXNE wird 1)

### GPIO im Alternate Function Mode

Die SPI-Pins (SCK, MISO, MOSI) werden nicht als normale GPIO-Ausgänge konfiguriert, sondern im **Alternate Function Mode** (`MODER = 10`). Dies übergibt die Pin-Steuerung an die SPI-Peripherie:

| Port-Pin | CT-Board | SPI-Funktion | GPIO-Modus |
|----------|----------|--------------|------------|
| PA4 | P5.5 | NSS (SS) | Output (manuell via GPIO) |
| PA5 | P5.6 | SCK | Alternate Function 5 |
| PA6 | P5.7 | MISO | Alternate Function 5 |
| PA7 | P5.8 | MOSI | Alternate Function 5 |
| PA8 | P5.9 | SBUF | Input |

**AF5** = SPI1 auf dem STM32F429 (laut Datasheet). PA4 wird bewusst **nicht** als AF konfiguriert, da SS per Software gesteuert wird (SSM=1).

## Aufgaben und Umsetzung

### Aufgabe 1: SPI-Initialisierung (`hal_spi_init`)

- **Ziel:** SPI1 als Master konfigurieren mit Mode 0, 164 kHz Clock, Software Slave Select
- **Umsetzung:** GPIO-Konfiguration war vorgegeben (PA5–PA7 als AF5, PA4 als Output). Studentencode konfiguriert `SPI1->CR1` mit Prescaler, Master, SSM/SSI und aktiviert SPE als letztes.
- **Schlüssel-Code:**
  ```c
  SPI1->CR1 |= (0x7 << 3);   // BR[2:0] = 111 --> fPCLK/256 = 164 kHz
  SPI1->CR1 |= (0x1 << 2);   // MSTR = 1     --> Master mode
  SPI1->CR1 |= (0x1 << 9);   // SSM = 1      --> Software slave management
  SPI1->CR1 |= (0x1 << 8);   // SSI = 1      --> Internal slave select high
  SPI1->CR1 |= (0x1 << 6);   // SPE = 1      --> SPI enable (set last)
  ```

### Aufgabe 2: Sende-/Empfangsfunktion (`hal_spi_read_write`)

- **Ziel:** Ein Byte senden und gleichzeitig ein Byte empfangen (Full-Duplex)
- **Umsetzung:** Polling-basierter Ablauf: SS low → TXE warten → DR schreiben → RXNE warten → DR lesen → BSY warten → 10 µs Pause → SS high
- **Schlüssel-Code:**
  ```c
  set_ss_pin_low();
  while (!(SPI1->SR & BIT_TXE)) {}    // wait until Tx buffer empty
  SPI1->DR = send_byte;                // write byte to data register
  while (!(SPI1->SR & BIT_RXNE)) {}   // wait until Rx buffer not empty
  rec_byte = SPI1->DR;                 // read received byte
  while (SPI1->SR & BIT_BSY) {}        // wait until SPI not busy
  wait_10_us();
  set_ss_pin_high();
  return rec_byte;
  ```

### Aufgabe 3: Loopback-Test

- **Ziel:** Sende-/Empfangsfunktion verifizieren und SPI-Signale am Oszilloskop analysieren
- **Umsetzung:** MOSI (Pin 8) und MISO (Pin 7) auf Port 5 mit Drahtbrücke verbinden. Display darf **nicht** angeschlossen sein.
- **Ergebnis:** Testprogramm liest DIP Switches S7–S0, sendet den Wert via SPI und zeigt den empfangenen Wert auf LED23–LED16. Bei korrektem Loopback: LED7–LED0 = LED23–LED16.

## Wichtige Register / Hardware

| Register | Adresse/Pfad | Relevante Bits | Funktion |
|----------|-------------|----------------|----------|
| `SPI1->CR1` | Offset `0x00` | Bits [15:0] | Konfiguration: Mode, Prescaler, Master, SSM, SPE |
| `SPI1->CR2` | Offset `0x04` | — (auf `0x0000`) | Interrupts und DMA disabled |
| `SPI1->SR` | Offset `0x08` | TXE (Bit 1), RXNE (Bit 0), BSY (Bit 7) | Status-Flags für Polling |
| `SPI1->DR` | Offset `0x0C` | Bits [7:0] bei 8-Bit | Datenregister (Senden und Empfangen) |
| `RCC->APB2ENR` | — | Bit 12 (SPI1EN) | Clock für SPI1 aktivieren |
| `RCC->AHB1ENR` | — | Bit 0 (GPIOAEN) | Clock für GPIOA aktivieren |
| `GPIOA->MODER` | `0x40020000` | Bits [17:8] für PA4–PA8 | AF für PA5–PA7, Output für PA4, Input für PA8 |
| `GPIOA->AFRL` | `0x40020020` | Bits [31:20] für PA5–PA7 | AF5 = SPI1 |
| `GPIOA->BSRR` | `0x40020018` | Bit 4 (Set), Bit 20 (Reset) | SS-Pin (PA4) manuell steuern |

## Schlussfolgerungen

- **SPI ist Vollduplex**: Für jedes gesendete Byte wird automatisch ein Byte empfangen — auch wenn der Slave keine sinnvollen Daten sendet. Das Shift-Register arbeitet in beide Richtungen gleichzeitig.
- **Prescaler korrekt berechnen**: Die SPI-Clock ergibt sich aus $f_{PCLK} / Prescaler$. Es muss immer der nächstgrössere Prescaler gewählt werden, der die maximale Frequenz des Slaves nicht überschreitet. Im Zweifel langsamer takten.
- **SPE immer zuletzt setzen**: Alle Konfigurationsbits (Mode, Prescaler, Master, SSM/SSI) müssen vor dem Aktivieren der SPI-Peripherie gesetzt sein. SPE macht alle Settings „scharf".
- **Software Slave Select erfordert SSI=1**: Ohne SSI=1 interpretiert der SPI-Controller den intern auf LOW liegenden NSS-Wert als Angriff eines anderen Masters und löst einen MODF-Fehler aus.
- **Polling-Reihenfolge ist kritisch**: TXE → Write DR → RXNE → Read DR → BSY=0. Diese Reihenfolge entspricht dem Reference Manual und stellt sicher, dass keine Daten verloren gehen und der Transfer vollständig abgeschlossen ist.
- **GPIO Alternate Function**: SPI-Pins (SCK, MISO, MOSI) werden im AF-Modus betrieben — die GPIO-Steuerung wird an die SPI-Peripherie übergeben. Nur der SS-Pin bleibt ein normaler GPIO-Output, da er per Software gesteuert wird.

## Verwandte Themen

- **Woche 3 (GPIO):** GPIO-Konfiguration über MODER, OTYPER, OSPEEDR, PUPDR — hier werden dieselben Register für die SPI-Pin-Konfiguration im Alternate Function Mode verwendet
- **Woche 2 (Bus Cycles):** Volatile Pointer und Memory-Mapped I/O — der SPI-Registerzugriff folgt demselben Prinzip
- **Woche 5 (SPI Part 2):** Aufbauend auf dieser Initialisierung wird das Display tatsächlich angeschlossen und angesteuert
