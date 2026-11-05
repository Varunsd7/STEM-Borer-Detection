# Wiring notes

Default ESP32 pin assignments:

| Device | Signal | ESP32 pin |
|---|---|---|
| KY-037 | Analog output (AO) | GPIO 34 |
| DHT22 | Data | GPIO 4 |
| Relay board | IN | GPIO 26 |

## Power and electrical safety

- ESP32 ADC pins must never receive more than 3.3 V.
- Check the KY-037 board's supply and analog-output range. If its AO can exceed 3.3 V, add appropriate level conditioning.
- Use a suitable regulated supply and common ground for low-voltage signal connections.
- Relay module input compatibility varies. Confirm active-high/active-low behaviour before connecting a load.
- Test first with a low-voltage indicator (for example, an LED load).
- Do not prototype mains wiring on a breadboard. Mains-powered traps require isolation, enclosure, fusing, strain relief, and qualified installation.
