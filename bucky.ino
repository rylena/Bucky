#include <BleKeyboard.h>

BleKeyboard bleKeyboard("ESP32 Auto Keystroker");

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE Keyboard...");
    bleKeyboard.begin();
}

void loop() {
    if (bleKeyboard.isConnected()) {
        Serial.println("Sending: helloworld");
        bleKeyboard.print(String("helloworld").c_str());  // Convert to const char*
        delay(5000);  // Wait 5 seconds before sending again
    }
}
