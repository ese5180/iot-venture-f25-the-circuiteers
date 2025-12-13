---
layout: default
---

# VendGuard: Smart Health Monitoring for Vending Machines

**Team:** The Circuiteers  
**Members:** Kevin Wang, Shun Yao, Zheyu Li, Megha Mistry  
**Course:** ESE5180: IoT Wireless, Security, & Scaling (Fall 2025)

---

## Project Overview

**VendGuard** is a plug-and-play IoT solution designed to transform vending machine maintenance from reactive to predictive. By using sensors to monitor critical components—such as power draw, temperature, and dispensing motor function—VendGuard helps operators identify potential failures before they occur.

The vending machine industry represents a significant global market where maintenance costs can severely impact profitability. Traditional maintenance is often reactive and expensive. VendGuard aims to reduce these costs by up to 25% through real-time health monitoring and predictive analytics.

### Key Features
*   **Edge Device:** Battery-backed sensor node (STM32WL55JC) monitoring temperature, humidity, pressure and vibration.
*   **LoRaWAN Connectivity:** Long-range, low-power communication for distributed fleets.
*   **Cloud Platform:** Centralized dashboard for real-time status and alerts using AWS IoT.
*   **Secure:** End-to-end security with signed firmware updates (FUOTA) and secure boot.

---

## Final Report

### 1. Successes (3.14.2)
*What parts of your project would you consider a success? Why?*

> We successfully implemented a complete end-to-end IoT data pipeline and secure over-the-air firmware update mechanism:
>
> **Complete Data Flow Architecture:** We established a robust data acquisition and transmission pipeline spanning from edge sensor nodes to cloud infrastructure. The system seamlessly integrates sensor data collection at the MCU level, LoRaWAN transmission through gateway infrastructure, AWS cloud processing, and real-time alert delivery to mobile endpoints. This demonstrates successful system integration across heterogeneous platforms.
>
> **Secure Firmware Update Over The Air (FUOTA):** We implemented a production-grade FUOTA mechanism leveraging LoRaWAN as the transport layer with MCUboot as the secure bootloader. Despite the inherent bandwidth constraints of LoRaWAN and packet loss during transmission, the system reliably delivers and verifies new firmware images using ECDSA P-256 signatures, enabling secure device updates without physical access—a critical capability for large-scale IoT deployments.
>
> **Robust Hardware Enclosure Design:** The device enclosure was engineered to provide secure sensor mounting while maintaining optimal sensor exposure within the vending machine environment. The design ensures mechanical reliability during transit and operation while preserving measurement accuracy for temperature and vibration sensors.

### 2. Challenges & Failures (3.14.3)
*What parts of your project didn't go well? Why?*

> Several technical and design challenges emerged during the development cycle that warrant further investigation and optimization:
>
> **Power Budget Constraints:** The current device's power consumption profile has not been fully validated in extended field deployments. Initial calculations suggest a battery lifetime of 5-6 months under typical usage patterns, which may be insufficient for certain deployment scenarios where annual battery replacement is preferred. This necessitates further optimization of the power consumption profile.
>
> **Limited Power Management Implementation:** The current firmware lacks comprehensive sleep state management for periodic sensor operations. Implementing event-driven sensor activation and dynamic sleep cycles could significantly reduce overall power consumption and extend operational lifetime. The MCU remains in active mode for extended periods that could be optimized through intelligent scheduling.
>
> **Cloud Data Processing Limitations:** The current AWS data pipeline implements threshold-based alerting for anomaly detection. To provide more actionable insights and improve predictive maintenance capabilities, the system could benefit from machine learning models capable of detecting subtle failure patterns and predicting component degradation before critical failures occur.
>
> **Firmware Image Size Constraints:** The MCU's limited on-chip flash memory restricts the size of firmware updates that can be transmitted over FUOTA. Migration to a device with increased flash capacity would enable more feature-rich firmware updates and reduce deployment complexity.


### 3. Retrospective (3.14.4)
*If you had to do it again, how might you change your development approach given the finite time and money resources?*

> The FUOTA implementation emerged as the most time-intensive and architecturally complex component of the project. In retrospect, several development strategies could have improved efficiency:
>
> **Earlier Network Simulation:** We would have prioritized the development of a simulation framework for LoRaWAN packet transmission and loss scenarios before hardware implementation. This would have enabled earlier identification of timeout behavior, packet fragmentation challenges, and retransmission logic that required extensive debugging on physical hardware.
>
> **Incremental Integration Testing:** Rather than treating FUOTA as a monolithic integration task, decomposing the LoRa stack integration into smaller, independently testable components earlier in the cycle would have accelerated troubleshooting and reduced critical path delays.
>
> **Hardware Selection Evaluation:** Conducting a more rigorous evaluation of MCU alternatives with larger flash memory and better power management capabilities during the initial design phase could have eliminated several late-stage constraints that emerged during development.


### 4. System Design Changes (3.14.5)
*Would you change your system design after this development cycle?*

> Several design modifications would be considered for a production iteration based on operational experience and market feedback:
>
> **Communication Protocol Evaluation:** While LoRaWAN provided excellent range characteristics and power efficiency for remote deployments, cellular connectivity (NB-IoT or LTE-M) could offer superior bandwidth and lower latency for FUOTA operations if power can be sourced directly from the vending machine infrastructure. This trade-off should be evaluated based on specific deployment scenarios and machine capability.
>
> **Supply Chain and Integration Strategy:** The current design operates as a third-party retrofit solution, which introduces integration constraints and duplicative subsystems (independent power management, mechanical mounting). Strategic partnerships with vending machine manufacturers would enable deeper integration, larger sensor placement flexibility, and leveraging of existing power infrastructure—potentially reducing both cost and complexity.
>
> **Sensor Placement Optimization:** The current vibration sensor placement provides good overall machine health monitoring. However, positioning the sensor in closer proximity to high-frequency mechanical systems (compressor, dispensing motors) could improve early fault detection sensitivity and reduce false positive alerts from ambient vibrations.
>
> **Multi-Sensor Fusion:** Integration of additional sensing modalities (acoustic analysis, power draw signatures) alongside temperature and vibration could enable more sophisticated fault pattern recognition and improve the precision of predictive analytics.


---

## Media & Demonstration

### Final Product Hardware (3.14.6)
*Images of your final product hardware implementation.*

> **TODO:** Add photos of your device here. Ideally, take photos in a realistic environment (e.g., inside or attached to a vending machine) or use the light box in the Detkin Lab.
>
> `![Device Photo 1](images/placeholder_device.jpg)`

### Project Image (3.14.7)

![Project Icon](images/project_icon_400x400.png)

### Video Demo (3.14.8)

*A <=5 minute video demonstrating the product.*

> **TODO:** Add a link or embed your video here.
>
> [Watch our Demo Video](YOUR_VIDEO_LINK_HERE)

---

## Technical Architecture

### Device Block Diagram
![Device Block Diagram](images/Device_diagram.png)

### Communication Architecture
![Communication Architecture](images/Communication_diagram.png)

### Firmware & Security
Our system implements a secure dual-slot bootloading architecture using MCUboot. We support **Firmware Update Over The Air (FUOTA)** using LoRaWAN to ensure devices can be maintained remotely and securely.

*   **Security:** ECDSA P-256 signatures for firmware verification.
*   **Failover:** Dual-slot architecture with automatic rollback.

![UART Boot](images/uart_boot.jpg)

---

## Source Code

The complete source code for our firmware and software is available in our GitHub repository:

[**View Source Code on GitHub**](https://github.com/ese5180/iot-venture-f25-the-circuiteers)
