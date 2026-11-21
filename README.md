# ESP32 Smart Home System (ESP-IDF / ESP RainMaker)

An enterprise-grade, two-device Smart Home system built for generic ESP32 DevKit boards using ESP-IDF (C++) via PlatformIO and ESP RainMaker. This project adapts Chapter 9 ("Project – Smart Home") of *"Developing IoT Projects with ESP32, 2nd Edition"*, featuring clean FreeRTOS architecture, hardware interrupts, software timers, thread-safe I2C bus arbitration, and native unit testing.

---

## 1. Overview & System Features

The project encompasses two independent firmware targets managed in a single unified PlatformIO workspace:

1. **Smart Plug Node (`plug`)**:
   - High-voltage load control via 5V Relay on GPIO 19.
   - Status indication via onboard blue LED on GPIO 2.
   - Physical user control via BOOT button (GPIO 0) using hardware ISR and FreeRTOS software timer debouncing.
   - Remote control, state reporting, OTA firmware updates, scheduling, scenes, and Alexa integration via ESP RainMaker cloud.
   - Unified `plug_task` event loop processing serialized commands from physical inputs and cloud MQTT messages.

2. **Environmental Sensor Node (`sensor`)**:
   - BMP280 temperature sensing (I2C address `0x76`).
   - TSL2561 ambient light intensity measurement (I2C address `0x39`) mapped to 0–100%.
   - SSD1306 128x64 OLED display (I2C address `0x3C`) with real-time digital clock, sensor metrics, and network/cloud status flags.
   - Periodic sensor acquisition task executing precisely every 10 seconds via `vTaskDelayUntil()`.
   - Length-1 overwrite queue connecting the acquisition task to the display rendering task.
   - Thread-safe I2C bus sharing guarded by a FreeRTOS mutex.
   - 1-second FreeRTOS software timer driving display clock updates.
   - FreeRTOS event group tracking Wi-Fi station connectivity and RainMaker MQTT broker link status.

---

## 2. Hardware Specifications & Wiring Summary

### Target Platform
- **Board**: `esp32dev` (Generic ESP32 DevKit V1 / ESP-WROOM-32)
- **Flash Size**: 4MB Flash (configured with custom dual OTA partition table in `partitions.csv`)
- **Target ESP-IDF Version**: ESP-IDF v5.2.1 (`framework-espidf @ 3.50201.240515`)
- **Target PlatformIO Platform**: `platform = espressif32 @ 6.6.0` (pinned to ensure compatible Xtensa toolchain v12.2.0)
- **Component Resolution**: Dependencies (`espressif/esp_rainmaker`, `rmaker_app_network`, `rmaker_app_insights`, `mdns`, `bmp280`, `tsl2561`, `i2cdev`) are resolved declaratively via `src/idf_component.yml` using the ESP-IDF Component Manager.
- **Target Selection**: `platformio.ini` supplies `board_build.cmake_extra_args = -DDEVICE=plug` / `-DDEVICE=sensor` so `src/CMakeLists.txt` selectively compiles only `common/` plus the target device code.

### Smart Plug Pinout

| ESP32 Pin | Connected Component | Function / Mode | Active Level | Default State |
|---|---|---|---|---|
| GPIO 19 | 5V Relay IN | Digital Output | HIGH (1 = ON) | LOW (OFF) |
| GPIO 2 | Onboard LED | Digital Output | HIGH (1 = ON) | LOW (OFF) |
| GPIO 0 | BOOT Button | Digital Input (Pullup, Negedge ISR) | LOW (0 = Pressed) | Released |
| VIN (5V) | Relay VCC | Power Rail | 5V DC | Active |
| GND | Relay GND | Common Ground | 0V | Ground |

### Environmental Sensor Pinout

| ESP32 Pin | Connected Component | Function / Mode | Notes |
|---|---|---|---|
| GPIO 21 | BMP280, TSL2561, SSD1306 SDA | Shared I2C Data (SDA) | Pull-up enabled |
| GPIO 22 | BMP280, TSL2561, SSD1306 SCL | Shared I2C Clock (SCL)| 100 kHz standard clock |
| 3V3 (3.3V)| All Sensors & Display VCC | 3.3V Power Rail | Connect to ESP32 3.3V |
| GND | All Sensors & Display GND | Common Ground | Connect to ESP32 GND |

For detailed wiring schematics and pin tables, refer to [`docs/hardware.md`](docs/hardware.md).

---

## 3. Project Structure

