#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "monitor_functions.h"
#include "telegram_content.h"

#define WIFI_SSID "Telecentro-996b"
#define WIFI_PASSWORD "ZNYUW3MDZDTM"

#define GPIO_LED    2


void conectarWiFi();

unsigned long tiempoInicio;
unsigned long ultimaVezMensaje = 0;
unsigned long ultimaVezParpadeo = 0;

unsigned long intervaloMensajes = 500;
unsigned long intervaloParpadeo = 250;
int n_new_messages;

void setup()
{
    Serial.begin(115200);
    delay(2000);
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

void conectarWiFi()
{
    Serial.println("Conectando a WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 20)
    {
        delay(500);
        Serial.print(".");
        intentos++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWiFi conectado");
        Serial.print("Dirección IP: ");
        Serial.println(WiFi.localIP());
        secured_client.setInsecure();
        delay(1000);
        bot.sendMessage(ID_CHAT, "✅ Conectado a la red WiFi. Usa /ayuda para las funciones.", "");
    }
    else
    {
        Serial.println("\nError al conectar a WiFi.");
        bot.sendMessage(ID_CHAT, "❌ Error al conectar a la red WiFi.", "");
    }
}

