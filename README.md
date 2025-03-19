![Bucky](https://raw.githubusercontent.com/rylena/Bucky/refs/heads/main/Bucky.png)

# Bucky

Bucky is a Bluetooth-enabled keystroke injector built with an ESP32, allowing remote execution of keyboard inputs on Windows, Linux, and macOS. It emulates a Bluetooth keyboard, supporting commands like text input, key combinations, delays, and Ducky Script for automation. Ideal for security testing and automation, Bucky enables users to execute scripts wirelessly via the serial monitor. learn more here:https://rylena.github.io/Bucky/


# Quick Start Demo

![Demo Preview](https://raw.githubusercontent.com/rylena/Bucky/refs/heads/main/demo.gif)

# Table of Contents

- [Project Title](#project-title)
- [Quick Start Demo](#quick-start-demo)
- [Table of Contents](#table-of-contents)
- [Installation](#installation)
- [Usage](#usage)


# Installation
[(Back to top)](#table-of-contents)

### 1. Install Arduino IDE  
Download and install the [Arduino IDE](https://www.arduino.cc/en/software) on your computer.  

### 2. Install ESP32 Board Support  
1. Open **File** → **Preferences**  
2. Add the following URL to **Additional Board Manager URLs:**https://dl.espressif.com/dl/package_esp32_index.json
3. Open **Tools** → **Board** → **Boards Manager**, search for "ESP32", and install it.  

### 3. Install Required Library  
1. Download the **ESP32-BLE-Keyboard** library from [GitHub](https://github.com/T-vK/ESP32-BLE-Keyboard).  
2. In Arduino IDE, go to **Sketch** → **Include Library** → **Add .ZIP Library**, then select the downloaded ZIP file.  

### 4. Upload the Code to ESP32  
1. Connect the ESP32 to your computer via USB.  
2. Select the correct **Board** (ESP32 Dev Module) and **Port** in Arduino IDE.
3. Download the code from ([Releases](https://github.com/rylena/Bucky/releases/tag/bucky))
4. Click **Upload** to flash the code.  

### 5. Open Serial Monitor  
- Use **PuTTY** or the Arduino Serial Monitor to connect at **115200 baud rate**.  
- Once initialized, "Bucky" will appear in the list of available Bluetooth devices.  


# Usage
[(Back to top)](#table-of-contents)

## Available Commands  

### **Basic Commands**  
- **`clear`** – Clears the terminal screen.  
- **`pair`** – Scans for nearby Bluetooth devices.  
- **`rename <name>`** – Renames the Bluetooth device.  

### **Typing Commands**  
- **`STRING <text>`** – Types the specified text.  
- **`SPACE`** – Presses the spacebar.  
- **`ENTER`** – Presses the Enter key.  
- **`DELAY <ms>`** – Waits for the specified milliseconds before continuing.  

### **Key Combination Commands**  
- **`WIN/META`** – Presses the Windows (Meta) key.  
- **`WIN/META <key>`** – Performs a Windows/Meta key combination (e.g., `WIN r`).  
- **`CTRL/ALT/SHIFT <key>`** – Sends a key combination (e.g., `CTRL c` for Copy).  

### **Script Mode**  
- **`script`** – Enables script mode for pasting and executing **Ducky Script**.  

---

## Usage Example  

1. Connect to Bucky via Bluetooth.  
2. Open a terminal or text editor.  
3. Run the following commands:  
