# CT2 - Computertechnik 2

Repository für das Modul Computertechnik 2 (CT2) an der ZHAW (Zürcher Hochschule für Angewandte Wissenschaften).

## Inhalt

### Woche 1 - Coding Style
Elektronische Würfel-Applikation (Electronic Dice) für das CT-Board. Die Applikation demonstriert:
- Modularisierung in C (Module: `counter`, `display`, `statistics`, `button_state`)
- Coding-Style-Richtlinien nach ZHAW InES Standard
- UML-Moduldiagramm der Applikationsarchitektur

## Projektstruktur

```
CT2/
├── Woche 1 lab_coding_style/
│   ├── app/            # Applikationscode (main, Module)
│   ├── RTE/            # Runtime Environment (HAL, Device)
│   ├── build/          # Build-Konfiguration
│   └── uml_diagram.puml  # UML-Diagramm (PlantUML)
```

## Toolchain

- Keil µVision (ARM Compiler)
- CT-Board (STM32-basiert)