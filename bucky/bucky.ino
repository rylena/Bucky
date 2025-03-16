#include <BleKeyboard.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

BleKeyboard* bleKeyboard;
BLEScan* pBLEScan;

#define MAX_COMMAND_LENGTH 256
#define HISTORY_SIZE 10
#define SERIAL_BUFFER_SIZE 1024  // Increase buffer size

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
const char *CLEAR_SCREEN = "\033[2J\033[H";  
const char *BLUE_TEXT = "\033[94m";          
const char *GREEN_TEXT = "\033[92m";  
const char *RED_TEXT = "\033[91m";    
const char *RESET_COLOR = "\033[0m";         

// Add this near the top with other global variables
String deviceName = "ESP32 Ducky"; // Default name

// Function declarations
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

void scanForDevices() {
    Serial.println("Searching for devices...");
    BLEScanResults foundDevices = pBLEScan->start(5); // Scan for 5 seconds
    
    for(int i = 0; i < foundDevices.getCount(); i++) {
        BLEAdvertisedDevice device = foundDevices.getDevice(i);
        Serial.printf("%d) %s\n", 
            i + 1,
            device.getName().c_str()
        );
    }
    
    pBLEScan->clearResults();
}

void printDuck() {
    Serial.print(BLUE_TEXT);
    Serial.println("      ,~~.");
    Serial.println("     (  9 )-_,");
    Serial.println("(\___ )=='-'");
    Serial.println(" \\ .   ) )");
    Serial.println("  \\ `-' /");
    Serial.println("   `~j-'");
    Serial.println("     \"=:");
    Serial.print(RESET_COLOR);
}

void initializeBLE() {
    BLEDevice::init(deviceName.c_str());
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(true);
    
    // Create new BleKeyboard instance
    bleKeyboard = new BleKeyboard(deviceName.c_str());
    bleKeyboard->begin();
}

// Add this new function to execute a single command
void executeSingleCommand(String command) {
    command.trim();
    
    if (command == "SPACE") {
        Serial.println("Pressing Space");
        bleKeyboard->write(' ');
    }
    else if (command == "WIN" || command == "META") {
        Serial.println("Pressing Windows/Meta key");
        bleKeyboard->press(KEY_LEFT_GUI);
        delay(100);
        bleKeyboard->releaseAll();
    }
    else if (command.startsWith("WIN ") || command.startsWith("META ")) {
        char key = command.charAt(4);
        Serial.println("Pressing Windows/Meta+" + String(key));
        bleKeyboard->press(KEY_LEFT_GUI);
        bleKeyboard->press(key);
        delay(100);
        bleKeyboard->releaseAll();
    }
    else if (command.startsWith("STRING ")) {
        String text = command.substring(7);
        Serial.println("\nTyping: " + text);
        bleKeyboard->print(text);
    }
    else if (command == "ENTER") {
        Serial.println("Pressing ENTER");
        bleKeyboard->write(KEY_RETURN);
    }
    else if (command.startsWith("DELAY ")) {
        int delayTime = command.substring(6).toInt();
        Serial.println("Delaying: " + String(delayTime) + " ms");
        delay(delayTime);
    }
    else if (command == "CTRL ALT DELETE") {
        Serial.println("Pressing CTRL+ALT+DELETE");
        bleKeyboard->press(KEY_LEFT_CTRL);
        bleKeyboard->press(KEY_LEFT_ALT);
        bleKeyboard->press(KEY_DELETE);
        delay(100);
        bleKeyboard->releaseAll();
    }
    else if (command.startsWith("CTRL ")) {
        char key = command.charAt(5);
        Serial.println("Pressing CTRL+" + String(key));
        bleKeyboard->press(KEY_LEFT_CTRL);
        bleKeyboard->press(key);
        delay(100);
        bleKeyboard->releaseAll();
    }
    else if (command.startsWith("ALT ")) {
        char key = command.charAt(4);
        Serial.println("Pressing ALT+" + String(key));
        bleKeyboard->press(KEY_LEFT_ALT);
        bleKeyboard->press(key);
        delay(100);
        bleKeyboard->releaseAll();
    }
    else if (command.startsWith("SHIFT ")) {
        char key = command.charAt(6);
        Serial.println("Pressing SHIFT+" + String(key));
        bleKeyboard->press(KEY_LEFT_SHIFT);
        bleKeyboard->press(key);
        delay(100);
        bleKeyboard->releaseAll();
    }
}

