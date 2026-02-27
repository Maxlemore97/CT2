# CLAUDE.md - Project Guidelines

## Project Overview
CT2 (Computertechnik 2) repository for ZHAW coursework. Contains embedded C projects targeting the CT-Board (STM32F429ZI).

## Repository Structure
- Each week is a separate directory: `Woche N <lab_name>/`
- `app/` contains the application source code
- `RTE/` contains the runtime environment and HAL
- `build/` contains build artifacts
- PDF lab sheets are versioned for reference
- README.md contains a summary of each week's lecture topics and lab work

## CT-Board Hardware
- MCU: STM32F429ZI (ARM Cortex-M4 with NVIC)
- External bus: 16-bit synchronous data bus via FMC (Flexible Memory Controller)
- Peripherals accessible via memory-mapped I/O (base addresses: LEDs at 0x60000100, DIP switches at 0x60000200)
- CPLD handles GPIO, switches, buttons, LEDs, 7-segment displays
- CT-Board must be operated in Modus 3 for external bus access

## Language & Conventions
- All code is written in C (embedded, bare-metal)
- Coding style follows ZHAW InES standards (see `coding_style.pdf`)
- Module naming: `module_name.c` / `module_name.h`
- Function naming: `module_name_action()` (e.g., `counter_increment()`)
- Constants: `UPPER_CASE` (e.g., `NR_OF_DICE_VALUES`)
- Static module-level variables for encapsulation
- Header files: only declarations, macros, re-definition guards — no definitions, no variable allocations
- German is used for documentation and commit messages

## Code Patterns
- Memory-mapped I/O via volatile pointer casts: `(*((volatile uint16_t *) 0x60000100))`
- Student code goes between `/// STUDENTS: To be programmed` and `/// END: To be programmed` markers
- Access modes controlled via `#define` / `#ifdef` preprocessor switches

## Build System
- Keil µVision project files (`.uvprojx`, `.uvoptx`, `.uvguix.*`)
- Do NOT modify Keil project files unless explicitly asked

## Commits
- Commit-Messages auf Deutsch
- Claude NIEMALS als Co-Autor oder Co-Contributor aufführen (kein `Co-Authored-By` oder ähnliches)

## Important Notes
- Do not commit `.idea/`, `.DS_Store` or other IDE-specific files
- Binary build artifacts in `build/` should generally not be committed