#ifndef __MONITOR_FUNCTIONS__
#define __MONITOR_FUNCTIONS__

#include "Arduino.h"

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

enum sensor_type{
    VOLTAGE,
    CURRENT
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
    pinMode(GPIO_RELAY_1, HIGH);
    pinMode(GPIO_RELAY_2, HIGH);
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
#endif