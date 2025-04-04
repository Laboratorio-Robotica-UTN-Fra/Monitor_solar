#ifndef __TELEGRAM_CONTENT__
#define __TELEGRAM_CONTENT__

#include "monitor_functions.h"

#define TIME_DAY(time) String(time / (1000 * 60 * 60 * 24))
#define TIME_HS(time) String(time / (1000 * 3600))
#define TIME_MIN(time) String((time / (1000 * 60)) % 60)
#define TIME_SEG(time) String((time / 1000) % 60)
#define FORMAT_TIME(time) TIME_HS(time) + ":" + TIME_MIN(time) + ":" + TIME_SEG(time)


#define BOT_TOKEN "7507194258:AAEeaBbAkaprIwi9e3m0kBtYrEEBwsa88Zs"
#define ID_CHAT "7164870276"
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
            // for (int j = 0; j < numLeds; j++)
            // {
            //     led = leds[j];
            //     estado += "Luz " + String(j + 1) + ": " + (led.estado ? "Encendida" : "Apagada");
            //     if (led.t_prog > 0)
            //         estado += " (Apagado en " + FORMAT_TIME(led.t_prog - millis()) + ")";

            //     estado += "\n";
            // }
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
            String msg = "🔋 Voltaje: " + String(get_voltage_value(3300)) + "V\n";
            msg += "Voltaje IN: " + String(analogRead(A0) * ADC_BITS2VOLTS) + "V\n";
            bot.sendMessage(chat_id, msg, "");
            return;
        }
        if (text == "/current") {
            select_sensor(CURRENT);
            String msg = "⚡ Corriente: " + String(get_current_value()) + "A\n";
            msg += "Voltaje IN: " + String(analogRead(A0) * ADC_BITS2VOLTS) + "V\n";
            bot.sendMessage(chat_id, msg, "");
            return;
        }
        if (text == "/raw_adc") {
            select_sensor(VOLTAGE);
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

#endif