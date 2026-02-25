# CLAUDE.md - Project Guidelines

## Project Overview
CT2 (Computertechnik 2) repository for ZHAW coursework. Contains embedded C projects targeting the CT-Board (STM32-based).

## Repository Structure
- Each week is a separate directory: `Woche N <lab_name>/`
- `app/` contains the application source code
- `RTE/` contains the runtime environment and HAL
- `build/` contains build artifacts

## Language & Conventions
- All code is written in C (embedded, bare-metal)
- Coding style follows ZHAW InES standards (see `coding_style.pdf`)
- Module naming: `module_name.c` / `module_name.h`
- Function naming: `module_name_action()` (e.g., `counter_increment()`)
- Constants: `UPPER_CASE` (e.g., `NR_OF_DICE_VALUES`)
- Static module-level variables for encapsulation
- German is used for documentation and commit messages

## Build System
- Keil µVision project files (`.uvprojx`, `.uvoptx`, `.uvguix.*`)
- Do NOT modify Keil project files unless explicitly asked

## Important Notes
- Do not commit `.idea/` or other IDE-specific directories
- Binary build artifacts in `build/` should generally not be committed
- PDF lab sheets (`coding_style.pdf` etc.) are versioned for reference