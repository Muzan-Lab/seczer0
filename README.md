# seczer0

**seczer0** — a clone / firmware fork based on the Flipper Zero *concept* but adapted to run on **ESP32‑based** hardware. This repository contains a minimal Flipper‑style firmware and experimental modules for ESP32 devices, packaged for easy building with **PlatformIO**.

> ⚠️ Note: This project targets ESP32 development boards or custom ESP32 hardware that mimics Flipper‑style peripherals. It is **not** the official Flipper Zero firmware. Use responsibly and only on hardware you own.

---

## Table of Contents

- [What is this](#what-is-this)
- [Features](#features)
- [Supported Hardware / Compatibility](#supported-hardware--compatibility)
- [Project Structure](#project-structure)
- [Requirements](#requirements)
- [Installation & Build](#installation--build)
- [Flashing / Upload](#flashing--upload)
- [Usage](#usage)
- [Developing / Extending](#developing--extending)
- [Contributing](#contributing)
- [License](#license)
- [Disclaimer & Safety](#disclaimer--safety)

---

## What is this

A lightweight Flipper‑style firmware port for ESP32. The goal is to provide core modules and an approachable PlatformIO setup so researchers and makers can experiment with Flipper‑like functionality on ESP32 dev boards (e.g. ESP32‑S2, ESP32‑C3, or similar).

This repo is intended as a **learning / experimental** project — not a production replacement for the official Flipper firmware.

---

## Features

- Core framework and example modules (IR, GPIO helpers, simple UI placeholders)
- PlatformIO build configuration for common ESP32 environments
- Example source in `src/` and public headers in `include/`
- Fast iteration cycle: `platformio run` → `platformio upload`

---

## Supported Hardware / Compatibility

- ESP32 development boards (ESP32, ESP32‑S2, ESP32‑C3) — confirm pinouts and peripherals before flashing.
- Custom ESP32 boards that expose required peripherals (GPIO, I2C/SPI, display if used).

> If your hardware differs from the example `platformio.ini` environment, edit `platformio.ini` to match your board's `board` value.

---

## Project Structure

```
seczer0/
├── include/         ← public headers
├── src/             ← firmware source code and modules
├── .vscode/         ← editor settings (optional)
├── platformio.ini   ← PlatformIO build & envs
└── .gitignore
```

---

## Requirements

- [PlatformIO](https://platformio.org/) (recommended plugin for VSCode or `pip install platformio`)
- ESP32 development board and USB cable
- Serial driver for your board (CP210x / CH340 / etc.) if required
- Basic C/C++ toolchain (PlatformIO will install what it needs)

---

## Installation & Build

1. Clone the repository:

```bash
git clone https://github.com/Muzan-Lab/seczer0.git
cd seczer0
```

2. Inspect `platformio.ini` and choose the environment that matches your board. Example envs might be `esp32dev` or `esp32-s2`.

3. Build the firmware:

```bash
platformio run
```

If the build fails, check `platformio.ini` for the correct `platform`, `board`, and `framework` values.

---

## Flashing / Upload

Use PlatformIO to upload the firmware to your board (it will use the serial port automatically):

```bash
platformio run --target upload
```

Or choose a specific environment:

```bash
platformio run -e esp32dev --target upload
```

If you prefer `esptool.py` or another uploader, build first to produce the `.bin` files and then follow your normal flashing flow.

---

## Usage

- After flashing, connect to the board's serial console to see boot messages:

```bash
platformio device monitor
```

- The firmware includes example modules in `src/`. Interact via serial commands or through any UI the firmware exposes (depends on which modules are enabled).

---

## Developing / Extending

- Add new modules in `src/` and expose headers in `include/`.
- Keep modules modular and document public APIs in header files.
- Use `platformio run` frequently to catch compile errors early.
- If you add third‑party libraries, declare them in `platformio.ini` under `lib_deps`.

Example `platformio.ini` snippet you can adapt:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
  # list libraries here, e.g. adafruit/Adafruit BusIO

; change board/platform depending on your hardware
```

---

## Contributing

1. Fork the repository
2. Create a branch (`git checkout -b feature/my-feature`)
3. Commit with clear messages
4. Push and open a Pull Request

Please include concise descriptions of added modules, required hardware, and wiring diagrams when applicable.

---

## License

This project is licensed under the MIT License — see the [LICENSE](./LICENSE) file for details.


---

## Disclaimer & Safety

This project is provided for **educational and research** purposes only. The maintainers are not responsible for any misuse, damage, or legal issues arising from use of this firmware. Always follow laws and best practices when performing security research.

---

## Contact

- Muzan‑Lab — maintainers / owner
- For questions: open an Issue on the repository
