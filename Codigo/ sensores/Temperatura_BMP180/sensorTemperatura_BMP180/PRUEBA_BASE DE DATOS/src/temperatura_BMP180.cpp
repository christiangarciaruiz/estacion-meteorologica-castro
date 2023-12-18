#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>

// Prototipo de la función
void EnvioDatos(float presion);

Adafruit_BMP085 bmp;

const char* ssid = "OPPO A91";
const char* password = "1234567g";

void setup() {
  Serial.begin(9600);
  Serial.println("Conexión con el servidor - BMP180");

  if (!bmp.begin()) {
    Serial.println("Error al iniciar el BMP180");
    while (1); // Bucle infinito
  } else {
    Serial.println("BMP180 iniciado correctamente");
  }

  WiFi.begin(ssid, password);
  Serial.print("Conectando a la red WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConexión exitosa!");
  Serial.print("IP Local: ");
  Serial.println(WiFi.localIP());
}

// Definición de la función
void EnvioDatos(float presion) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String data_to_send = "presion=" + String(presion);

    http.begin("http://esp32-bmp180.000webhostapp.com/EspPost.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int http_code = http.POST(data_to_send);

    if (http_code > 0) {
      Serial.print("Código HTTP: ");
      Serial.println(http_code);

      if (http_code == 200) {
        String response = http.getString();
        Serial.println("El servidor respondió:");
        Serial.println(response);
      }
    } else {
      Serial.print("Error al enviar POST, código: ");
      Serial.println(http_code);
    }

    http.end();
  } else {
    Serial.println("Error en la conexión WiFi");
  }
}

void loop() {
  float pressure = bmp.readPressure();

  if (isnan(pressure)) {
    Serial.println("Error al leer la presión del BMP180");
    return;
  }

  Serial.print("Presión: ");
  Serial.print(pressure);
  Serial.println(" mb");

  EnvioDatos(pressure);
  delay(60000); // Espera 60 segundos
}
