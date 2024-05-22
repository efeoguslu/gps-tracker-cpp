#ifndef LED_H
#define LED_H

#include <chrono>
#include <iostream>

const int gpsLedPin{ 12 };
const int gpsLedDelayDurationMs{ 500 };

class Led {
public:
    Led(int pin);
    void update(bool isConnected);   

private:
    void turnOn();
    void turnOff();
    void toggle();

    int pinNumber;
    bool isBlinking;
    std::chrono::steady_clock::time_point lastUpdateTime;

    void checkAndToggle(); // Method to check elapsed time and toggle LED if needed
};


#endif // LED_H