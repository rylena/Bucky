#include <BleKeyboard.h>

BleKeyboard bleKeyboard("ESP32 Ducky");

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE Keyboard...");
    bleKeyboard.begin();
}

void loop() {
    if (bleKeyboard.isConnected()) {
        if (Serial.available()) {
            String command = Serial.readStringUntil('\n');  // Read line from Serial
            command.trim();  // Remove extra spaces/newlines
            
            if (command.startsWith("STRING ")) {  // Typing text
                String text = command.substring(7);
                bleKeyboard.print(text);
            } 
            else if (command == "ENTER") {
                bleKeyboard.write(KEY_RETURN);
            }
            else if (command.startsWith("DELAY ")) {  // Pause execution
                int delayTime = command.substring(6).toInt();
                delay(delayTime);
            }
            else if (command == "CTRL ALT DELETE") {  // Special key combo
                bleKeyboard.press(KEY_LEFT_CTRL);
                bleKeyboard.press(KEY_LEFT_ALT);
                bleKeyboard.press(KEY_DELETE);
                delay(100);
                bleKeyboard.releaseAll();
            }
            else if (command.startsWith("CTRL ")) {  // CTRL + <Key>
                char key = command[5];
                bleKeyboard.press(KEY_LEFT_CTRL);
                bleKeyboard.press(key);
                delay(100);
                bleKeyboard.releaseAll();
            }
            else if (command.startsWith("ALT ")) {  // ALT + <Key>
                char key = command[4];
                bleKeyboard.press(KEY_LEFT_ALT);
                bleKeyboard.press(key);
                delay(100);
                bleKeyboard.releaseAll();
            }
            else if (command.startsWith("SHIFT ")) {  // SHIFT + <Key>
                char key = command[6];
                bleKeyboard.press(KEY_LEFT_SHIFT);
                bleKeyboard.press(key);
                delay(100);
                bleKeyboard.releaseAll();
            }

            Serial.println("Executed: " + command);
        }
    }
}
