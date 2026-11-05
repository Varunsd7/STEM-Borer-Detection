# 🌱 IoT-Based Stem Borer Detection and Monitoring System

## Overview

The **Stem Borer Detection and Monitoring System** is an IoT-based agricultural monitoring solution designed to detect potential stem borer activity using acoustic sensing and environmental monitoring.

The system uses an ESP32 microcontroller integrated with a KY-037 sound sensor and DHT22 temperature and humidity sensor. It monitors acoustic activity, collects environmental parameters, and communicates live readings through a Telegram bot.

The system can also control a light trap through a relay module.

## Objectives

- Monitor acoustic activity associated with insect movement.
- Measure surrounding temperature and humidity.
- Provide real-time monitoring through Telegram.
- Enable relay-based light trap control.
- Explore IoT-based pest monitoring for smart agriculture.

## Hardware Components

| Component | Application |
|---|---|
| ESP32 DevKit V1 | Main microcontroller |
| KY-037 | Acoustic sensing |
| DHT22 | Temperature and humidity monitoring |
| Relay Module | Light trap control |
| Light Trap | Pest attraction mechanism |
| Power Supply | System power |

## Working Principle

1. The KY-037 microphone module captures surrounding acoustic signals.
2. The ESP32 processes the analog signal and estimates acoustic activity.
3. The DHT22 measures temperature and humidity.
4. The ESP32 sends readings through Wi-Fi to a Telegram bot.
5. The relay module provides control of the light trap.

**Note:** Acoustic activity is an experimental indicator and does not independently confirm stem borer presence. Detection thresholds require calibration and field validation.

## Technology Stack

- **Microcontroller:** ESP32
- **Programming:** Embedded C/C++, Python
- **Sensors:** KY-037, DHT22
- **Communication:** Wi-Fi
- **IoT Platform:** Telegram Bot API
- **Development Environment:** Arduino IDE

## Repository Structure

```text
STEM-Borer-Detection/
├── README.md
├── firmware/
│   └── esp32_code.ino
├── sensors/
│   ├── KY037.md
│   └── DHT22.md
├── telegram_bot/
│   └── bot_code.py
├── circuit_diagrams/
├── images/
├── documentation/
└── LICENSE
```

## Installation and Setup

1. Install Arduino IDE.
2. Install ESP32 board support.
3. Install the required DHT and Telegram libraries.
4. Connect the sensors according to the circuit documentation.
5. Configure Wi-Fi credentials and Telegram bot credentials.
6. Upload the ESP32 firmware.
7. Start the Telegram bot and monitor the sensor readings.

## Future Scope

- Improved acoustic signal processing.
- Machine learning-based insect classification.
- Historical sensor data logging.
- Cloud-based monitoring dashboard.
- Field testing and performance evaluation.

## Project Status

Prototype development and experimental testing.

## Contributors

Project developed as an IoT-based smart agriculture initiative.

## License

This project is licensed under the MIT License.
