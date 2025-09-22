# Remote Odor Transmission System (ROTS)

A distributed IoT system for remote odor recognition and transmission using edge AI and multi-sensor fusion technology.

## Overview

The Remote Odor Transmission System (ROTS) implements digital olfaction technology through advanced sensor arrays, edge AI processing, and precise odor generation for remote scent transmission applications.

### Key Features

- **Edge AI Processing**: Real-time odor recognition on ESP32 with <500ms response time
- **Multi-Sensor Fusion**: 8-channel gas sensor array with environmental compensation
- **Remote Transmission**: Low-latency wireless communication (<200ms)
- **Precise Odor Generation**: 5-channel modular scent delivery system
- **Modular Design**: Scalable architecture for different applications

## Technical Specifications

| Parameter | Specification |
|-----------|---------------|
| Recognition Accuracy | >85% |
| Recognition Time | <500ms |
| Transmission Latency | <200ms |
| Supported Odor Types | ≥5 categories |
| Power Consumption | Sender: <5W, Receiver: <15W |
| Operating Range | WiFi coverage area |

## System Architecture

```
┌─────────────────┐    WiFi/Internet    ┌─────────────────┐
│   Sender (ESP32)│───────────────────▶│ Receiver (RPi4) │
│                 │                     │                 │
│  ├─Sensor Array │                     │  ├─Control Unit │
│  ├─Edge AI      │                     │  ├─Odor Generator│
│  └─Communication│◀───────────────────│  └─Status Display│
└─────────────────┘                     └─────────────────┘
```

### Sender Components
- **MCU**: ESP32-WROOM-32E (4MB Flash + PSRAM)
- **Sensors**: 8x MQ-series gas sensors + environmental sensors
- **AI Engine**: TensorFlow Lite Micro
- **Communication**: WiFi with TCP/UDP protocols

### Receiver Components
- **MCU**: Raspberry Pi 4B (4GB RAM)
- **Actuators**: 5x micro peristaltic pumps + solenoid valves
- **Mixing System**: 3D-printed mixing chamber
- **Display**: 0.96" OLED screen


## AI Model Development

### Data Collection
- **Target Odors**: Coffee, Alcohol, Lemon, Mint, Lavender
- **Sample Size**: 1000 samples per odor type
- **Collection Environment**: Controlled temperature (10-30°C) and humidity (30-80%)

### Model Architecture
- **Base Model**: 1D CNN + LSTM hybrid
- **Quantization**: INT8 for ESP32 deployment
- **Model Size**: <200KB
- **Inference Time**: <200ms


## Hardware Specifications

### Sender Hardware
| Component | Model | Quantity |
|-----------|-------|----------|
| MCU | ESP32-WROOM-32E | 1 |
| Gas Sensors | MQ-2 to MQ-9 | 8 |
| Environmental | DHT22, BMP280 | 2 |
| ADC | ADS1115 | 1 |

### Receiver Hardware
| Component | Model | Quantity |
|-----------|-------|----------|
| MCU | Raspberry Pi 4B | 1 |
| Pumps | Micro Peristaltic | 5 |
| Valves | Solenoid 12V | 5 |
| Mixing Chamber | 3D Printed | 1 |

## Project Structure

```
ROTS/
├── sender/                 # ESP32 firmware
│   ├── src/               # Source code
│   ├── lib/               # Libraries
│   └── models/            # AI models
├── receiver/              # Raspberry Pi software
│   ├── src/               # Python source
│   ├── config/            # Configuration files
│   └── recipes/           # Odor recipes database
├── docs/                  # Documentation
│   ├── hardware/          # Hardware guides
│   ├── software/          # Software documentation
│   └── api/               # API reference
├── models/                # 3D printable parts
├── data/                  # Training data
└── tests/                 # Test scripts
```


## Performance Targets

| Metric | Target |
|--------|--------|
| Recognition Accuracy | >85% |
| Recognition Time | <500ms |
| Transmission Latency | <200ms |
| Power Consumption | <5W (Sender), <15W (Receiver) |
| Model Size | <200KB |

## Implementation Plan

### Phase 1: Hardware Prototype (4-6 weeks)
- ESP32 sensor array development and testing
- Basic gas sensor calibration and data collection
- Raspberry Pi actuator control system
- Initial communication protocol implementation

### Phase 2: AI Model Development (6-8 weeks)
- Data collection from controlled environments
- Feature engineering and model training
- TensorFlow Lite model optimization for ESP32
- Real-time inference testing and validation

### Phase 3: System Integration (4-6 weeks)
- End-to-end system integration
- Performance optimization and tuning
- User interface development
- Comprehensive testing and validation

### Phase 4: Advanced Features (4-6 weeks)
- Multi-odor recognition capabilities
- Advanced mixing algorithms
- Remote monitoring and control
- Documentation and deployment guides

## Current Research Directions

- **Sensor Fusion Algorithms**: Developing robust multi-sensor data fusion techniques
- **Edge AI Optimization**: Exploring model compression and quantization strategies
- **Odor Mixing Theory**: Researching optimal scent combination algorithms
- **Real-time Processing**: Investigating low-latency data processing pipelines



## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
