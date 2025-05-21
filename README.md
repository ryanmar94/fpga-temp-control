# FPGA Temperature Control System – DE1-SOC

This project was developed as part of my Advanced Embedded Systems Design course. It demonstrates a real-time temperature control system implemented on an FPGA using a DE1-SOC development board.

## 🔧 Project Summary

The goal of this project was to maintain the temperature of the FPGA chip on the DE1-SOC board by dynamically controlling a fan using a PID (Proportional-Integral-Derivative) controller and PWM (Pulse Width Modulation). The system uses an analog temperature sensor to monitor the chip's temperature and adjusts fan speed accordingly.

## 📌 Features

- PID control algorithm implemented from scratch
- PWM generation for variable-speed fan control
- Analog-to-Digital Converter (ADC) integration for temperature sensing
- Real-time data processing and hardware interfacing

## 🛠️ Tools & Technologies

- **Board**: DE1-SOC FPGA development board
- **Languages**: Embedded C, some Verilog for board-level interface (if applicable)
- **Hardware Components**: Temperature sensor, fan, breadboard wiring
- **Software**: Quartus Prime, ModelSim, VS Code

## 📚 What I Learned

- Hands-on experience with real-time embedded control
- How to design, test, and debug PID control systems
- Interfacing analog components with FPGA platforms
- Balancing software logic with hardware constraints

## 🚀 How to Run

> NOTE: This project requires a DE1-SOC board and basic lab hardware (temperature sensor, fan, jumper wires).

1. Connect the temperature sensor and fan to the appropriate GPIO pins on the DE1-SOC.
2. Upload the firmware using Quartus or relevant development tools.
3. Monitor temperature data via serial output (if included) or visual indicators.
4. Adjust PID constants in code to tune the system response.

## 📎 Notes

This project was a capstone-style assignment that combined my hardware and software engineering skills. It illustrates my ability to build embedded systems from the ground up using both theoretical knowledge and hands-on experimentation.

---

## 📫 Contact

**Ryan Marcum**  
[LinkedIn](https://www.linkedin.com/in/ryan-marcum-946924b8/)  
ryanmar94@gmail.com