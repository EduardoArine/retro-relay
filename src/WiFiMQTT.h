#pragma once
#include <WiFi.h>

const char *ssid = "Arppen Mobile";
const char *password = "Arppen!@#4";

void conectarWiFi()
{
    WiFi.begin(ssid, password);
    Serial.println("Conectando ao Wi-Fi...");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConectado: IP = %s\n", WiFi.localIP().toString().c_str());
}
