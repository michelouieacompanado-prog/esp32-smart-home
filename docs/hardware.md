<!-- Hardware specifications, bill of materials, pin assignments, and wiring schematics. -->

# Hardware Documentation: ESP32 Smart Home

This document details the bill of materials, pin mapping tables, electrical considerations, and wiring schematics for both the Smart Plug and Environmental Sensor nodes.

---

## 1. Bill of Materials (BOM)

| Item | Description | Quantity | Interface | Operating Voltage |
|------|-------------|----------|-----------|-------------------|
| Microcontroller | ESP32 DevKit V1 (ESP-WROOM-32, 4MB Flash) | 2 | Micro-USB / 5V | 3.3V / 5V |
| Power Relay | 5V 1-Channel Relay Module with Optocoupler | 1 | Digital GPIO | 5V VCC, 3.3V Logic |
| Status LED | Onboard Blue LED (GPIO 2) | 1 (onboard) | Digital GPIO | 3.3V |
| User Button | Onboard BOOT Button (GPIO 0) | 1 (onboard) | Digital Input | Pulled-up to 3.3V |
| Temp Sensor | BMP280 Barometric Pressure & Temp Sensor | 1 | I2C (0x76) | 3.3V |
| Light Sensor | TSL2561 Luminosity / Ambient Light Sensor | 1 | I2C (0x39) | 3.3V |
| OLED Display | SSD1306 0.96" 128x64 Monochrome OLED | 1 | I2C (0x3C) | 3.3V / 5V |
| Prototyping | Breadboard and Dupont jumper wires | As needed | N/A | N/A |

---

## 2. Pin Mapping Tables

### 2.1 Smart Plug Node

| ESP32 Pin | Connected Component | Function | Active Level | Default State |
|-----------|---------------------|----------|--------------|---------------|
| GPIO 19   | Relay Module IN     | Digital Output (Control) | HIGH (1)     | LOW (0) (Off) |
| GPIO 2    | Onboard LED         | Status Indicator         | HIGH (1)     | LOW (0) (Off) |
| GPIO 0    | BOOT Push Button    | Interrupt Input (Debounced) | LOW (0)    | HIGH (Pulled up) |
| VIN (5V)  | Relay VCC           | Power                    | 5V DC        | N/A |
| GND       | Relay GND           | Common Ground            | 0V           | N/A |

### 2.2 Environmental Sensor Node

| ESP32 Pin | Connected Component | Function | Notes |
|-----------|---------------------|----------|-------|
| GPIO 21   | BMP280, TSL2561, SSD1306 SDA | Shared I2C Data   | External or internal pull-up required |
| GPIO 22   | BMP280, TSL2561, SSD1306 SCL | Shared I2C Clock  | External or internal pull-up required |
| 3V3 (3.3V)| Sensor & OLED VCC            | Power Rail        | Connect to 3.3V pin on ESP32 |
| GND       | Sensor & OLED GND            | Common Ground     | Connect to GND pin on ESP32 |

---

## 3. Wiring Schematics

### 3.1 Smart Plug Wiring Diagram

```
       +-----------------------+
       |   ESP32 DevKit (Plug) |
       |                       |
       |               GPIO 19 +------------------> Relay Module [IN]
       |                       |
       |               GPIO 2  +------------------> Onboard Blue LED
       |                       |
       |               GPIO 0  +<------------------ Onboard BOOT Button
       |                       |
       |               VIN(5V) +------------------> Relay Module [VCC]
       |                       |
       |                   GND +------------------> Relay Module [GND]
       +-----------------------+

       [Relay Terminal Block]:
       Mains AC Live In ------> [ COM ]
       Load / Appliance  <----- [ NO  ]
```

### 3.2 Environmental Sensor Wiring Diagram

The BMP280, TSL2561, and SSD1306 display share the single hardware I2C bus:

```
                         ESP32 DevKit (Sensor)
                       +-----------------------+
                       |                       |
                       |       GPIO 21 (SDA) --+-----+--------------+--------------+
                       |       GPIO 22 (SCL) --+--+  |              |              |
                       |                       |  |  |              |              |
                       |                  3V3 -+--+--+---+          |              |
                       |                  GND -+--+--+---+----+     |              |
                       +-----------------------+  |  |   |    |     |              |
                                                  |  |   |    |     |              |
               +----------------------------------+  |   |    |     |              |
               |       +-----------------------------+   |    |     |              |
               |       |                                 |    |     |              |
         +-----+-------+-----+                     +-----+----+-----+        +-----+-----+-----+
         | SCL  SDA  VCC GND |                     | SCL  SDA VCC GND |       | SCL SDA VCC GND |
         |                   |                     |                  |       |                 |
         |  BMP280 (0x76)    |                     |  TSL2561 (0x39)  |       | SSD1306 (0x3C)  |
         +-------------------+                     +------------------+       +-----------------+
```

---

## 4. Electrical and Safety Considerations

1. **Relay Isolation**: Always verify that the relay board features optocoupler isolation between the ESP32 3.3V logic circuit and the high-voltage relay coil/contacts.
2. **I2C Bus Loading**: BMP280, TSL2561, and SSD1306 modules generally contain onboard 4.7 kOhm or 10 kOhm pull-up resistors to 3.3V. Ensure total parallel resistance on SDA/SCL remains above 1.5 kOhm.
3. **Mains Electricity Safety**: When switching real alternating current (AC 110V/220V) with the relay, exercise extreme caution. Keep high-voltage connections securely shielded in an enclosure.
