#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "monitor_functions.h"
#include "telegram_content.h"


#define GPIO_LED    2
#define GPIO_LED_ON 0
#define SPREADSHEET_ON 0

unsigned long tiempoInicio;
unsigned long ultimaVezMensaje = 0;
unsigned long ultimaVezGsheet = 0;

unsigned long intervaloMensajes = 500;
unsigned long intervaloGsheet = 10000;
int n_new_messages;

void setup()
{
    Serial.begin(115200);
    tiempoInicio = millis();
    Serial.println("Start Device");
    #if GPIO_LED_ON
    pinMode(GPIO_LED, OUTPUT);
    #endif
    solar_gpio_init();
    Serial.println("Finish GPIO");
    conectarWiFi();
    
    
    select_sensor(VOLTAGE);
    float v_adc_value = get_voltage_value(3300);
    Serial.print("El valor de tension actualmente es: ");
    Serial.println(v_adc_value);
    Serial.print("Voltaje de entrada: ");
    Serial.println(analogRead(A0) * ADC_BITS2VOLTS);
    delay(500);

    select_sensor(CURRENT);
    float i_adc_value = get_current_value();
    Serial.print("El valor de corriente actualmente es: ");
    Serial.println(i_adc_value);
    Serial.print("Voltaje de entrada: ");
    Serial.println(analogRead(A0) * ADC_BITS2VOLTS);

    ultimaVezGsheet = millis();
}


void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        // Parpadeo cuando hay conexión
        #if SPREADSHEET_ON
        if (millis() - ultimaVezGsheet > intervaloGsheet) {
            Serial.println("Guardar data en gsheet");
            PanelData panel_data = get_panel_data(ultimaVezGsheet);
            sendDataToGoogleSheets(panel_data);
            ultimaVezGsheet = millis();

        }
        #endif
        if (millis() - ultimaVezMensaje > intervaloMensajes)
        {
            #if GPIO_LED_ON
            digitalWrite(GPIO_LED, HIGH);
            #endif
            n_new_messages = bot.getUpdates(bot.last_message_received + 1);
            while(n_new_messages > 0) {
                manejarMensajesNuevos(n_new_messages);
                n_new_messages = bot.getUpdates(bot.last_message_received + 1);
            }
            ultimaVezMensaje = millis();
        }
    }
    else
    {
        Serial.println("Error desconexion de wifi");
        // Si se pierde la conexión, reconectar y mantener el LED fijo
        #if GPIO_LED_ON
        digitalWrite(GPIO_LED, LOW);
        #endif
        conectarWiFi();
    }

}



