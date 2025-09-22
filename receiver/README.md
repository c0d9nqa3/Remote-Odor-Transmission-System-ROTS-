# ROTS Receiver - STM32F407VET6

This directory contains the STM32F407VET6 based receiver firmware for the ROTS (Remote Odor Transmission System).

## Hardware Requirements

- STM32F407VET6 Development Board
- ESP8266 WiFi Module
- SSD1306 OLED Display (128x64)
- 5x Micro Peristaltic Pumps
- 5x Solenoid Valves
- 2x Cooling Fans
- 16MB SPI Flash (for recipe storage)

## Project Structure

```
receiver/
├── src/                    # Source code
│   ├── main.c             # Main application
│   ├── rots_receiver.h    # Main header file
│   ├── rots_communication.c/h    # ESP32 communication
│   ├── rots_actuator_control.c/h # Pump/valve control
│   ├── rots_recipe_manager.c/h   # Recipe management
│   ├── rots_display.c/h          # OLED display
│   └── rots_system_monitor.c/h   # System monitoring
├── config/                # Configuration files
│   └── rots_config.h      # System configuration
├── recipes/               # Recipe database
├── Makefile              # Build configuration
└── README.md             # This file
```

## Building the Project

### Prerequisites

- ARM GCC toolchain
- STM32CubeMX (for HAL library)
- OpenOCD (for flashing)
- Make

### Build Commands

```bash
# Build project
make all

# Flash to MCU
make flash

# Debug with GDB
make debug

# Clean build files
make clean
```

## Configuration

Edit `config/rots_config.h` to modify system parameters:

- Communication settings
- Actuator configuration
- Display settings
- Safety parameters

## Features

- **Real-time Communication**: Receives odor commands from ESP32 sender
- **Precise Control**: PWM-based pump speed control
- **Recipe Management**: Stores and manages odor recipes
- **Status Display**: OLED display for system status
- **Error Handling**: Comprehensive error logging and recovery
- **Safety Features**: Emergency stop and safety limits

## Pin Configuration

### ESP8266 Communication
- UART2 (PA2/PA3)
- Enable Pin: PA4

### Display
- I2C1 (PB6/PB7)
- Address: 0x3C

### Pumps (PWM Control)
- Pump 1: PA8 (TIM1_CH1)
- Pump 2: PA9 (TIM1_CH2)
- Pump 3: PA10 (TIM1_CH3)
- Pump 4: PB0 (TIM3_CH3)
- Pump 5: PB1 (TIM3_CH4)

### Valves (GPIO Control)
- Valve 1-5: PC0-PC4

### Fans (GPIO Control)
- Fan 1-2: PC5-PC6

## Communication Protocol

The receiver communicates with the ESP32 sender using a custom protocol:

### Message Format
```
[Start][Type][Data][Checksum]
```

### Message Types
- 0x01: Odor Command
- 0x02: Status Request
- 0x03: Recipe Update
- 0x04: System Config
- 0x05: Emergency Stop

## Development

### Adding New Features

1. Create new module files in `src/`
2. Add function prototypes to header files
3. Update `main.c` to include new modules
4. Update `Makefile` if needed

### Debugging

Use GDB with OpenOCD for debugging:

```bash
# Terminal 1: Start OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg

# Terminal 2: Start GDB
make debug
```

## License

This project is licensed under the MIT License - see the main project README for details.