```
esp32-smart-home/
├── README.md                    # System documentation and operational guide
├── .gitignore                   # Git ignore rules for build artifacts and temporary files
├── platformio.ini               # Two environments: plug, sensor
├── CMakeLists.txt               # ESP-IDF root project definition
├── sdkconfig.defaults           # Default configuration (4MB flash, dual OTA, BLE, RainMaker)
├── partitions.csv               # 4MB flash partition table with two ~1.9MB OTA slots
├── docs/
│   ├── hardware.md              # Parts list, pin tables, electrical wiring schematics
│   └── design.md                # Class diagram, RTOS concurrency tables, sequence diagrams
├── include/                     # Headers (mirrors src/)
│   ├── common/
│   │   ├── network.hpp          # NVS and Wi-Fi provisioning controller
│   │   └── rainmaker_node.hpp   # Base RainMaker node (OTA, system service, scenes)
│   ├── plug/
│   │   ├── config.hpp           # Hardware pin assignments and RTOS parameters
│   │   ├── relay.hpp            # Relay and status LED driver interface
│   │   ├── button.hpp           # ISR and software-timer debounce button handler
│   │   ├── plug_task.hpp        # Relay control and command serialization task
│   │   └── plug_node.hpp        # Smart plug RainMaker device implementation
│   └── sensor/
│       ├── config.hpp           # I2C configuration, addresses, and RTOS constants
│       ├── reading.hpp          # Sensor reading data structures
│       ├── logic.hpp            # Pure mapping functions (lux -> 0-100), unit-testable
│       ├── i2c_bus.hpp          # Mutex-guarded I2C bus driver
│       ├── temperature_sensor.hpp # BMP280 sensor driver wrapper
│       ├── light_sensor.hpp     # TSL2561 sensor driver wrapper
│       ├── oled_display.hpp     # SSD1306 OLED display driver
│       ├── sensor_task.hpp      # Periodic 10s sensor sampling task
│       ├── display_task.hpp     # 1s OLED display and clock update task
│       └── sensor_node.hpp      # Sensor RainMaker device implementation
├── src/                         # Implementations (mirrors include/)
│   ├── CMakeLists.txt           # Selects sources based on target device
│   ├── idf_component.yml        # Managed dependencies from Espressif Component Registry
│   ├── common/
│   │   ├── network.cpp          # NVS setup and BLE provisioning implementation
│   │   └── rainmaker_node.cpp   # Base node logic, OTA, and cloud event callbacks
│   ├── plug/
│   │   ├── main.cpp             # Startup wiring for the plug
│   │   ├── relay.cpp            # Relay and LED control implementation
│   │   ├── button.cpp           # Interrupt service routine and debounce timer logic
│   │   ├── plug_task.cpp        # Event processing loop and watermark logging
│   │   └── plug_node.cpp        # RainMaker parameter registration and write handling
│   └── sensor/
│       ├── main.cpp             # Startup wiring for the environmental sensor
│       ├── i2c_bus.cpp          # I2C master driver with FreeRTOS mutex
│       ├── temperature_sensor.cpp # BMP280 data reading and error handling
│       ├── light_sensor.cpp     # TSL2561 data reading and error handling
│       ├── oled_display.cpp     # OLED buffer rendering and frame flush
│       ├── sensor_task.cpp      # Periodic sampling, conversion, and cloud reporting
│       ├── display_task.cpp     # Screen formatting and 1s timer handling
│       └── sensor_node.cpp      # Temperature & light param publishing
└── test/
    └── test_sensor_logic.cpp    # Unity native unit tests for logic.hpp
```

---

## 4. Building and Flashing

### 4.1 Prerequisites
- PlatformIO Core (CLI) or PlatformIO IDE extension for VS Code.
- USB-to-UART drivers (CP2102 or CH340) installed.

### 4.2 Compile Firmware

To compile either device firmware:

```bash
# Build the smart plug firmware
pio run -e plug

# Build the environmental sensor firmware
pio run -e sensor
```

### 4.3 Flash to ESP32 Hardware

Connect the corresponding ESP32 DevKit to your computer via USB and upload:

```bash
# Flash Smart Plug
pio run -e plug -t upload

# Flash Environmental Sensor
pio run -e sensor -t upload
```

### 4.4 Serial Monitor

Launch the serial terminal to monitor logs and stack high-water marks:

```bash
# Monitor the Smart Plug
pio device monitor -e plug

# Monitor the Environmental Sensor
pio device monitor -e sensor
```

### 4.5 Execute Unit Tests

The conversion mathematics (`luxToLightIntensity`) are isolated in `include/sensor/logic.hpp` without hardware dependencies. Run the native tests:

