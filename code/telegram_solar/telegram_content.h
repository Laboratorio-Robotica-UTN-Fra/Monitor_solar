#ifndef __TELEGRAM_CONTENT__
#define __TELEGRAM_CONTENT__

#include "monitor_functions.h"
#include "credentials.h"
#include <ArduinoJson.h>

#define TIME_DAY(time) String(time / (1000 * 60 * 60 * 24))
#define TIME_HS(time) String(time / (1000 * 3600))
#define TIME_MIN(time) String((time / (1000 * 60)) % 60)
#define TIME_SEG(time) String((time / 1000) % 60)
#define FORMAT_TIME(time) TIME_HS(time) + ":" + TIME_MIN(time) + ":" + TIME_SEG(time)

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

extern unsigned long tiempoInicio;

const String menu_content =
    "💡 Bienvenido al sistema de monitor solar.\n"
    "Estas son tus opciones:\n\n"
    "/estado: Muestra el estado actual de las luces y tiempos programados\n"
    "/live: Muestra cuánto tiempo lleva encendido el dispositivo\n"
    "/uso: Muestra cuántas veces se encendieron y apagaron las luces\n"
    "/relay1_on: Enciende la luz 1\n"
    "/relay1_off: Apaga la luz 1\n"
    "/relay2_on: Enciende la luz 2\n"
    "/relay2_off: Apaga la luz 2\n"
    "/voltage: Muestra el voltaje actual de la batería\n"
    "/current: Muestra la corriente actual de la batería\n"
    "/raw_adc: Muestra el valor del ADC\n"
    "/chat_id: Muestra el ID del chat\n"
    
    "Recuerda que los comandos deben comenzar con '/'"
;

void manejarMensajesNuevos(int cantidadMensajes)
{
    for (int i = 0; i < cantidadMensajes; i++)
    {
        String chat_id = bot.messages[i].chat_id;
        String text = bot.messages[i].text;
        text.toLowerCase();

        if (text == "/estado")
        {
            String estado = "🔍 Estado del sistema:\n";
            
            select_sensor(VOLTAGE);
            estado += "🔋 Voltaje: " + String(get_voltage_value(3300)) + "V\n";
            select_sensor(CURRENT);
            estado += "⚡ Corriente: " + String(get_current_value()) + "A\n";
            bot.sendMessage(chat_id, estado, "");
            return;
        }

        if (text == "/ayuda")
        {
            bot.sendMessage(chat_id, menu_content, "");
            return;
        }

        if (text == "/live")
        {
            unsigned long t_diff = millis() - tiempoInicio;
            String tiempo = TIME_DAY(t_diff) + " días, " + FORMAT_TIME(t_diff);
            bot.sendMessage(chat_id, "⏳ Tiempo encendido: " + tiempo, "");

            return;
        }

        if (text == "/relay1_on") {
            digitalWrite(GPIO_RELAY_1, LOW);
            bot.sendMessage(chat_id, "🔆 Luz 1 encendida", "");
            return;
        }
        if (text == "/relay1_off") {
            digitalWrite(GPIO_RELAY_1, HIGH);
            bot.sendMessage(chat_id, "🌑 Luz 1 apagada", "");
            return;
        }
        if (text == "/relay2_on") {
            digitalWrite(GPIO_RELAY_2, LOW);
            bot.sendMessage(chat_id, "🔆 Luz 2 encendida", "");
            return;
        }
        if (text == "/relay2_off") {
            digitalWrite(GPIO_RELAY_2, HIGH);
            bot.sendMessage(chat_id, "🌑 Luz 2 apagada", "");
            return;
        }

        if (text == "/voltage") {
            select_sensor(VOLTAGE);
            int value = analogRead(A0);
            String msg = "🔋 Voltaje: " + String(get_voltage_value(3300)) + "V\n";
            msg += "Valor ADC: " + String(value) + "\nVoltaje IN: " + String(value * ADC_BITS2VOLTS) + "V\n";
            bot.sendMessage(chat_id, msg, "");
            return;
        }
        if (text == "/current") {
            select_sensor(CURRENT);
            int value = analogRead(A0);
            String msg = "⚡ Corriente: " + String(get_current_value()) + "A\n";
            msg += "Valor ADC: " + String(value) + "\nVoltaje IN: " + String(value * ADC_BITS2VOLTS) + "V\n";
            bot.sendMessage(chat_id, msg, "");
            return;
        }
        if (text == "/raw_adc") {
            int value = analogRead(A0);
            bot.sendMessage(chat_id, "🔋 Valor ADC: " + String(value) + "\nVoltage IN: " + String(value * ADC_BITS2VOLTS), "");
            return;
        }
        if (text == "/chat_id") {
            bot.sendMessage(chat_id, "💬 ID del chat: " + chat_id, "");
            return;
        }

        bot.sendMessage(chat_id, "⚠️ Comando incorrecto. Usa /ayuda para ver los comandos disponibles.", "");
    }
}

void sendDataToGoogleSheets(PanelData data) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected!");
        return;
    }

    // secured_client.setInsecure(); // Ignore SSL certificate check

    if (!secured_client.connect("script.google.com", 443)) {
        Serial.println("Connection to Google failed");
        return;
    }

    StaticJsonDocument<256> jsonDoc;
    jsonDoc["voltage"] = data.voltage;
    jsonDoc["current"] = data.current;
    jsonDoc["power"] = data.power;
    jsonDoc["energy"] = data.energy;
    jsonDoc["date_time"] = data.date_time;

    String jsonString;
    serializeJson(jsonDoc, jsonString);

    String postRequest = String("POST ") + googleScriptUrl + " HTTP/1.1\r\n" +
                         "Host: script.google.com\r\n" +
                         "Content-Type: application/json\r\n" +
                         "Content-Length: " + jsonString.length() + "\r\n\r\n" +
                         jsonString;

    secured_client.print(postRequest);

    delay(200);

    while (secured_client.available()) {
        String line = secured_client.readStringUntil('\r');
        Serial.print(line);
    }

    secured_client.stop();
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
        // delay(1000);
        bot.sendMessage(ID_CHAT, "✅ Conectado a la red WiFi. Usa /ayuda para las funciones.", "");
    }
    else
    {
        Serial.println("\nError al conectar a WiFi.");
        bot.sendMessage(ID_CHAT, "❌ Error al conectar a la red WiFi.", "");
    }
}

#endif