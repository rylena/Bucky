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
String deviceName = "Bucky"; // Default name
bool readingScript = false;
String scriptBuffer = "";

// Add these function declarations near the top with other function declarations
void pressSpace(String);
void pressWinKey(String);
void pressWinWithKey(String);
void typeString(String);
void pressEnter(String);
void waitDelay(String);
void pressCtrlAltDelete(String);
void pressCtrlWithKey(String);
void pressAltWithKey(String);
void pressShiftWithKey(String);

// Add near the top with other function declarations
void executeCommand(String command);
void executeCommand();

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
    BLEScanResults foundDevices = *pBLEScan->start(5);
    
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

// Add these function definitions before executeSingleCommand
void pressSpace(String) {
    bleKeyboard->write(' ');
}

void pressWinKey(String) {
    bleKeyboard->press(KEY_LEFT_GUI);
    delay(100);
    bleKeyboard->releaseAll();
}

void pressWinWithKey(String param) {
    bleKeyboard->press(KEY_LEFT_GUI);
    bleKeyboard->press(param[0]);
    delay(100);
    bleKeyboard->releaseAll();
}

void typeString(String text) {
    bleKeyboard->print(text);
}

void pressEnter(String) {
    bleKeyboard->write(KEY_RETURN);
}

void waitDelay(String param) {
    delay(param.toInt());
}

void pressCtrlAltDelete(String) {
    bleKeyboard->press(KEY_LEFT_CTRL);
    bleKeyboard->press(KEY_LEFT_ALT);
    bleKeyboard->press(KEY_DELETE);
    delay(100);
    bleKeyboard->releaseAll();
}

void pressCtrlWithKey(String param) {
    bleKeyboard->press(KEY_LEFT_CTRL);
    bleKeyboard->press(param[0]);
    delay(100);
    bleKeyboard->releaseAll();
}

void pressAltWithKey(String param) {
    bleKeyboard->press(KEY_LEFT_ALT);
    bleKeyboard->press(param[0]);
    delay(100);
    bleKeyboard->releaseAll();
}

void pressShiftWithKey(String param) {
    bleKeyboard->press(KEY_LEFT_SHIFT);
    bleKeyboard->press(param[0]);
    delay(100);
    bleKeyboard->releaseAll();
}

// Modify executeSingleCommand to use the global functions
void executeSingleCommand(String command) {
    command.trim();
    
    // Define command map structure
    struct CommandAction {
        const char* name;
        void (*action)(String);
    };

    // Define command array
    static const CommandAction commands[] = {
        {"SPACE", pressSpace},
        {"WIN", pressWinKey},
        {"META", pressWinKey},
        {"WIN ", pressWinWithKey},
        {"META ", pressWinWithKey},
        {"STRING ", typeString},
        {"ENTER", pressEnter},
        {"DELAY ", waitDelay},
        {"CTRL ALT DELETE", pressCtrlAltDelete},
        {"CTRL ", pressCtrlWithKey},
        {"ALT ", pressAltWithKey},
        {"SHIFT ", pressShiftWithKey}
    };

    // Find and execute the command
    for (const CommandAction& cmd : commands) {
        if (command.startsWith(cmd.name)) {
            String param = command.substring(strlen(cmd.name));
            Serial.println("Executing: " + command);
            cmd.action(param);
            return;
        }
    }
}

void executeScript(String script) {
    Serial.println("Executing script...");
    
    // Split the script into lines
    int start = 0;
    int end = script.indexOf('\n');
    
    while (start < script.length()) {
        String line;
        if (end == -1) {
            line = script.substring(start);
            start = script.length();
        } else {
            line = script.substring(start, end);
            start = end + 1;
            end = script.indexOf('\n', start);
        }
        
        line.trim();
        if (line.length() > 0 && !line.startsWith("//")) {  // Skip empty lines and comments
            Serial.println("> " + line);
            executeCommand(line);  // Use existing command execution
            delay(50);  // Small delay between commands
        }
    }
    
    Serial.println("Script execution completed");
}

void executeCommand(String command) {
    if (bleKeyboard->isConnected()) {
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
            delay(50);
        }
    } else {
        Serial.println("Bluetooth not connected");
    }
}

void executeCommand() {
    String command = String(cmdBuffer);
    command.trim();
    
    addToHistory(command);
    currentHistoryPos = -1;

    if (command == "script") {
        readingScript = true;
        scriptBuffer = "";
        Serial.println("Paste your script below. Send 'END' on a new line to finish:");
        Serial.println("------------------------");
    }
    else if (readingScript) {
        if (command == "END") {
            readingScript = false;
            Serial.println("------------------------");
            executeScript(scriptBuffer);
        } else {
            scriptBuffer += command + "\n";
        }
    }
    else if (command.startsWith("rename ")) {
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
        Serial.println("  script   - Enter script mode (paste Ducky Script)");
        Serial.println("\nIn script mode, type 'END' on a new line to execute");
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
            delay(50);
        }
    }
    else {
        Serial.println("Bluetooth not connected");
    }
    
    cmdBuffer[0] = '\0';
    cmdIndex = 0;
    Serial.println();
    if (!readingScript) {
        printPrompt();
    }
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