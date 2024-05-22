#include "led.h"
#include "wiringPi.h"

Led::Led(int pin) : pinNumber(pin), isBlinking(false), lastUpdateTime(std::chrono::steady_clock::now()) {
    pinMode(pinNumber, OUTPUT);

    // std::cout << "Starting LED object.\n";
    turnOff(); // Ensure LED starts off
}

void Led::turnOn(){
    digitalWrite(pinNumber, HIGH);
    // std::cout << "LED is ON.\n";
}

void Led::turnOff(){
    digitalWrite(pinNumber, LOW);
    // std::cout << "LED is OFF.\n";
}

void Led::toggle(){
    digitalWrite(pinNumber, !digitalRead(pinNumber));
    // std::cout << "LED is toggling.\n";
}

void Led::update(bool isConnected) {
    if (isConnected) {
        turnOn();
        isBlinking = false;
    } else {
        isBlinking = true;
    }
    checkAndToggle();
}

void Led::checkAndToggle() {
    if (!isBlinking) {
        return; // Do nothing if not in blinking mode
    }

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdateTime).count();
    if (elapsed >= gpsLedDelayDurationMs) {
        toggle();
        lastUpdateTime = now;
    }
}