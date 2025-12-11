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
*   **Edge Device:** Battery-backed sensor node (STM32WL55JC) monitoring temperature and vibration.
*   **LoRaWAN Connectivity:** Long-range, low-power communication for distributed fleets.
*   **Cloud Platform:** Centralized dashboard for real-time status and alerts.
*   **Secure:** End-to-end security with signed firmware updates (FUOTA) and secure boot.

---

## Final Report

### 1. Successes (3.14.2)
*What parts of your project would you consider a success? Why?*

> **TODO:** Replace this text with your reflection.
> Example: We successfully implemented the LoRaWAN FUOTA update mechanism, which was a complex integration of MCUboot and the LoRa stack. This allows us to update devices remotely without physical access, a critical feature for scalable IoT deployments.

### 2. Challenges & Failures (3.14.3)
*What parts of your project didn’t go well? Why?*

> **TODO:** Replace this text with your reflection.
> Example: We struggled with the initial power budget for the vibration sensor, which consumed more current than expected in idle mode. This required us to implement a more aggressive sleep cycle, which complicated the interrupt handling logic.

### 3. Retrospective (3.14.4)
*If you had to do it again, how might you change your development approach given the finite time and money resources?*

> **TODO:** Replace this text with your reflection.
> Example: If we were to start over, we would simulate the network latency earlier in the process. We spent a lot of time debugging timeout issues on the hardware that could have been identified with a better simulation model.

### 4. System Design Changes (3.14.5)
*Would you change your system design after this development cycle?*

> **TODO:** Replace this text with your reflection. Consider:
> *   Was LoRaWAN the right choice?
> *   Would other sensors work better?
> *   Did the target market feedback suggest changes?

---

## Media & Demonstration

### Final Product Hardware (3.14.6)
*Images of your final product hardware implementation.*

> **TODO:** Add photos of your device here. Ideally, take photos in a realistic environment (e.g., inside or attached to a vending machine) or use the light box in the Detkin Lab.
>
> `![Device Photo 1](images/placeholder_device.jpg)`

### Project Image (3.14.7)
*An exactly 400x400 pixel .jpg image that captures your project.*

> **TODO:** Add your 400x400 image here.
>
> `![Project Icon](images/project_icon_400x400.jpg)`

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

## Source Code (3.14.9)

The complete source code for our firmware and software is available in our GitHub repository:

[**View Source Code on GitHub**](https://github.com/ese5180/iot-venture-f25-the-circuiteers)

---

*This report was generated for the ESE5180 Final Project.*
