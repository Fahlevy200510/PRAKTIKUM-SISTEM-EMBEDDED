#include <Arduino.h>

#define NTC_PIN     34
#define SERIES_R    10000   // 10k series resistor
#define NTC_R25     10000   // NTC resistance at 25C
#define BETA        3950    // NTC beta coefficient

float readTemperature() {
    int adc = analogRead(NTC_PIN);
    
    // Calculate NTC resistance
    float voltage = adc * 3.3 / 4095.0;
    float ntc_r = SERIES_R * voltage / (3.3 - voltage);
    
    // Steinhart-Hart equation (simplified)
    float steinhart = ntc_r / NTC_R25;
    steinhart = log(steinhart);
    steinhart /= BETA;
    steinhart += 1.0 / (25.0 + 273.15);
    float tempK = 1.0 / steinhart;
    float tempC = tempK - 273.15;
    
    return tempC;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Program 11: NTC Temperature - ESP32\n");
    analogReadResolution(12);
}

void loop() {
    float temp = readTemperature();
    Serial.printf("Temperature: %.1f C\n", temp);
    delay(1000);
}
