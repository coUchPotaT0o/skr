# SKR Mini E3 — Simple Motor-Speed Menu Firmware

Purpose: minimal PlatformIO firmware to let the printer's rotary encoder + 12864 screen set each motor (X/Y/Z/E) to a constant step frequency and direction.

Important notes:
- This is a prototype. You MUST update pin defines in `src/config.h` to match your SKR Mini E3 V3 board pinout before building.
- Update `board` in `platformio.ini` (`skr_mini_e3_v3` env) to the exact board id for your SKR Mini E3 V3 if available.
- Step pulse timing here is driven from the main loop; for production use move `motor_tick_isr()` to a hardware timer ISR for accurate high-frequency pulses.

Build & flash (options):

Local build (PlatformIO installed):

1. Edit `platformio.ini`, set `board` under `[env:skr_mini_e3_v3]` to your board id.
2. Edit `src/config.h` to set correct pins.
3. Build and upload with PlatformIO:

```powershell
platformio run -e skr_mini_e3_v3 --target upload
```

CI build (no local PlatformIO required):

1. Push this repository to GitHub (main or master branch) or trigger the workflow manually in Actions.
2. GitHub Actions will build the `skr_mini_e3_v3` environment and attach `firmware-skr_mini_e3_v3` as an artifact.
3. Download `firmware.bin` from the workflow run page (Actions -> latest run -> Artifacts).

Flashing the built binary:

- Via PlatformIO (recommended when available):

```powershell
platformio run -e skr_mini_e3_v3 --target upload
```

- Via STM32CubeProgrammer (SWD/ST-Link):

1. Connect an ST-Link or SWD programmer to the board.
2. Open STM32CubeProgrammer, connect to the target, and `Erasing & Programming` -> select `firmware.bin` and start.

- Via DFU/USB bootloader (if your board supports it): follow your board's DFU flashing steps.

STM32CubeProgrammer (CLI) example

If you have an ST-Link (or compatible) and STM32CubeProgrammer installed, you can flash the built `firmware.bin` from the command line. Run these from a PowerShell prompt (replace the path to `STM32_Programmer_CLI.exe` if not on PATH):

```powershell
# Erase full chip
STM32_Programmer_CLI.exe -c port=SWD -e all

# Write the binary to flash (0x08000000 is the common flash start - confirm for your MCU)
STM32_Programmer_CLI.exe -c port=SWD -w .pio/build/skr_mini_e3_v3/firmware.bin 0x08000000

# Verify written content
STM32_Programmer_CLI.exe -c port=SWD -v
```

Notes:
- Confirm the correct flash start address for your board's MCU before writing (commonly `0x08000000` for many STM32F1/F3 parts).
- If your programmer connects differently (USB or specific ST-Link index), adjust the `-c` option accordingly (e.g., `-c port=USB1`).
- If `STM32_Programmer_CLI.exe` isn't on PATH, run it from the STM32CubeProgrammer install directory or give the full path to the executable.

Safety note: Do not run this on a printer connected to heaters/moving parts without testing step pulses at low frequency first.

Testing:
- With no motor attached, you can still use the encoder and screen to change displayed values.
- With motors attached, set a low frequency first and verify direction and step pulses with an oscilloscope or LED before running the printer.

Limitations & next steps:
- Move step pulse generation into a hardware timer for precise timing.
- Debounce encoder and support acceleration ramps if needed.
- Integrate with Marlin or the original firmware for safety interlocks.
