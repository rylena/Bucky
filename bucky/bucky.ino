#include <BleKeyboard.h>

BleKeyboard bleKeyboard("ESP32 Ducky");

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE Keyboard...");
    bleKeyboard.begin();
    Serial.println("Enter commands (e.g., STRING Hello, CTRL S, ALT F4, DELAY 1000):");
}

void loop() {
    if (bleKeyboard.isConnected()) {
        if (Serial.available()) {
            String command = Serial.readStringUntil('\n');  // Read line from Serial
            command.trim();  // Remove extra spaces/newlines
            
            Serial.println("Received: " + command);  // Debugging message

            if (command.startsWith("STRING ")) {  // Typing text
                String text = command.substring(7);
                Serial.println("Typing: " + text);  // Debugging message
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
    } else {
        Serial.println("BLE Keyboard not connected.");
    }
}
