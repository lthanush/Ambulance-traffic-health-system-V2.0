# Automatic Triggering of Traffic Signal and Hospital Assistance System for Ambulance

A complete IoT-based emergency response system that combines RFID-triggered traffic signal control with real-time patient health monitoring. The system enables ambulances to clear traffic automatically while transmitting vital signs to hospitals.

---

## System Overview

This project consists of two integrated subsystems:

1. **Automatic Traffic Signal Control**: RFID tags on ambulances trigger traffic signals via ZigBee wireless communication
2. **Health Assistance System**: Real-time monitoring of patient vitals (heart rate and SpO2) transmitted to hospitals via WiFi

---

## System Architecture

### Transmitter Side (Ambulance)
- **RFID Tag Emulation**: Button interface for road selection
- **ZigBee Module**: Wireless transmission to traffic junction
- **Health Monitoring**: ESP32 with MAX30102 pulse oximeter
- **OLED Display**: Real-time vital signs display
- **WiFi Module**: Data transmission to hospital servers

### Receiver Side (Traffic Junction)
- **ZigBee Receiver**: Receives ambulance approach signals
- **Traffic Controller**: Arduino Mega 2560 managing 4-way intersection
- **LCD Display**: Shows current traffic status
- **LED Arrays**: 12 LEDs (Red, Yellow, Green × 4 roads)

---

## Hardware Requirements

### Ambulance Transmitter Unit

| Component | Part Name/Module | Quantity | Purpose |
|---|---|---|---|
| Microcontroller | Arduino UNO | 1 | Controls RFID tag simulation and ZigBee |
| Wireless Module | XBee S2C or HC-12 433MHz | 1 | Transmits road clearance signals |
| Push Buttons | Tactile push button | 4 | Road selection (testing/manual trigger) |
| Power Supply | 9V battery or 5V adapter | 1 | Powers Arduino UNO |

### Health Monitoring Unit

| Component | Part Name/Module | Quantity | Purpose |
|---|---|---|---|
| Microcontroller | ESP32 DevKit V1 | 1 | Processes sensor data and WiFi transmission |
| Pulse Oximeter | MAX30102 or MAX30100 module | 1 | Measures heart rate and SpO2 |
| Display | SSD1306 OLED 128x64 I2C | 1 | Shows vital signs locally |
| Power Supply | 5V USB power bank | 1 | Powers ESP32 |

### Traffic Junction Receiver Unit

| Component | Part Name/Module | Quantity | Purpose |
|---|---|---|---|
| Microcontroller | Arduino Mega 2560 | 1 | Main traffic controller |
| Wireless Module | XBee S2C or HC-12 433MHz | 1 | Receives ambulance signals |
| Display | 16×2 LCD (HD44780) | 1 | Shows traffic status |
| Red LEDs | 5mm LED (red) | 4 | Red signals |
| Yellow LEDs | 5mm LED (yellow) | 4 | Yellow signals |
| Green LEDs | 5mm LED (green) | 4 | Green signals |
| Resistors | 220Ω resistor | 12 | Current limiting for LEDs |
| Potentiometer | 10kΩ potentiometer | 1 | LCD contrast adjustment |
| Power Supply | 12V 2A adapter | 1 | Powers Arduino Mega |

---

## Pin Configuration

### Ambulance Transmitter (Arduino UNO)

| Component | Pin | Arduino Pin |
|---|---|---|
| Road 1 Button | Signal | 2 |
| Road 2 Button | Signal | 3 |
| Road 3 Button | Signal | 4 |
| Road 4 Button | Signal | 5 |
| ZigBee Module | TX | 1 (TX) |
| ZigBee Module | RX | 0 (RX) |

### Health Monitor (ESP32)

| Component | Pin | ESP32 Pin |
|---|---|---|
| MAX30102 | SDA | GPIO 21 |
| MAX30102 | SCL | GPIO 22 |
| OLED Display | SDA | GPIO 21 |
| OLED Display | SCL | GPIO 22 |

*Both devices share the I2C bus*

### Traffic Receiver (Arduino Mega 2560)

| Component | Pin Type | Arduino Pin |
|---|---|---|
| LCD RS | Control | 22 |
| LCD EN | Control | 23 |
| LCD D4-D7 | Data | 24-27 |
| Road 1 RED | LED | 2 |
| Road 1 YELLOW | LED | 3 |
| Road 1 GREEN | LED | 4 |
| Road 2 RED | LED | 5 |
| Road 2 YELLOW | LED | 6 |
| Road 2 GREEN | LED | 7 |
| Road 3 RED | LED | 8 |
| Road 3 YELLOW | LED | 9 |
| Road 3 GREEN | LED | 10 |
| Road 4 RED | LED | 11 |
| Road 4 YELLOW | LED | 12 |
| Road 4 GREEN | LED | 13 |
| ZigBee Module | RX | 19 (RX1) |
| ZigBee Module | TX | 18 (TX1) |

---

## Software Dependencies

### Ambulance Transmitter (Arduino UNO)
- No external libraries required (uses built-in functions only)

### Health Monitor (ESP32)
Install these libraries via Arduino Library Manager:
- `Adafruit SSD1306` (OLED display)
- `Adafruit GFX Library` (Graphics support)
- `SparkFun MAX3010x Pulse and Proximity Sensor Library` (Heart rate sensor)
- `WiFi` (Built-in ESP32 library)

### Traffic Receiver (Arduino Mega)
- `LiquidCrystal` (Built-in Arduino library)

---

## Configuration Instructions

### 1. Health Monitor WiFi Setup

