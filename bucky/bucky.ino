#include <BleKeyboard.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

BleKeyboard bleKeyboard("ESP32 Ducky");
BLEScan* pBLEScan;
bool isScanning = false;
bool isStandby = false;

#define MAX_COMMAND_LENGTH 256
#define HISTORY_SIZE 10

// Command history management
String commandHistory[HISTORY_SIZE];
int historyIndex = 0;
int currentHistoryPos = -1;

// Current command buffer
char cmdBuffer[MAX_COMMAND_LENGTH];
int cmdIndex = 0;

// ANSI escape codes
const char *CLEAR_LINE = "\033[2K\r";
const char *CURSOR_LEFT = "\033[D";
const char *CLEAR_SCREEN = "\033[2J\033[H";  // Clear screen and move cursor to home position
const char *BLUE_TEXT = "\033[94m";          // Set text color to light blue
const char *RESET_COLOR = "\033[0m";         // Reset text color

// Add these new color codes
const char *GREEN_TEXT = "\033[92m";  // Bright green
const char *RED_TEXT = "\033[91m";    // Bright red

void setup() {
    Serial.begin(115200);
    
    // Clear screen and show welcome message in blue
    Serial.print(CLEAR_SCREEN);
    Serial.print(BLUE_TEXT);
    Serial.println("Welcome to Bucky! Made by RylenA");
    Serial.println("Type 'help' for available commands");
    Serial.print(RESET_COLOR);
    
    // Initialize BLE scanning
    BLEDevice::init("ESP32 Scanner");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(true);
    
    printPrompt();
}

void printPrompt() {
    Serial.print("\033[0m$ ");  // Reset color and show prompt
}

void clearLine() {
    Serial.print(CLEAR_LINE);
    printPrompt();
}

void handleBackspace() {
    if (cmdIndex > 0) {
        cmdIndex--;
        cmdBuffer[cmdIndex] = '\0';
        Serial.print("\b \b");  // Erase character from terminal
    }
}

void addToHistory(const String& command) {
    if (command.length() > 0) {
        // Shift history
        for (int i = HISTORY_SIZE - 1; i > 0; i--) {
            commandHistory[i] = commandHistory[i-1];
        }
        commandHistory[0] = command;
        if (historyIndex < HISTORY_SIZE) historyIndex++;
    }
}

void navigateHistory(bool up) {
    if (up) {
        if (currentHistoryPos < historyIndex - 1) {
            currentHistoryPos++;
            clearLine();
            strncpy(cmdBuffer, commandHistory[currentHistoryPos].c_str(), MAX_COMMAND_LENGTH - 1);
            cmdIndex = strlen(cmdBuffer);
            Serial.print(cmdBuffer);
        }
    } else {
        if (currentHistoryPos > -1) {
            currentHistoryPos--;
            clearLine();
            if (currentHistoryPos == -1) {
                cmdBuffer[0] = '\0';
                cmdIndex = 0;
            } else {
                strncpy(cmdBuffer, commandHistory[currentHistoryPos].c_str(), MAX_COMMAND_LENGTH - 1);
                cmdIndex = strlen(cmdBuffer);
                Serial.print(cmdBuffer);
            }
        }
    }
}

void handleEscapeSequence() {
    if (Serial.available() >= 2) {
        char second = Serial.read();
        char third = Serial.read();
        
        if (second == '[') {
            switch (third) {
                case 'A': // Up arrow
                    navigateHistory(true);
                    break;
                case 'B': // Down arrow
                    navigateHistory(false);
                    break;
                // Add left/right navigation if desired
            }
        }
    }
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        Serial.printf("Device found: %s\n", advertisedDevice.toString().c_str());
    }
};

void scanForDevices() {
    Serial.println("Scanning for BLE devices...");
    BLEScanResults foundDevices = pBLEScan->start(5); // Scan for 5 seconds
    Serial.print(BLUE_TEXT);
    Serial.printf("Devices found: %d\n", foundDevices.getCount());
    Serial.print(RESET_COLOR);
    
    for(int i = 0; i < foundDevices.getCount(); i++) {
        BLEAdvertisedDevice device = foundDevices.getDevice(i);
        Serial.printf("%d) Name: %s   Address: %s   RSSI: %d\n", 
            i + 1,
            device.getName().c_str(),
            device.getAddress().toString().c_str(),
            device.getRSSI()
        );
    }
    
    Serial.println("\nEnter 'connect <number>' to connect to a device");
    pBLEScan->clearResults();
}

