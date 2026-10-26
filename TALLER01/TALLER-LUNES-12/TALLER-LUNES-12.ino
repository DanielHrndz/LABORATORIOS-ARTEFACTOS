#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ======================================================
// CONFIGURACIÓN MQTT
// ======================================================

const char* WIFI_SSID = "ARTEFACTOS";
const char* WIFI_PASSWORD = "s^3sQ#9teskVgnb";

// ======================================================
// CONFIGURACIÓN MQTT
// ======================================================

const char* MQTT_SERVER = "10.40.9.81";
const int MQTT_PORT = 1883;

// Topics utilizados por el dashboard React.
const char* TOPIC_TEMPERATURA = "clima/esp32/temperatura";
const char* TOPIC_HUMEDAD = "clima/esp32/humedad";
const char* TOPIC_LUZ = "clima/esp32/luz";


// ======================================================
// CONFIGURACIÓN DE SENSORES
// ======================================================

// (Estudiante)
// Definir los pines y objetos necesarios para los sensores.
#define DHTPIN 35
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define LDRPIN 25

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Enviar una lectura cada 2 segundos.
const unsigned long INTERVALO_ENVIO = 2000;

unsigned long ultimoEnvio = 0;

void conectarWiFi() {

  Serial.print("Conectando a WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado");

  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.localIP());
}

void conectarMQTT() {

  while (!mqttClient.connected()) {

    Serial.print("Conectando a Mosquitto...");

    // Crear un Client ID diferente para este ESP32.
    String clientId = "ESP32-";
    clientId += String(
      (uint32_t)(ESP.getEfuseMac() & 0xFFFFFFFF),
      HEX
    );

    if (mqttClient.connect(clientId.c_str())) {

      Serial.println(" conectado");

    } else {

      Serial.print(" error, rc=");
      Serial.print(mqttClient.state());
      Serial.println(". Reintentando en 2 segundos...");

      delay(2000);
    }
  }
}


// ======================================================
// LEER Y PUBLICAR SENSORES
// ======================================================

void publicarSensores() {

  /*
    (Estudiante)
      1. Obtener la temperatura.

      2. Obtener la humedad.

      3. Obtener la lectura del sensor de iluminación.

      4. Preparar los valores para poder publicarlos
         mediante MQTT.

      5. Publicar cada dato en su topic correspondiente:

         TOPIC_TEMPERATURA
         TOPIC_HUMEDAD
         TOPIC_LUZ

      6. Mostrar las mediciones en el Monitor Serial.
  */
  
  // 1. Obtener la temperatura
  float temperatura = dht.readTemperature();
  
  // 2. Obtener la humedad
  float humedad = dht.readHumidity();
  
  // 3. Obtener la lectura del sensor de iluminación
  int ldr_adc = analogRead(LDRPIN);
  
  // Convertir lectura ADC a porcentaje de luz (0-4095 -> 0-100%)
  int luz_porcentaje = map(ldr_adc, 0, 4095, 0, 100);

  if (isnan(temperatura) || isnan(humedad)) {
    return;
  }

  // 4. Preparar los valores para publicarlos
  String str_temp = String(temperatura, 2);
  String str_hum = String(humedad, 2);
  String str_luz = String(luz_porcentaje);

  // 5. Publicar cada dato
  mqttClient.publish(TOPIC_TEMPERATURA, str_temp.c_str());
  mqttClient.publish(TOPIC_HUMEDAD, str_hum.c_str());
  mqttClient.publish(TOPIC_LUZ, str_luz.c_str());

  // 6. Mostrar mediciones
  Serial.println("");
  Serial.print("Temperatura: ");
  Serial.print(str_temp);
  Serial.println(" °C");
  
  Serial.print("Humedad: ");
  Serial.print(str_hum);
  Serial.println(" %");
  
  Serial.print("LDR ADC: ");
  Serial.print(ldr_adc);
  Serial.print(" | Luz: ");
  Serial.print(str_luz);
  Serial.println(" %");
  Serial.println("------------------------------");
}

void setup() {

  Serial.begin(115200);

  // (Estudiante)
  // Inicializar los sensores y configurar los pines
  // necesarios.
  dht.begin();
  analogReadResolution(12);

  conectarWiFi();

  mqttClient.setServer(
    MQTT_SERVER,
    MQTT_PORT
  );
}

void loop() {

  // Si se pierde WiFi, volver a conectarse.
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  // Si se pierde MQTT, volver a conectarse.
  if (!mqttClient.connected()) {
    conectarMQTT();
  }

  mqttClient.loop();


  // Publicar sensores cada 2 segundos.
  if (millis() - ultimoEnvio >= INTERVALO_ENVIO) {

    ultimoEnvio = millis();

    publicarSensores();
  }
}