Edit `health_monitoring_esp32.ino`:

```cpp
const char* ssid = "YourWiFiSSID";         // Replace with your WiFi network name
const char* password = "YourWiFiPassword"; // Replace with your WiFi password
const char* serverHost = "192.168.1.100";  // Replace with hospital server IP
const int serverPort = 80;                 // HTTP port
```

### 2. ZigBee Module Configuration

Both transmitter and receiver ZigBee modules must be configured as a matched pair:

**Transmitter (Ambulance) - Coordinator Mode:**
- PAN ID: `1234` (example - use same on both)
- Destination Address: `0000` (broadcast or specific receiver address)
- Baud Rate: `9600`

**Receiver (Traffic Junction) - End Device Mode:**
- PAN ID: `1234` (must match transmitter)
- Baud Rate: `9600`

For HC-12 modules, use AT commands via Serial monitor:
```
AT+C001        // Set channel 001
AT+B9600       // Set baud rate 9600
AT+FU3         // Set to mode FU3 (long range)
```

---

## System Operation

### Traffic Signal Control Flow

1. **Normal Cycle**: Roads cycle in sequence (1→2→3→4) with 10-second green phases
2. **RFID Detection**: When ambulance presses road button, ZigBee transmits road ID
3. **Emergency Override**: Traffic controller receives signal and:
   - Completes current yellow clearance safely
   - Turns all signals red (2-second safety gap)
   - Gives ambulance road extended 15-second green
   - Returns to normal cycle from next road (fair resumption)

### Health Monitoring Flow

1. **Sensor Reading**: MAX30102 continuously monitors heart rate and SpO2
2. **Local Display**: Real-time vitals shown on OLED display
3. **Data Transmission**: Every 5 seconds, ESP32 sends JSON data to hospital server:
   ```json
   {
     "heartRate": 75,
     "spo2": 98,
     "timestamp": 123456
   }
   ```
4. **Hospital Dashboard**: Server receives and displays patient data in real-time

---

## Timing Parameters

### Traffic Control

| Parameter | Duration | Description |
|---|---|---|
| Normal green | 10 seconds | Standard green phase |
| Normal yellow | 2 seconds | Yellow warning/clearance |
| All-red gap | 2 seconds | Safety gap between phases |
| Emergency green | 15 seconds | Extended green for ambulance |
| Emergency yellow | 3 seconds | Ambulance warning period |
| Command debounce | 500 ms | Prevents duplicate triggers |

### Health Monitoring

| Parameter | Duration | Description |
|---|---|---|
| Display update | 1 second | OLED refresh rate |
| Server transmission | 5 seconds | Data upload interval |
| Sensor sampling | 100 Hz | MAX30102 sample rate |

---

## Serial Commands

### Ambulance Transmitter → Traffic Receiver

| Command | Function |
|---|---|
| `A` | Clear Road 1 (ambulance approaching) |
| `B` | Clear Road 2 (ambulance approaching) |
| `C` | Clear Road 3 (ambulance approaching) |
| `D` | Clear Road 4 (ambulance approaching) |

---

## Testing Procedure

### 1. Test Traffic Receiver Standalone
1. Upload `traffic_receiver_zigbee.ino` to Arduino Mega
2. Connect LCD and LEDs
3. Open Serial Monitor at 9600 baud
4. Observe normal traffic cycle

### 2. Test Ambulance Transmitter
1. Upload `ambulance_transmitter.ino` to Arduino UNO
2. Connect 4 push buttons to pins 2-5 (with pull-up resistors)
3. Open Serial Monitor at 9600 baud
4. Press buttons to see commands transmitted

### 3. Test ZigBee Communication
1. Connect ZigBee modules to both Arduino boards
2. Press button on transmitter
3. Observe traffic signal override on receiver
4. Check Serial Monitor for "RFID DETECTED" message

### 4. Test Health Monitor
1. Upload `health_monitoring_esp32.ino` to ESP32
2. Connect MAX30102 and OLED display
3. Place finger on sensor
4. Verify readings on OLED display
5. Check Serial Monitor for WiFi connection and data transmission

---

## Troubleshooting

### ZigBee Communication Issues
- **No signal received**: Check baud rate matches (9600) on both modules
- **Intermittent connection**: Verify PAN ID matches on both modules
- **Range issues**: Ensure clear line of sight, check antenna connections

### Health Monitor Issues
- **"Sensor Error"**: Check I2C wiring (SDA/SCL), verify sensor address (usually 0x57)
- **No heart rate detected**: Ensure finger covers sensor completely, check LED brightness
- **WiFi connection fails**: Verify SSID/password, check network availability

### Traffic Control Issues
- **LCD blank**: Adjust contrast potentiometer, check pin connections
- **LEDs not lighting**: Check current-limiting resistors (220Ω), verify pin assignments
- **Stuck in emergency mode**: Reset Arduino, check for stuck button or ZigBee noise

---

## Hospital Server Setup (Optional)

For receiving health data, set up a simple Node.js server:

```javascript
const express = require('express');
const app = express();
app.use(express.json());

app.post('/patient-data', (req, res) => {
  console.log('Patient Data:', req.body);
  // Store in database or forward to dashboard
  res.sendStatus(200);
});

app.listen(80, () => {
  console.log('Hospital server running on port 80');
});
```

---

## Future Enhancements

- GPS integration for automatic road detection
- Multiple ambulance priority handling
- Web-based hospital dashboard with real-time graphs
- SMS alerts to hospital staff when ambulance is approaching
- Integration with city traffic management systems
- Machine learning for traffic pattern optimization

---

