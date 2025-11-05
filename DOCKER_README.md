# Team 8 Circuiteers - Embedded Development Environment

This Docker-based development environment is configured for developing embedded applications using Zephyr RTOS. It supports multiple development boards including STM32WL55xx, Nordic nRF series, and other Zephyr-supported platforms.

**Note:** This setup uses the full Zephyr project with all modules for maximum compatibility and to avoid dependency issues.

## Prerequisites

- Docker and Docker Compose installed on your system
- USB access for programming and debugging embedded boards

## Quick Start

1. **Build the development image:**
   ```bash
   docker compose build
   ```

2. **Start the development container:**
   ```bash
   docker compose run --rm team8-circuiteers
   ```

3. **Inside the container, initialize Zephyr (first time only):** 
   ```bash
   west init -o=--filter=tree:0
   west update -o=--filter=tree:0
   # If update fails due to network issues, retry:
   west update --fetch-opt=--filter=tree:0
   west zephyr-export
   ```

4. **Build your application:**
   ```bash
   # For NUCLEO-WL55JC (default)
   west build -p always -b nucleo_wl55jc apps
   ```

5. **Flash to the board:**
   ```bash
   west flash
   ```

## Docker Commands

```bash
# Build the development image
docker compose build

# Start interactive development container
docker compose run --rm team8-circuiteers

# Connect to serial console (from host)
docker compose run --rm team8-circuiteers minicom -D /dev/ttyACM0 -b 115200

# List USB devices (from host)
docker compose run --rm team8-circuiteers lsusb
```

## Development Workflow

1. **Start the container:**
   ```bash
   docker compose run --rm team8-circuiteers
   ```

2. **Initialize Zephyr workspace (first time only):**
   ```bash
   west init -o=--filter=tree:0
   west update -o=--filter=tree:0
   # If update fails due to network issues, retry:
   west update --fetch-opt=--filter=tree:0
   west zephyr-export
   ```

3. **Build and develop:**
   ```bash
   # Build applications
   west build -p always -b nucleo_wl55jc apps --pristine

   # Flash to board
   west flash

   # Clean build
   rm -rf build

   # Debug (if supported by board)
   west debug
   ```

## Project Structure

After initialization, your workspace will contain:

```
iot-venture-f25-the-circuiteers/
├── apps/                     # Your application source code
│   ├── src/main.c           # Main application file
│   ├── prj.conf             # Project configuration
│   └── CMakeLists.txt       # Build configuration
├── .devcontainer/           # VS Code dev container configuration (if using)
├── dev/                     # Development files (bash history, etc.)
├── zephyr/                  # Zephyr RTOS source (auto-generated)
├── modules/                 # Zephyr modules (auto-generated)
├── .west/                   # West workspace metadata (auto-generated)
├── Dockerfile               # Docker image definition
├── compose.yaml             # Docker Compose configuration
└── DOCKER_README.md         # This file
```

**Note:** The `zephyr/`, `modules/`, and `.west/` directories are automatically created during `west init` and contain the complete Zephyr ecosystem.

## Troubleshooting

### Build Error: "No such file or directory" for STM32 device tree files

If you encounter errors like:
```
fatal error: st/wl/stm32wl55jcix-pinctrl.dtsi: No such file or directory
```

This means the STM32 HAL module wasn't fully downloaded. Fix it by running:
```bash
west update --fetch-opt=--filter=tree:0
```

### Network Timeout During west update

If `west update` fails due to network timeouts, simply retry the command:
```bash
west update --fetch-opt=--filter=tree:0
```

The update process is resumable and will continue from where it left off.



#### Bootloader flashing

First flash the bootloader
```
/Applications/STMicroelectronics/STM32Cube/STM32CubeProgrammer/STM32CubeProgrammer.app/Contents/MacOs/bin/STM32_Programmer_CLI \
  -c port=SWD mode=UR \
  -w ./build/mcuboot/zephyr/zephyr.hex \              
  -v \
  -rst
```

Then flash the signed application
```
/Applications/STMicroelectronics/STM32Cube/STM32CubeProgrammer/STM32CubeProgrammer.app/Contents/MacOs/bin/STM32_Programmer_CLI \
  -c port=SWD mode=UR \
  -w ./build/boot_with_keys/zephyr/zephyr.signed.hex \
  -v \
  -rst
```