// Modify executeCommand() to handle multiple commands
void executeCommand() {
    String command = String(cmdBuffer);
    command.trim();
    
    addToHistory(command);
    currentHistoryPos = -1;

    if (command.startsWith("rename ")) {
        // Keep rename command handling here since it's system-level
        String newName = command.substring(7);
        if (newName.length() > 0) {
            deviceName = newName;
            Serial.println("Device name changed to: " + deviceName);
            
            bleKeyboard->end();
            delete bleKeyboard;
            BLEDevice::deinit();
            
            delay(500);
            initializeBLE();
            
            Serial.println("BLE reinitialized with new name");
        } else {
            Serial.println("Please provide a name after 'rename'");
        }
    }
    else if (command == "clear") {
        Serial.print(CLEAR_SCREEN);
        printDuck();
        Serial.print(BLUE_TEXT);
        Serial.println("Welcome to Bucky! Made by RylenA");
        Serial.print(RESET_COLOR);
    }
    else if (command == "pair") {
        scanForDevices();
    }
    else if (command == "help") {
        Serial.println("Available commands:");
        Serial.println("  clear    - Clear the screen");
        Serial.println("  pair     - Scan for nearby BLE devices");
        Serial.println("  rename <name> - Change device name");
        Serial.println("  STRING <text>    - Type text");
        Serial.println("  SPACE    - Press Space key");
        Serial.println("  ENTER    - Press Enter key");
        Serial.println("  DELAY <ms>       - Wait specified milliseconds");
        Serial.println("  WIN/META         - Press Windows/Meta key");
        Serial.println("  WIN/META <key>   - Windows/Meta key combination");
        Serial.println("  CTRL/ALT/SHIFT <key> - Send key combination");
        Serial.println("\nMultiple commands can be chained with semicolons:");
        Serial.println("  Example: WIN r;STRING cmd;ENTER");
    }
    else if (bleKeyboard->isConnected()) {
        // Split the command string by semicolons and execute each command
        int start = 0;
        int end = command.indexOf(';');
        
        while (start < command.length()) {
            String singleCommand;
            if (end == -1) {
                singleCommand = command.substring(start);
                start = command.length();
            } else {
                singleCommand = command.substring(start, end);
                start = end + 1;
                end = command.indexOf(';', start);
            }
            
            executeSingleCommand(singleCommand);
            delay(50); // Small delay between commands
        }
    }
    else {
        Serial.println("Bluetooth not connected");
    }
    
    cmdBuffer[0] = '\0';
    cmdIndex = 0;
    Serial.println();
    printPrompt();
}

void setup() {
    Serial.begin(115200);
    
    // Clear screen and show welcome message with duck
    Serial.print(CLEAR_SCREEN);
    printDuck();
    Serial.print(BLUE_TEXT);
    Serial.println("Welcome to Bucky! Made by RylenA");
    Serial.println("Type 'help' for available commands");
    Serial.print(RESET_COLOR);
    
    // Initialize BLE
    initializeBLE();
    printPrompt();
}

void loop() {
    while (Serial.available()) {
        char c = Serial.read();
        delay(1);  // Small delay to ensure reliable reading of pasted text
        
        switch (c) {
            case '\x1B': // Escape sequence
                handleEscapeSequence();
                break;
                
            case '\r': // Enter
            case '\n':
                if (cmdIndex > 0) {  // Only execute if there's a command
                    executeCommand();
                }
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