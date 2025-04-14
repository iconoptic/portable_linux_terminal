# Portable Linux Terminal with Visible Light Communication Keyboard

This project is a **Raspberry Pi-powered cyberdeck** with a **detachable keyboard that transmits keystrokes using visible light communication (VLC)**. It was designed as a capstone project to explore embedded systems, optical signaling, and hardware-software co-design.

![Prototype Image](prototype.jpg)

## 🔧 Hardware Overview

- **Main Console**: Raspberry Pi 4, 5" DSI touchscreen, 3-D printed case, lithium battery + charge regulator.
- **Keyboard Module**: 
  - Raspberry Pi Pico
  - 64-key matrix with diode-protected rows/columns
  - RGB LEDs for VLC (Color Shift Keying + PWM)
  - White LED sync pulses for timing
  - Photodiode-based receiver on console side

## 📐 Architecture

```
.
├── keyboard-firmware/     # RP2040 firmware for encoding + LED signaling
├── vlc_receiver/          # Raspberry Pi 4 code for decoding photodiode signals
├── media/                 # Schematics, PCB, scope captures
├── README.md              # This file
```

## 🧠 Engineering Focus

- Custom **VLC protocol** using PWM + CSK (color shift keying)
- Clock synchronization via **white LED pulses**
- **MOSFET driver design** and RC constant tuning for LED switching
- Ambient light rejection via photodiode spectral filtering
- Safe aluminum plate-based **power delivery system**
- **3D printed case** with access to all ports + GPIO expansion

## 📸 Media & Validation

- **Oscilloscope traces** of encoded PWM signal
- **.lst register maps** of memory layout for firmware
- Scope captures of sync pulse timing validation
- Custom **firmware in C++** for the RP2040

## 💡 Technologies & Tools

- Embedded C++ (Pico SDK)
- GPIO + PWM signal generation
- Eagle/KiCad (PCB design)
- Linux shell scripting + low-level debugging
- Visible light + photodiode interfacing

## 🧪 Use Case

This project blends protocol design, real-world electrical engineering, and embedded development. It's a practical and portable proof-of-concept for wireless peripherals that communicate via optical signals, suitable for R&D or educational demonstrations in **embedded comms, optoelectronics, or secure hardware interfaces**.

## 📎 License

MIT License