void executeCommand() {
    String command = String(cmdBuffer);
    command.trim();
    
    addToHistory(command);
    currentHistoryPos = -1;
    
    if (command == "clear") {
        Serial.print(CLEAR_SCREEN);
        Serial.print(BLUE_TEXT);
        Serial.println("Welcome to Bucky! Made by RylenA");
        Serial.print(RESET_COLOR);
    }
    else if (command == "standby") {
        isStandby = true;
        bleKeyboard.begin();
        Serial.println("Waiting for connections...");
    }
    else if (command == "pair") {
        scanForDevices();
    }
    else if (command.startsWith("connect ")) {
        int deviceNum = command.substring(8).toInt();
        // Simulate connection attempt (you'll need to implement actual connection logic)
        if (deviceNum > 0) {
            // Simulating success/failure randomly for demonstration
            if (random(2) == 0) {
                Serial.print(GREEN_TEXT);
                Serial.println("Connected to device successfully!");
                Serial.print(RESET_COLOR);
            } else {
                Serial.print(RED_TEXT);
                Serial.println("Connection to device failed!");
                Serial.print(RESET_COLOR);
            }
        }
    }
    else if (command == "help") {
        Serial.println("Available commands:");
        Serial.println("  clear    - Clear the screen");
        Serial.println("  standby  - Enter standby mode and wait for connections");
        Serial.println("  pair     - Scan for nearby BLE devices");
        Serial.println("  connect <number> - Connect to a specific device");
        Serial.println("  STRING <text>    - Type text");
        Serial.println("  ENTER    - Press Enter key");
        Serial.println("  DELAY <ms>       - Wait specified milliseconds");
        Serial.println("  CTRL/ALT/SHIFT <key> - Send key combination");
    }
    else if (isStandby && bleKeyboard.isConnected()) {
        // Your existing keyboard command handlers
        if (command.startsWith("STRING ")) {
            String text = command.substring(7);
            Serial.println("\nTyping: " + text);
            bleKeyboard.print(text);
        }
        else if (command == "ENTER") {
            Serial.println("Pressing ENTER");
            bleKeyboard.write(KEY_RETURN);
        }
        else if (command.startsWith("DELAY ")) {  // Pause execution
            int delayTime = command.substring(6).toInt();
            Serial.println("Delaying: " + String(delayTime) + " ms");
            delay(delayTime);
        }
        else if (command == "CTRL ALT DELETE") {  // Special key combo
            Serial.println("Pressing CTRL+ALT+DELETE");
            bleKeyboard.press(KEY_LEFT_CTRL);
            bleKeyboard.press(KEY_LEFT_ALT);
            bleKeyboard.press(KEY_DELETE);
            delay(100);
            bleKeyboard.releaseAll();
        }
        else if (command.startsWith("CTRL ")) {  // CTRL + <Key>
            char key = command.charAt(5);  // Get character after CTRL
            Serial.println("Pressing CTRL+" + String(key));
            bleKeyboard.press(KEY_LEFT_CTRL);
            bleKeyboard.press(key);  // Sends the key as ASCII
            delay(100);
            bleKeyboard.releaseAll();
        }
        else if (command.startsWith("ALT ")) {  // ALT + <Key>
            char key = command.charAt(4);  // Get character after ALT
            Serial.println("Pressing ALT+" + String(key));
            bleKeyboard.press(KEY_LEFT_ALT);
            bleKeyboard.press(key);  // Sends the key as ASCII
            delay(100);
            bleKeyboard.releaseAll();
        }
        else if (command.startsWith("SHIFT ")) {  // SHIFT + <Key>
            char key = command.charAt(6);  // Get character after SHIFT
            Serial.println("Pressing SHIFT+" + String(key));
            bleKeyboard.press(KEY_LEFT_SHIFT);
            bleKeyboard.press(key);  // Sends the key as ASCII
            delay(100);
            bleKeyboard.releaseAll();
        }

        Serial.println("Executed: " + command);  // Debugging message
    }
    else if (!isStandby) {
        Serial.println("Please enter 'standby' mode first to enable BLE functionality");
    }
    else {
        Serial.println("Not connected to any device. Use 'pair' to find devices.");
    }
    
    cmdBuffer[0] = '\0';
    cmdIndex = 0;
    Serial.println();
    printPrompt();
}

void loop() {
    if (Serial.available()) {
        char c = Serial.read();
        
        switch (c) {
            case '\x1B': // Escape sequence
                handleEscapeSequence();
                break;
                
            case '\r': // Enter
            case '\n':
                executeCommand();
                break;
                
            case '\b': // Backspace
            case 0x7F: // Delete
                handleBackspace();
                break;
                
            default:
                if (cmdIndex < MAX_COMMAND_LENGTH - 1 && c >= 32 && c <= 126) {
                    cmdBuffer[cmdIndex++] = c;
                    cmdBuffer[cmdIndex] = '\0';
                    Serial.print(c);
                }
                break;
        }
    }
}