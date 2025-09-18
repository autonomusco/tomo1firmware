Tomo Pendant Firmware (ESP32-S3)

![ESP32-S3 Firmware Build](https://github.com/autonomusco/tomopendantfirmware/actions/workflows/esp32s3-build.yml/badge.svg)

The **Tomo Pendant** is a wearable safety and companionship device designed for elder care.  
This firmware runs on the **ESP32-S3** microcontroller and enables:  

- 🔗 BLE connectivity with the caregiver mobile app  
- 🧭 Fall detection using the **MPU6050** accelerometer/gyro  
- 🔋 Battery monitoring with the **MAX17048** fuel gauge  
- 🆘 Emergency button input and alert pipeline  
- ☁️ Cloud API hooks for caregiver notifications  
- 🔄 OTA update framework for secure upgrades  
- 🎙 Voice trigger and audio hooks for future features  
- 🧑‍⚕️ Health sensor integration (HR/SpO₂/temp) planned for later  


Repository Structure
```
.github/workflows/     → GitHub Actions CI/CD pipeline
docs/                  → Hardware schematic and pin map (PDF)
main/                  → Firmware source files (BLE, fall detection, power, OTA, etc.)
CMakeLists.txt         → Build system entry
sdkconfig.defaults     → Default ESP-IDF configuration
README.md              → This file
```

Developer Guide

1. Clone the Repository
```bash
git clone <repo-url>
cd tomo-pendant-firmware
```

2. Set Up ESP-IDF (v5.2.1, ESP32-S3)
```bash
git clone -b v5.2.1 --recursive https://github.com/espressif/esp-idf.git ~/esp-idf
cd ~/esp-idf
./install.sh esp32s3
. ~/esp-idf/export.sh
```
> Run `. ~/esp-idf/export.sh` **in every new terminal session** before building.

3. Build the Firmware
```bash
idf.py set-target esp32s3
idf.py build
```

### 4. Flash the Device
Connect the ESP32-S3 via USB, then:
```bash
idf.py -p /dev/ttyUSB0 flash monitor
```
(Replace `/dev/ttyUSB0` with your serial port, e.g., `/dev/cu.usbserial-*` on macOS.)*  
Press **Ctrl+]** to exit the monitor.

5. CI/CD with GitHub Actions
- Every push to `main` triggers a GitHub Actions build.  
- Build artifacts (`.bin`, `.elf`) are downloadable from the **Actions tab**.  
- Each artifact is tagged with the commit SHA for traceability.



Hardware Reference

I²C Devices
- **MPU6050** – accelerometer/gyro for fall detection  
- **MAX17048** – fuel gauge for battery state of charge  

Pin Map

| Function             | ESP32-S3 Pin      | Connected To                                |
|----------------------|------------------|---------------------------------------------|
| I2C SDA              | GPIO 8           | MPU6050 SDA, MAX17048 SDA                   |
| I2C SCL              | GPIO 9           | MPU6050 SCL, MAX17048 SCL                   |
| Power 3.3V           | 3V3              | VCC of MPU6050 + MAX17048                   |
| Ground               | GND              | GND of MPU6050 + MAX17048                   |
| Emergency Button     | GPIO 0 (example) | Button input (active low w/ pull-up)        |
| Buzzer/Vibration     | GPIO 10 (example)| Driver transistor for buzzer/motor          |
| Battery Sense (opt.) | GPIO 1 (ADC)     | Voltage divider if needed                   |
| Microphone (future)  | GPIO 4,5,6 (I2S) | ICS-43434 / SPH0645 MEMS mic                |

See `docs/tomo_pendant_hardware_reference.pdf` for the schematic and pin map one-pager.


Next Steps for Development
- Implement fall detection algorithm (threshold + orientation logic).  
- Complete MAX17048 driver integration.  
- Add OTA update logic.  
- Implement BLE GATT services for event uplink.  
- Test battery life under sleep/wake cycles.  
- Extend hooks for health sensors and voice triggers.  


Developer Notes
- All modules are scaffolded with `TODO` placeholders but compile cleanly.  
- Contractors must **implement inside existing `.c/.h` stubs** without changing repo structure.  
- CI builds must stay green — do not merge code that breaks `idf.py build`.  
- Hardware pins can be updated in `i2c_bus.c` if wiring changes.  
- Keep all commits small and traceable. Each feature branch should map to a Jira/Notion task.  
- Use GitHub Actions artifacts for testing before flashing hardware.  
- Document any new external components or libraries in `/docs`.  

Contribution Workflow
1. Create a feature branch.  
2. Implement and test locally.  
3. Push branch and open a Pull Request.  
4. Ensure GitHub Actions build passes.  
5. Request review and merge.  

Commit directly to main.

## 📜 License
© Autonomus Co. – All rights reserved.  
For Tomo development use only. 
