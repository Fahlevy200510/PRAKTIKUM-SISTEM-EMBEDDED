#include <Arduino.h>

#define NTC_PIN PA0
#define SERIES_R 10000
#define NTC_R25  10000
#define BETA     3950

float readTemp() {
    int adc = analogRead(NTC_PIN);
    float v = adc * 3.3 / 4095.0;
    float r = SERIES_R * v / (3.3 - v);
    float t = r / NTC_R25;
    t = log(t) / BETA + 1.0 / 298.15;
    return 1.0 / t - 273.15;
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Program 11: NTC Temp - STM32\n");
}

void loop() {
    Serial.printf("Temp: %.1f C\n", readTemp());
    delay(1000);
}
