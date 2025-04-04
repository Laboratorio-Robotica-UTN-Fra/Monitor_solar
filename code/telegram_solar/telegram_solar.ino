#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "monitor_functions.h"
#include "telegram_content.h"


#define GPIO_LED    2

unsigned long tiempoInicio;
unsigned long ultimaVezMensaje = 0;
unsigned long ultimaVezParpadeo = 0;

unsigned long intervaloMensajes = 2000;
unsigned long intervaloParpadeo = 10000;
int n_new_messages;

void setup()
{
    Serial.begin(115200);
    delay(3000);
    tiempoInicio = millis();
    Serial.println("Start Device");
    pinMode(GPIO_LED, OUTPUT);
    solar_gpio_init();
    Serial.println("Finish GPIO");
    conectarWiFi();
    
    select_sensor(VOLTAGE);
    get_voltage_value(3300);
}


void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        
        // Parpadeo cuando hay conexión
        if (millis() - ultimaVezParpadeo > intervaloParpadeo)
        {
            digitalWrite(GPIO_LED, !digitalRead(GPIO_LED));
            PanelData panel_data = get_panel_data(ultimaVezParpadeo);
            sendDataToGoogleSheets(panel_data);
            ultimaVezParpadeo = millis();
        }
        if (millis() - ultimaVezMensaje > intervaloMensajes)
        {
            digitalWrite(GPIO_LED, HIGH);
            n_new_messages = bot.getUpdates(bot.last_message_received + 1);
            manejarMensajesNuevos(n_new_messages);
            ultimaVezMensaje = millis();
        }
    }
    else
    {
        Serial.println("Error desconexion de wifi");
        // Si se pierde la conexión, reconectar y mantener el LED fijo
        digitalWrite(GPIO_LED, LOW);
        conectarWiFi();
    }

}
