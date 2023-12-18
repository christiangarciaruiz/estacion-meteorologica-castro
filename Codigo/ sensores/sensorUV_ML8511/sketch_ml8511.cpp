#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Credenciales para conectarse a la red del centro
const char* ssid     = "CENTRO";
const char* password = "";

// Nombre del dominio y archivo a donde subir los datos
const char* serverName = "https://ultravioletasuperestacion.000webhostapp.com/post-esp-data.php";

// El archivo "post-esp-data.php" en nuestra base de datos necesita tener la misma clave de API para funcionar
String apiKeyValue = "tPmAT5Ab3j7F9";
String sensorName = "ML8511";
String sensorLocation = "Home";

int UVOUT = 32; // Salida de datos del sensor
int REF_3V3 = 4; // Fuente de 3.3V del sensor

void setup()
{
  // Iniciamos el serial
  Serial.begin(115200);
  
  // Iniciamos los pines del sensor
  pinMode(UVOUT, INPUT);
  pinMode(REF_3V3, INPUT);

  // Iniciamos todo lo relacionado con el WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED)
  { 
    delay(500);
    Serial.print(".");
  }

  Serial.println("Conectado al WiFi");

  // Pasamos al serial la direccion IP y MAC en caso de necesitarlas
  Serial.println("Dirección IP: ");
  Serial.print(WiFi.localIP());

  Serial.println("Dirección MAC: ");
  Serial.print(WiFi.macAddress());
}

// Devuelve la media de la salida del sensor
int averageAnalogRead(int pinToRead)
{
  byte numberOfReadings = 8;
  unsigned int runningValue = 0; 
 
  for(int x = 0 ; x < numberOfReadings ; x++)
  {
    runningValue += analogRead(pinToRead);
  }
  
  runningValue /= numberOfReadings;
 
  return(runningValue);
}

// Limita el valor de la variable que le introduzcamos
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Devuelve el valor de indice UV, es decir, el valor final que vamos a usar
int indicereturn()
{
  int uvLevel = averageAnalogRead(UVOUT);
  int refLevel = averageAnalogRead(REF_3V3);
  
  // Usamos el pin de 3.3V del sensor como referencia para conseguir datos precisos
  float outputVoltage = 3.3 / refLevel * uvLevel;
  
  float uvIntensity = mapfloat(outputVoltage, 0.82, 2.8, 0.0, 15.0); // Convertimos el voltaje en intensidad UV
  // uvLevel = salida del ML8511
  // outputVoltage = voltaje del ML8511

  int indice;
  int longonda = map(uvLevel, 0, 4095, 0, 1170);

  if (longonda < 50) { indice = 0; }
  else if (longonda < 227) { indice = 1; }
  else if (longonda < 318) { indice = 2; }
  else if (longonda < 408) { indice = 3; }
  else if (longonda < 503) { indice = 4; }
  else if (longonda < 606) { indice = 5; }
  else if (longonda < 696) { indice = 6; }
  else if (longonda < 795) { indice = 7; }
  else if (longonda < 881) { indice = 8; }
  else if (longonda < 976) { indice = 9; }
  else if (longonda < 1079) { indice = 10; }
  else { indice = 11; }

  return indice;
}

int oneminute = 60000;
void loop()
{
  // Solo ejecutaremos el código si estamos conectados al WiFi
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    
    // Iniciamos con la url antes indicada
    http.begin(serverName);
    
    // Specify content-type header
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    // Creamos el POST que vamos a mandar mas adelante
    String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName
                          + "&location=" + sensorLocation + "&value1=" + String(indicereturn());

    // La pasamos al serial para poder comprobar que es correcta
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);

    // Mandamos el POST que hemos creado
    int httpResponseCode = http.POST(httpRequestData);

    // Pasamos el código HTTP de respuesta al serial
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    
    http.end();

    // Mandamos datos cada 10 minutos
    delay(oneminute * 10); 
  }
  else
    Serial.println("WiFi no conectado");
}