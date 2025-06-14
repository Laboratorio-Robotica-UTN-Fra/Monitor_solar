#ifndef __MONITOR_FUNCTIONS__
#define __MONITOR_FUNCTIONS__

#include "Arduino.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

#define GPIO_MUX_A  13
#define GPIO_MUX_B  14
#define GPIO_MUX_C  12

#define GPIO_RELAY_1  10
#define GPIO_RELAY_2  2

#define ADC_RESOLUTION 1024
#define ADC_VREF 1.0f
#define ADC_BITS2VOLTS ((float) (ADC_VREF / ADC_RESOLUTION))


// 2.2 * (1 + 3.3 / 1.5) / 102.2 = 0.068845 => ^-1 = 14.517
#define ADC_V_GAIN 100680.0 / 680
#define PANEL_V_OFFSET 0.6f
// Resistencias = ((33k + 20k) / 33k)
// Inversa de la pendiente sensor ACS712
// 52k // 100k => 0.342 ~ 0.35
// 0A -> 2.52v -> 0.88v     | 0 v
// 30A -> 0.55v -> 0.19v    | 0.69 v
#define ACS712_OFFSET_COUNTS 934
#define ACS712_SENSITIVITY 42.47f 


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -3 * 3600, 60000);

enum sensor_type{
    VOLTAGE,
    CURRENT
};

void select_sensor(enum sensor_type tipo);
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
    select_sensor(VOLTAGE);
    
    pinMode(GPIO_RELAY_1, OUTPUT);
    pinMode(GPIO_RELAY_2, OUTPUT);
    digitalWrite(GPIO_RELAY_1, HIGH);
    digitalWrite(GPIO_RELAY_2, HIGH);
}

float get_voltage_value(int r1_value) {
    float gain =  ADC_V_GAIN / (1 + r1_value / 1500.0);
    return gain * analogRead(A0) * ADC_BITS2VOLTS - PANEL_V_OFFSET;
}

float get_current_value() {
    float adc_i_volts =  ACS712_SENSITIVITY * (ACS712_OFFSET_COUNTS - analogRead(A0)) * ADC_BITS2VOLTS;
    return adc_i_volts; // - 0.878
}


void select_sensor(enum sensor_type tipo) {
    switch (tipo) {
        case VOLTAGE:
            digitalWrite(GPIO_MUX_A, LOW);
            digitalWrite(GPIO_MUX_B, HIGH);
            digitalWrite(GPIO_MUX_C, LOW);
            break;
        case CURRENT:
            digitalWrite(GPIO_MUX_A, HIGH);
            digitalWrite(GPIO_MUX_B, HIGH);
            digitalWrite(GPIO_MUX_C, LOW);
            break;
    }
    delay(50);
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