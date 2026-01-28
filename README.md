# ESP32 Sauna Controller - THIS IS A WORK IN PROGRESS AND MAY NOT BE COMPLETE

A smart sauna controller built with ESPHome, featuring temperature monitoring, automated safety cutoffs, and Home Assistant integration.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![ESPHome](https://img.shields.io/badge/ESPHome-2025.4.0-green.svg)

## Overview

This project creates a fully-functional sauna controller with:
- Real-time temperature monitoring (°C and °F)
- 16x2 LCD display with time and temperature
- Automated safety features (overheat protection, 6-hour auto-shutoff)
- Smart temperature control (auto-resume heating when safe)
- Home Assistant integration with notifications
- Web interface for remote control

## Table of Contents

- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Wiring Diagram](#wiring-diagram)
- [Software Setup](#software-setup)
- [Safety Features](#safety-features)
- [Home Assistant Integration](#home-assistant-integration)
- [Troubleshooting](#troubleshooting)
- [License](#license)

## Features

### Core Functionality
- **Temperature Monitoring**: DS18B20 waterproof probe with 5-second updates
- **LCD Display**: 16x2 I2C display showing date, time, temperature (°C/°F), and heater status
- **Relay Control**: Control of sauna heater

### Safety Features
- **Overheat Protection**: Automatic shutoff at 200°F (93.3°C)
- **Auto-Resume**: Resumes heating when temperature drops below 190°F (87.8°C) if master switch is still on
- **6-Hour Timeout**: Automatic shutoff after 6 hours of continuous operation
- **Temperature Alerts**: Notification when sauna reaches 160°F (71°C)

### Smart Features
- **Home Assistant Integration**: Full control and monitoring through HA dashboard
- **Web Interface**: Built-in web server for standalone control
- **OTA Updates**: Wireless firmware updates
- **WiFi Fallback**: Creates backup hotspot if main WiFi fails

## Hardware Requirements

### Required Components

| Component | Specification | Qty | Estimated Cost | Source |
|-----------|--------------|-----|----------------|--------|
| **ESP32 WROOM-32** | DevKit V1, 30-pin | 1 | $8 | Amazon, AliExpress |
| **1602 LCD Display** | 16x2 with I2C backpack (PCF8574) | 1 | $6 | Amazon, AliExpress |
| **DS18B20 Temperature Probe** | Waterproof, 3-wire cable | 1 | $8 | Amazon, AliExpress |
| **Relay Module** | 5V, 10A+ rated, optoisolated | 1 | $5 | Amazon, AliExpress |
| **4.7kΩ Resistor** | 1/4W (may be built into or included w/ probe) | 1 | $0.10 | Local electronics store |
| **USB Power Supply** | 5V, 1A+ | 1 | $5 | Amazon |
| **Wires** | 22 AWG for low voltage | - | $5 | Amazon |
| **Wago Connectors** | 221 series or similar | 10 | $10 | Amazon, Home Depot |
| **Project Enclosure** | 100x65x45mm (approx) | 1 | $8 | DIY, Linked |

**Total Cost**: ~$55

### Optional Components
- **3D printed enclosure** (STL files in `/hardware` folder)

### Tools Needed
- Soldering iron and solder
- Wire strippers
- Multimeter
- Screwdrivers
- Computer with USB port

## Wiring Diagram

### Pin Connections

| Component | Wire/Pin | → | ESP32 Pin | Notes |
|-----------|----------|---|-----------|-------|
| **DS18B20 Temp Probe** | | | | |
| Red wire | VCC | → | 3.3V | Power |
| Black wire | GND | → | GND | Ground |
| Yellow wire | DATA | → | GPIO4 | OneWire data |
| **(Optional)** | 4.7kΩ Resistor | → | Between DATA & 3.3V | Pull-up (may be built-in) |
| **1602 LCD (I2C)** | | | | |
| VCC | Power | → | 5V | Check your LCD voltage |
| GND | Ground | → | GND | Common ground |
| SDA | Data | → | GPIO21 | I2C data line |
| SCL | Clock | → | GPIO22 | I2C clock line |
| **Relay Module** | | | | |
| VCC | Power | → | 5V | Most relays need 5V |
| GND | Ground | → | GND | Common ground |
| IN | Signal | → | GPIO26 | Relay trigger |

### Visual Wiring Diagram

```
                    ┌─────────────────┐
                    │   ESP32 WROOM   │
                    │                 │
   ┌────────────────┤ 3.3V            │
   │    ┌───────────┤ GND             │
   │    │  ┌────────┤ GPIO4           │──► DS18B20 Data (Yellow)
   │    │  │  ┌─────┤ GPIO21          │──► LCD SDA
   │    │  │  │  ┌──┤ GPIO22          │──► LCD SCL
   │    │  │  │  │┌─┤ GPIO26          │──► Relay IN
   │    │  │  │  ││ │ 5V              │──┐
   │    │  │  │  ││ └─────────────────┘  │
   │    │  │  │  ││                      │
   │    │  │  │  ││  ┌──────────────┐   │
   │    │  │  │  ││  │  1602 LCD    │   │
   │    │  │  │  │└──┤ SDA          │   │
   │    │  │  │  └───┤ SCL          │   │
   │    │  │  │      │ VCC ◄────────┼───┤
   │    │  │  │   ┌──┤ GND          │   │
   │    │  │  │   │  └──────────────┘   │
   │    │  │  │   │                     │
   │    │  │  │   │  ┌──────────────┐   │
   │    │  │  │   │  │ DS18B20      │   │
   │    │  │  └───┼──┤ Data (Yellow)│   │
   │    │  │      │  │ GND (Black)  │   │
   └────┼──┼──────┤  │ VCC (Red)    │   │
        │  │      │  └──────────────┘   │
        │  │      │                     │
        │  │      │  ┌──────────────┐   │
        │  │      │  │ Relay Module │   │
        │  │      └──┤ GND          │   │
        │  └─────────┤ IN (Signal)  │   │
        │            │ VCC ◄────────┼───┘
        │            └──────────────┘
        │
        └─── Common Ground Bus


     HIGH VOLTAGE SIDE (⚡ DANGER!)
     ┌──────────────────────┐
     │ Relay Output         │
     │ COM ◄─── Line (Hot)  │
     │ NO  ───► To Heater   │
     │ NC  (not used)       │
     └──────────────────────┘
```

### Important Wiring Notes

⚠️ **CRITICAL SAFETY WARNINGS**:
- The relay output side carries **MAINS VOLTAGE** (120V/240V)
- Use proper electrical junction box
- Follow local electrical codes
- Use minimum 12 AWG wire for heater connections
- **Consider hiring a licensed electrician** for the high-voltage connections

✓ **Best Practices**:
- Use **Wago 221 lever connectors** for reliable connections
- Keep all low-voltage wiring away from high-voltage
- Use colored wire for easy identification
- Label all connections
- Test continuity before applying power

## Software Setup

### Two Installation Methods

You can set up this controller in two ways:

#### Method 1: Standalone (No ESPHome Installation Required) ⭐ EASIEST

Perfect if you just want a working sauna controller without installing anything on your computer!

**What you get**:
- Web interface at `http://saunacontroller.local` or device IP address
- Toggle sauna on/off from your phone/computer browser
- View temperature in real-time
- Works on your WiFi or its own hotspot

**Steps**:
1. Download the pre-compiled `.bin` firmware from the [Releases](../../releases) page
2. Flash to ESP32 using [ESPHome Web Flasher](https://web.esphome.io/)
3. Connect ESP32 via USB to computer
4. Open https://web.esphome.io/ in Chrome/Edge browser
5. Click "Connect" and select your ESP32
6. Click "Install" and choose the downloaded `.bin` file
7. After flashing, connect to "Sauna Fallback" WiFi (password: `fallback123`)
8. Configure your WiFi through the captive portal
9. Done! Access via `http://saunacontroller.local` or the IP shown on LCD

**No computer software required after initial flash!**

#### Method 2: DIY/Developer Method (With ESPHome)

If you want to customize the configuration or don't want to use pre-compiled firmware:

**Prerequisites**:

1. **Install ESPHome**
   ```bash
   pip install esphome
   ```

2. **Create secrets.yaml file** (optional but recommended)
   ```yaml
   wifi_ssid: "YourWiFiSSID"
   wifi_password: "YourWiFiPassword"
   ```

### ESPHome Configuration

Save this as `saunacontroller.yaml`:

```yaml
# See full configuration in repository
esphome:
  name: saunacontroller
  friendly_name: SaunaController

esp32:
  board: esp32dev
  framework:
    type: arduino

# ... (full config in repo)
```

### Standalone Usage (No Home Assistant)

Once flashed, your sauna controller works completely independently!

#### Accessing the Web Interface

**On Your WiFi**:
1. Open browser on phone/computer
2. Go to `http://saunacontroller.local`
3. Or use the IP address shown on the LCD display

**Using Fallback Hotspot** (if WiFi fails):
1. Connect to WiFi network "Sauna Fallback"
2. Password: `fallback123`
3. Go to `http://192.168.4.1`

#### Web Interface Features

The built-in web interface shows:
- 🌡️ **Current temperature** (live updates)
- 🔄 **Sauna on/off toggle** (click to control)
- 📊 **Sensor status** (temp probe, relay state)
- 📶 **WiFi signal strength**
- 🔄 **Uptime** counter

**Example**:
```
Sauna Controller
═══════════════════════

🌡️ Sauna Temperature: 165.2°F

🔥 Sauna: ○ OFF  [Toggle]

🔥 Sauna Heater: OFF

📶 WiFi Signal: -45 dBm

⏱️ Uptime: 2d 5h 32m
```

#### Changing WiFi Settings (Standalone)

If you need to connect to a different WiFi network:

1. Power on the ESP32
2. Wait 30 seconds for it to try connecting
3. If connection fails, it creates fallback hotspot
4. Connect to "Sauna Fallback" (password: `fallback123`)
5. Captive portal automatically opens
6. Enter your new WiFi credentials
7. Device will reboot and connect

**No computer needed for WiFi changes!**

#### OTA Updates (Standalone)

Even in standalone mode, you can update firmware wirelessly:

1. Compile new firmware (on any computer with ESPHome)
2. Go to `http://saunacontroller.local`
3. Click "OTA Update"
4. Upload the new `.bin` file
5. Wait for update to complete

**Or use the web flasher**:
1. Download new firmware from releases
2. Go to https://web.esphome.io/
3. Connect via USB
4. Flash new version

1. **Connect ESP32 to computer via USB**

2. **Compile and flash**:
   ```bash
   esphome run saunacontroller.yaml
   ```

3. **Select COM port** when prompted

4. **Wait for compilation and upload** (~2-5 minutes)

5. **Check logs** for sensor detection:
   ```
   [Dallas] Found sensors:
   [Dallas]   0x28FF1234567890AB
   ```

6. **Update configuration** with your sensor's address (replace `0x1234567890abcdef`)

7. **Flash again** with correct sensor address

#### Subsequent Updates (Wireless OTA)

After initial USB flash, use wireless updates:

```bash
esphome run saunacontroller.yaml --device saunacontroller.local
```

Or update through Home Assistant's ESPHome dashboard.

### Finding Your DS18B20 Address

The sensor address appears in logs during first boot:
```
[Dallas] Found sensors:
[Dallas]   0x28FF1234567890AB  <-- This is your address
```

Update this line in your config:
```yaml
sensor:
  - platform: dallas_temp
    address: 0x28FF1234567890AB  # <-- Your sensor address here
```

### Finding Your LCD I2C Address

Common addresses are `0x27` or `0x3F`. The config will scan and show in logs:
```
[i2c] Found device at address 0x27
```

If your LCD doesn't work, try changing:
```yaml
display:
  - platform: lcd_pcf8574
    address: 0x3F  # Try 0x3F if 0x27 doesn't work
```

## Safety Features

### Overheat Protection

**Trigger**: Temperature exceeds 200°F (93.3°C)
**Action**: 
- Relay immediately turns OFF
- Master switch stays ON (user intent preserved)
- Event sent to Home Assistant: `esphome.sauna_overheat`
- Display shows heater OFF

**Auto-Resume**: When temperature drops below 190°F (87.8°C) and master switch is still ON, heating automatically resumes.

### 6-Hour Auto-Shutoff

**Trigger**: 6 hours of continuous operation
**Action**:
- Master switch turns OFF
- Relay turns OFF
- Event sent to Home Assistant: `esphome.sauna_timeout`
- Prevents accidental overnight operation

### Temperature Ready Alert

**Trigger**: Temperature reaches 160°F (71°C)
**Action**:
- Event sent to Home Assistant: `esphome.sauna_ready`
- One-time notification per heating session
- Resets when you turn sauna off

## Home Assistant Integration (Optional)

**Note**: Home Assistant is **completely optional**! The sauna controller works perfectly standalone via its web interface.

However, if you use Home Assistant, you get these extra features:
- 📱 **Mobile app notifications** (sauna ready, overheat alerts)
- 🤖 **Automations** (schedule preheating, auto-shutoff rules)
- 📊 **History graphs** (temperature trends over time)
- 🗣️ **Voice control** ("Hey Google, turn on the sauna")
- 📲 **Remote access** (control from anywhere)

### Automatic Discovery

The sauna controller will automatically appear in Home Assistant once ESPHome integration is set up.

### Entities Created

- **switch.sauna** - Master control (what you toggle)
- **binary_sensor.sauna_heater** - Actual relay state (for diagnostics)
- **sensor.sauna_temperature** - Temperature in °C
- **sensor.sauna_wifi_signal** - WiFi strength
- **text_sensor.sauna_ip_address** - Device IP
- **text_sensor.sauna_wifi_ssid** - Connected network

### Setting Up Notifications

Add to your `configuration.yaml`:

```yaml
automation:
  - alias: "Sauna Ready Notification"
    trigger:
      - platform: event
        event_type: esphome.sauna_ready
    action:
      - service: notify.mobile_app_your_phone
        data:
          title: "🔥 Sauna Ready!"
          message: "{{ trigger.event.data.message }}"
          data:
            tag: "sauna_ready"
            group: "sauna"

  - alias: "Sauna Overheat Alert"
    trigger:
      - platform: event
        event_type: esphome.sauna_overheat
    action:
      - service: notify.mobile_app_your_phone
        data:
          title: "⚠️ SAUNA OVERHEAT"
          message: "{{ trigger.event.data.message }}"
          data:
            priority: high
            tag: "sauna_alert"
            group: "sauna"

  - alias: "Sauna Auto-Shutoff"
    trigger:
      - platform: event
        event_type: esphome.sauna_timeout
    action:
      - service: notify.mobile_app_your_phone
        data:
          title: "🛑 Sauna Auto-Off"
          message: "{{ trigger.event.data.message }}"
          data:
            tag: "sauna_timeout"
            group: "sauna"
```

Replace `notify.mobile_app_your_phone` with your actual notification service.

### Dashboard Card Example

```yaml
type: entities
title: Sauna Controller
entities:
  - entity: switch.sauna
    name: Sauna
  - entity: binary_sensor.sauna_heater
    name: Heater Status
  - entity: sensor.sauna_temperature
    name: Temperature
  - type: divider
  - entity: sensor.sauna_wifi_signal
    name: WiFi Signal
```

Or use a thermostat card:

```yaml
type: thermostat
entity: switch.sauna
name: Sauna
```

## Troubleshooting

### Temperature Shows NaN or No Reading

**Symptoms**: LCD shows `--°F` or logs show `[Dallas] Found no devices!`

**Solutions**:
1. **Check wiring**:
   - Yellow wire → GPIO4
   - Red wire → 3.3V
   - Black wire → GND

2. **Verify connections**:
   - If using Wago connectors, ensure wires are fully inserted
   - Check for loose connections

3. **Add pull-up resistor**:
   - 4.7kΩ between yellow wire (data) and 3.3V
   - Some probes have this built-in, some don't

4. **Try different GPIO pin**:
   - Change `pin: GPIO4` to `pin: GPIO15` or `pin: GPIO13`
   - Move yellow wire accordingly

5. **Enable internal pull-up**:
   ```yaml
   one_wire:
     - platform: gpio
       pin: 
         number: GPIO4
         mode:
           input: true
           pullup: true
   ```

6. **Check sensor address**:
   - Look in logs for actual address
   - Update in configuration

### LCD Not Working

**Blank Screen**:
- Check power: VCC to 5V (or 3.3V depending on LCD)
- Verify I2C address: try 0x27 or 0x3F
- Check contrast potentiometer on I2C backpack

**Garbled Text**:
- Wrong I2C address
- Loose SDA/SCL connections
- Try lower I2C frequency: `frequency: 100kHz`

**No Backlight**:
- Some I2C backpacks have jumper to enable/disable backlight
- Check jumper position

### Relay Not Clicking

**No Response**:
1. Check GPIO26 connection
2. Verify 5V power to relay VCC
3. Measure voltage on IN pin when toggling (should be 3.3V HIGH / 0V LOW)
4. Try inverting logic: add `inverted: true` to switch config

**Clicks But Doesn't Control Heater**:
- Check high-voltage wiring on output side
- Verify relay is rated for your heater's current
- Test with multimeter in continuity mode

### WiFi Connection Issues

**Won't Connect**:
1. Check SSID and password
2. Move closer to router
3. Check 2.4GHz network (ESP32 doesn't support 5GHz)
4. Look for fallback hotspot "Sauna Fallback"

**Frequent Disconnects**:
- Weak signal (check WiFi strength sensor)
- Router compatibility
- Add to router config: `wifi_band: 2.4ghz_only`

### ESP32 Won't Boot

**Symptoms**: LED not flashing, no serial output

**Solutions**:
1. Check power supply (needs 500mA minimum)
2. Try different USB cable
3. Press BOOT button while connecting to enter flash mode
4. Check for short circuits

### Display Shows Wrong Time

**Issue**: Time is incorrect or not showing

**Solutions**:
1. Verify Home Assistant connection
2. Check timezone setting in config
3. Ensure Home Assistant time is correct
4. Internet connection required for NTP sync

## Maintenance

### Regular Checks
- **Weekly**: Verify temperature readings are accurate
- **Monthly**: Clean LCD display
- **Quarterly**: Inspect wiring connections
- **Annually**: Test all safety features

### Testing Safety Features

**Overheat Protection Test**:
1. Set up monitoring
2. Allow sauna to reach 195°F
3. Verify relay turns off at 200°F
4. Verify relay turns back on below 190°F

**Auto-Shutoff Test**:
1. Turn sauna on
2. Set timer for 6 hours
3. Verify automatic shutoff

⚠️ **Never disable safety features in production use**

## Project Structure

```
sauna-controller/
├── README.md                    # This file
├── saunacontroller.yaml         # Main ESPHome configuration
├── secrets.yaml.example         # Template for WiFi credentials
├── docs/
│   ├── wiring-diagram.png       # Detailed wiring diagram
│   ├── assembly-guide.md        # Step-by-step assembly
│   └── troubleshooting.md       # Extended troubleshooting
├── hardware/
│   ├── enclosure-base.stl       # 3D printable base
│   ├── enclosure-lid.stl        # 3D printable lid
│   └── mounting-template.pdf    # Drilling template
└── home-assistant/
    ├── automations.yaml         # Example HA automations
    ├── dashboard.yaml           # Example dashboard card
    └── scripts.yaml             # Helper scripts
```

## Future Enhancements

Potential improvements for v2.0:
- [ ] Add humidity sensor (DHT22/BME280)
- [ ] Implement PID temperature control
- [ ] Add physical buttons for manual control
- [ ] RGB status LED
- [ ] Door sensor integration
- [ ] Energy monitoring
- [ ] Scheduling preheating
- [ ] Voice control integration
- [ ] Mobile app with push notifications

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Built with [ESPHome](https://esphome.io/)
- Integrates with [Home Assistant](https://www.home-assistant.io/)
- Thanks to the open-source community for libraries and support

## Support

If you encounter issues:
1. Check the [Troubleshooting](#troubleshooting) section
2. Review ESPHome logs for errors
3. Open an issue on GitHub with:
   - Your configuration file (redact WiFi credentials)
   - Complete error logs
   - Photos of wiring if relevant

## Safety Disclaimer

⚠️ **IMPORTANT**: This project involves:
- High voltage electrical work (120V/240V)
- Control of heating elements
- Potential fire hazards if improperly installed

**You are responsible for**:
- Following local electrical codes
- Proper installation and testing
- Regular maintenance and inspection
- Understanding your sauna's requirements

**Recommendations**:
- Have a licensed electrician review your installation
- Install proper circuit breakers and GFCI protection
- Never leave sauna unattended while heating
- Ensure proper ventilation
- Keep fire extinguisher nearby

**The author assumes no liability for damages, injuries, or losses resulting from use of this project.**

---

**Version**: 1.0.0
**Last Updated**: January 2026
**Author**: Rob Waldhauser
