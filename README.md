# 🚀 TANK DRIVE: Next-Gen nRF Control System

![Project Status](https://img.shields.io/badge/Status-Active-brightgreen) ![Platform](https://img.shields.io/badge/Platform-ESP32%20%7C%20ESP8266-blue) ![Communication](https://img.shields.io/badge/Comm-nRF24L01%20%2B%20WiFi-orange) 

Ek ultra-smooth, high-frequency (5000Hz) differential tank drive system jise control karne ke liye kisi bhi **bulky physical remote ki zaroorat nahi hai**. Yah project traditional RC remote controllers ko poori tarah se replace karta hai aur aapke **Mobile Phone** ko hi ek powerful, zero-latency transmitter bana deta hai.

---

## 🌟 The Innovation (Naya Kya Hai?)

Normal nRF24 projects mein aapko ek physical remote (joystick, Arduino, aur nRF module) banana padta hai jise hath mein pakadna padta hai. **Lekin is invention mein hardware remote ko khatam kar diya gaya hai!**

Humara **Transmitter (TX) ESP8266** ek local WiFi Hotspot banata hai. Aap apna smartphone connect karte hain aur browser mein ek **Virtual Web Joystick UI** open ho jata hai. Aap phone par touch se control karte hain, aur ESP8266 us touch data ko ultra-fast nRF24L01 signal mein convert karke sidha Receiver ko bhej deta hai. 

**Result:** *Professional control, zero extra physical hardware, aur fully mobile-operated!*

---

## ⚙️ Kaise Kaam Karta Hai? (Architecture & Workflow)

Yeh system 2 hisson mein banta hua hai aur Tank Drive (Differential Steering) logic par kaam karta hai:

### 1. Transmitter Side (The Virtual Remote - ESP8266)
* **WiFi Access Point:** ESP8266 ek `ESP-REMOTE` naam ka WiFi network banata hai.
* **Web UI Joystick:** Mobile se connect karne par ek custom HTML/JS dual-joystick interface load hota hai.
* **Auto-Center Logic:** Screen par drag chhodte hi joysticks automatically middle (0) par snap ho jate hain. (Up = +100, Down = -100).
* **nRF Transmission:** Yeh data ESP8266 ke through nRF24L01 module se 250KBPS ki speed par broadcast hota hai.

### 2. Receiver Side (The Tank - ESP32)
* **Signal Reception:** ESP32 par laga nRF24L01 module us data ko receive karta hai.
* **Smart Mixing Logic:** Aage/Piche (Throttle) aur Left/Right (Steering) values ko process karke ESP32 khud decide karta hai ki kis motor ko kitni speed deni hai.
* **4-Channel Pure PWM (5000Hz):** Bina kisi external motor driver library ke, ESP32 direct 4 independent PWM signals generate karta hai:
    * Motor 1: `Forward PWM` & `Reverse PWM` (Pins 25, 26)
    * Motor 2: `Forward PWM` & `Reverse PWM` (Pins 32, 33)
* **Failsafe Security:** Agar signal toot jaye, toh 500ms ke andar saari motors automatically `0` (OFF) ho jayengi.

---

## ⚡ Core Features

* 📱 **Zero Physical Hardware Remote:** Tumhara smartphone hi tumhara remote hai.
* 🔄 **True Differential Mixing:** Gaadi apni jagah par 360° ghoom sakti hai (Tank Steering).
* 🛡️ **Auto-Failsafe:** Connection loss hone par gaadi turant ruk jayegi.
* ⚡ **High-Frequency Control:** 5000Hz PWM frequency se ultra-smooth motor rotation aur zero noise.
* 🚦 **Smart Signal Routing:** Ek motor ke Forward aur Reverse PWM kabhi bhi ek sath ON nahi hote, preventing short-circuits.

---

## 🛠️ Hardware Requirements

* **1x ESP8266** (Transmitter/Web Server ke liye)
* **1x ESP32** (Receiver/Car processing ke liye)
* **2x nRF24L01 Modules** (Fast communication ke liye)
* **Motor Controller/ESCs** (Signal receive karne aur power dene ke liye)
* **Motors** & Power Supply

---

## 🔌 Pin Mapping

### Transmitter (ESP8266)
| Component | ESP8266 Pin |
| :--- | :--- |
| nRF24 CE | D2 |
| nRF24 CSN | D1 |
| SPI Pins | D5 (SCK), D6 (MISO), D7 (MOSI) |

### Receiver (ESP32)
| Component / Output | ESP32 Pin |
| :--- | :--- |
| nRF24 CE | GPIO 4 |
| nRF24 CSN | GPIO 5 |
| SPI Pins | GPIO 18 (SCK), 19 (MISO), 23 (MOSI)|
| Left Motor (Forward) | GPIO 25 |
| Left Motor (Reverse) | GPIO 26 |
| Right Motor (Forward)| GPIO 32 |
| Right Motor (Reverse)| GPIO 33 |