```bash
# Run with host compiler (GCC/G++ with Unity)
g++ -Iinclude test/test_sensor_logic.cpp -lunity -o test_sensor_logic && ./test_sensor_logic
```

---

## 5. ESP RainMaker Phone App & Provisioning Walkthrough

1. **Install Mobile App**: Download the **ESP RainMaker** app from the App Store (iOS) or Google Play Store (Android).
2. **Account Sign-in**: Create or log in to your Espressif RainMaker account.
3. **Provisioning (BLE)**:
   - Power on the ESP32 device.
   - Tap **Add Device** (`+` icon) in the RainMaker app.
   - The device advertises over Bluetooth Low Energy (BLE).
   - If prompted for a Proof-of-Possession (PoP) code, check the serial monitor output (random PoP is printed on boot).
   - Select your 2.4 GHz Wi-Fi network and enter credentials.
   - Once connected to the local Wi-Fi, the device establishes a secure MQTT link with the ESP RainMaker cloud broker.
4. **Dashboard View**:
   - **Plug**: Displays a toggle switch named "power". Toggling updates the physical relay (GPIO 19) and onboard LED (GPIO 2) instantaneously.
   - **Sensor**: Displays real-time temperature (°C) and light intensity (%) gauges, which update every 10 seconds.

---

## 6. Testing Chapter 9 Cloud Features

ESP RainMaker enables advanced cloud automation workflows out of the box:

### 6.1 Schedules
- Open the device in the RainMaker app, navigate to **Schedules**, and tap **Add Schedule**.
- Configure actions (e.g., Turn ON plug at 07:00 AM every Monday through Friday).
- The schedule is synchronized to the device's internal RTC and executes autonomously even if the internet connection is temporarily interrupted.

### 6.2 Scenes
- Navigate to the **Scenes** tab in the app.
- Create a scene named "Good Night" and assign actions across both devices (e.g., Plug power -> OFF).
- Execute the scene with a single tap to trigger multi-device actions concurrently.

### 6.3 Automations
- In the RainMaker app, open **Automations** -> **Create Automation**.
- Set **Condition**: Sensor device -> `light-intensity` < 20%.
- Set **Action**: Plug device -> `power` = ON.
- Cover the TSL2561 sensor to observe the plug turning ON automatically via the cloud rules engine.

### 6.4 Voice Assistant Integration (Amazon Alexa)
- Enable the **ESP RainMaker** Skill in the Amazon Alexa app.
- Link your ESP RainMaker account.
- Discover devices: Alexa automatically detects "Plug" and "Sensor".
- Test voice commands:
  - *"Alexa, turn on the Plug."*
  - *"Alexa, what is the temperature on Sensor?"*

---

## 7. Troubleshooting Guide

| Issue | Probable Cause | Corrective Action |
|---|---|---|
| Relay does not click on state change | Incorrect GPIO or power supply | Ensure relay VCC is connected to 5V (VIN), and IN is on GPIO 19. Check active level setting in `include/plug/config.hpp`. |
| Display is blank or shows artifacts | I2C address mismatch or pull-up issue | Verify OLED address is `0x3C`. Ensure SDA (GPIO 21) and SCL (GPIO 22) connections are firm and powered at 3.3V. |
| Sensor read failure logged in serial console | I2C communication error | Check BMP280 (`0x76`) and TSL2561 (`0x39`) wiring. The driver gracefully suppresses cloud reporting on read error to prevent erroneous data publication. |
| Wi-Fi fails to connect during BLE provisioning | 5 GHz network selected | ESP32 only supports 2.4 GHz Wi-Fi networks. Ensure the router 2.4 GHz band is broadcast. |
| Flash overflow during compilation | Partition table too small | Ensure `board_build.partitions = partitions.csv` is active in `platformio.ini` to allocate 1.9MB dual OTA slots. |
| Toolchain assembler error (`--dynconfig=xtensa_esp32.so`) | MSYS2 or MinGW `as.exe` shadowing the toolchain on PATH | Pin `platform = espressif32 @ 6.6.0` in `platformio.ini` to use the self-contained `toolchain-xtensa-esp32` package. |
| Linker error: `.iram0.text` overflowed `iram0_0_seg` | FreeRTOS/Wi-Fi/mbedTLS functions placed in IRAM | Enabled `CONFIG_FREERTOS_PLACE_FUNCTIONS_INTO_FLASH=y`, `CONFIG_ESP_WIFI_IRAM_OPT=n`, and NimBLE in `sdkconfig.defaults`. |
