#ifndef __MONITOR_FUNCTIONS__
#define __MONITOR_FUNCTIONS__

#include "Arduino.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

#define ESP8266_BOARD 1
#define ESP12_F_BOARD 2

#define BOARD_USE ESP8266_BOARD

#if BOARD_USE == ESP8266_BOARD

#define GPIO_MUX_A  13
#define GPIO_MUX_B  14
#define GPIO_MUX_C  12

#define GPIO_RELAY_1  5
#define GPIO_RELAY_2  4

#define GPIO_POT_CS   16
#define GPIO_POT_STEP 5
#define GPIO_POT_DIR  4

#endif

#if BOARD_USE == ESP12_F_BOARD

#define GPIO_MUX_A  13
#define GPIO_MUX_B  14
#define GPIO_MUX_C  12

#define GPIO_RELAY_1  8
#define GPIO_RELAY_2  6

#define GPIO_POT_CS   10
#define GPIO_POT_STEP 9
#define GPIO_POT_DIR  7

#endif

#define ADC_RESOLUTION 1024
#define ADC_VREF 3.2
#define ADC_BITS2VOLTS ADC_VREF / ADC_RESOLUTION
// 2.2 * (1 + 3.3 / 1.5) / 102.2 = 0.068845 => ^-1 = 14.517
#define ADC_V_GAIN 14.517
// Resistencias = ((33k + 20k) / 33k)
#define ADC_A_GAIN 1.606
// Inversa de la pendiente sensor ACS712
// 30A / 2V = 15A/V
#define ACS712_SENSITIVITY_INVERSE 15


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -3 * 3600, 60000);

enum sensor_type{
    VOLTAGE,
    CURRENT
};

struct PanelData {
    float voltage;
    float current;
    float power;
    float energy;
    String date_time;
};

void solar_gpio_init(void) {
    pinMode(GPIO_MUX_A, OUTPUT);
    pinMode(GPIO_MUX_B, OUTPUT);
    pinMode(GPIO_MUX_C, OUTPUT);
    // pinMode(GPIO_POT_CS, OUTPUT);
    // pinMode(GPIO_POT_STEP, OUTPUT);
    // pinMode(GPIO_POT_DIR, OUTPUT);
    
    pinMode(GPIO_RELAY_1, OUTPUT);
    pinMode(GPIO_RELAY_2, OUTPUT);
    digitalWrite(GPIO_RELAY_1, LOW);
    digitalWrite(GPIO_RELAY_2, LOW);
}

float get_voltage_value(int r1_value) {
    float gain = 1022 / (22 * (1 + r1_value / 1500));
    return gain * analogRead(A0) * ADC_BITS2VOLTS;
}

float get_current_value() {
    int acs712_voltage = ADC_A_GAIN * analogRead(A0) * ADC_BITS2VOLTS;
    return acs712_voltage / ACS712_SENSITIVITY_INVERSE;
}


void select_sensor(enum sensor_type tipo) {
    switch (tipo) {
        case VOLTAGE:
            digitalWrite(GPIO_MUX_A, HIGH);
            digitalWrite(GPIO_MUX_B, LOW);
            digitalWrite(GPIO_MUX_C, LOW);
            break;
        case CURRENT:
            digitalWrite(GPIO_MUX_A, LOW);
            digitalWrite(GPIO_MUX_B, LOW);
            digitalWrite(GPIO_MUX_C, LOW);
            break;
    }
    delay(10);
}

void print_panel_data(PanelData panel_data) {
    Serial.println("Voltage: " + String(panel_data.voltage) + "V");
    Serial.println("Current: " + String(panel_data.current) + "A");
    Serial.println("Power: " + String(panel_data.power) + "W");
    Serial.println("Energy: " + String(panel_data.energy) + "kWh");
    Serial.println("Date/Time: " + panel_data.date_time);
}

PanelData get_panel_data(unsigned long last_reading_time) {
    PanelData panel_data;
    panel_data.voltage = get_voltage_value(3300);
    panel_data.current = get_current_value();
    panel_data.power = panel_data.voltage * panel_data.current;
    panel_data.energy += panel_data.power * (millis() - last_reading_time) / 3600000.0; // kWh
    timeClient.update();
    panel_data.date_time = timeClient.getFormattedTime();
    
    print_panel_data(panel_data);

    return panel_data;
}

#endif