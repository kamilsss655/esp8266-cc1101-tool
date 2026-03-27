# 🚀 esp8266-cc1101-tool

A lightweight yet powerful **ESP8266 + CC1101 RF toolkit** for sniffing, and emulating sub-GHz devices.

Built on top of a modified **ESPiLight** library, this project lets you **receive and transmit multiple RF protocols** over **433 MHz and 868 MHz** using a simple **serial interface**.

Perfect for:
- 🏠 reverse engineering remotes
- 🌦️ reading weather stations
- 🔌 controlling RF sockets
- 🧪 experimenting with sub-GHz communication

> [!WARNING]
> By using the firmware provided, users are responsible for ensuring compliance with all local laws and regulations governing the use of such technology. The author of the firmware shall not be held liable for any misuse or unlawful activities conducted by the user. It is the user's sole responsibility to use the firmware in a legal and responsible manner. By proceeding to use the firmware, users agree to abide by all applicable laws and regulations. Please note that this firmware has been created for scientific research purposes only.
The firmware provided does not come with any form of warranty, express or implied. The author of the firmware shall not be held responsible for any damage caused as a result of using the firmware. By proceeding to use the firmware, users agree to do so at their own risk and understand that the author will not be liable for any adverse consequences that may arise. It is the user's responsibility to proceed with caution and understand the potential risks involved in using the firmware.

---

## ✨ Features

- 📡 RX/TX support for **multiple RF protocols**
- 🔄 Switchable frequency bands: **433 MHz / 868 MHz**
- 🧠 Powered by **ESPiLight protocol decoding**
- 🖥️ Simple **serial command interface**
- 💡 Live RF activity LED indicator
- ⚡ Continuous TX burst (like real remotes)

---

## 📚 Supported Protocols

Thanks to ESPiLight, this tool supports a **wide range of RF protocols**, including:

- ELRO series (300 / 400 / 800)
- Quigg (GT1000, GT7000, GT9000)
- Alecto weather stations
- KlikAanKlikUit / ArcTech
- X10
- EV1527 / SC2262
- Conrad RSL
- Livolo switches
- Nexus weather
- Silvercrest devices
- And many more...

---

## 🧰 Hardware

### Required
- ESP8266 (NodeMCU, Wemos D1 Mini, etc.)
- CC1101 RF module

### Wiring

| ESP8266 | CC1101 |
|--------|--------|
| 4      | GDO0   |
| 16     | GDO2   |
| 14     | SCK    |
| 13     | MOSI   |
| 12     | MISO   |
| 15     | CSN    |
| 3V3    | VCC    |
| GND    | GND    |

---

## ⚙️ Getting Started

1. Flash firmware to ESP8266
2. Open Serial Monitor at baud rate: 115200
3. Start receiving RF signals instantly 🎉

---

## 🖥️ Serial Interface

The tool is fully controlled via **serial input**.

### ▶️ Example session

```
15:56:14.322 -> CC1101 detected OK
15:56:16.388 -> Configuring CC1101...
15:56:16.419 -> RX on 868 Mhz band
15:56:16.419 -> 
15:56:16.419 -> To send any code you can enter valid ESPiLight command ie:
15:56:16.419 -> [elro_800_switch] {"systemcode":1,"unitcode":2,"off":1}
15:56:16.452 -> 
15:56:16.452 -> Available extra commands:
15:56:16.452 -> /band 433 - switch to 433 band
15:56:16.452 -> /band 868 - switch to 868 band
15:56:16.452 -> 
15:56:16.452 -> CC1101 configured correctly!
15:56:16.452 -> Received codes will show up here: 
16:00:18.248 -> RX: [quigg_gt1000] {"id":22,"unit":3,"on":1}
16:01:05.838 -> TX: [quigg_gt1000] {"id":22,"unit":3,"off":1}
16:01:09.034 -> RX on 868 Mhz band
16:02:54.074 -> RX: [quigg_gt1000] {"id":22,"unit":3,"on":1}
16:03:02.794 -> RX: [quigg_gt1000] {"id":22,"unit":3,"off":1}
```


---

## 📡 Receiving Signals

When a valid RF signal is detected:

```
RX: [quigg_gt1000] {"id":22,"unit":3,"on":1}
RX: [quigg_gt1000] {"id":22,"unit":3,"off":1}
```


---

## 📤 Transmitting Signals

To replay or send a signal:

`[quigg_gt1000] {"id":22,"unit":3,"off":1}`

---

## 🎛️ Commands

### Change RF Band
* `/band 433` - switch to 433 Mhz band
* `/band 868` - switch to 868 Mhz band

Example:

```
/band 433
Switching to 433 MHz...
RX on 433 Mhz band
```

---

## 🧠 How It Works

- CC1101 handles RF modulation/demodulation
- ESP8266 processes signals via ESPiLight
- Protocols are decoded into JSON
- Commands are parsed from serial input
- TX simulates real remote bursts (timed repeats)

---

## ⚡ Notes

- Boot garbage on serial is normal (ESP8266 bootloader @ 74880 baud)
- Works best with a proper antenna tuned for your band
- ESP32 should work but is currently untested

---

## 🔮 Ideas / Roadmap

- [ ] Web interface (WiFi control)
- [ ] MQTT integration
- [ ] Save & replay signals
- [ ] Protocol auto-detection improvements
- [ ] OTA updates

---

## ❤️ Credits

- Based on **ESPiLight**
- Uses **SmartRC CC1101 Driver**
- Built for scientific research

---
