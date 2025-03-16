#include <BleKeyboard.h>

BleKeyboard bleKeyboard("ESP32 Ducky");

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

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE Keyboard...");
    bleKeyboard.begin();
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

void executeCommand() {
    String command = String(cmdBuffer);
    command.trim();
    
    // Add to history
    addToHistory(command);
    currentHistoryPos = -1;
    
    // Execute existing command logic
    if (bleKeyboard.isConnected()) {
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
    } else {
        Serial.println("\nBLE Keyboard not connected.");
    }
    
    // Reset command buffer
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