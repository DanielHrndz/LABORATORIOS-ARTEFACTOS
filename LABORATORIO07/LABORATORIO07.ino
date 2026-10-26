#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// ======================================================
// IDENTIFICACIÓN
// ======================================================

const int MI_ID = 2;   // Receptor de la pareja 1

// ======================================================
// OLED
// ======================================================

#define I2C_ADDRESS   0x3C
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ======================================================
// PINES LORA
// ======================================================

#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23
#define LORA_NSS   5

// ======================================================
// PROCESAR MENSAJE
// ======================================================

void procesarMensaje(String paquete) {

  // Formato: ORIGEN|DESTINO|MENSAJE
  int separador1 = paquete.indexOf('|');
  int separador2 = paquete.indexOf('|', separador1 + 1);

  if (separador1 == -1 || separador2 == -1) {
    Serial.println("Formato incorrecto recibido.");
    return;
  }

  int idOrigen = paquete.substring(0, separador1).toInt();
  int idDestino = paquete.substring(separador1 + 1, separador2).toInt();
  String mensaje = paquete.substring(separador2 + 1);

  // ----------------------------------------------------
  // COMPROBAR DESTINO
  // ----------------------------------------------------

  if (idDestino != MI_ID) {
    Serial.println("Mensaje ignorado (no es para mi ID).");
    return;
  }

  int rssi = LoRa.packetRssi();

  // ----------------------------------------------------
  // MONITOR SERIAL (Muestra lo mismo que la pantalla)
  // ----------------------------------------------------
  Serial.println("\n--- NUEVO MENSAJE ---");
  Serial.print("Mensaje de ID: ");
  Serial.println(idOrigen);
  Serial.println("Mensaje cifrado:");
  Serial.println(mensaje);
  Serial.print("RSSI: ");
  Serial.print(rssi);
  Serial.println(" dBm");
  Serial.println("---------------------");

  // ----------------------------------------------------
  // OLED (Muestra lo mismo que el Monitor Serial)
  // ----------------------------------------------------

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("Mensaje de ID: ");
  display.println(idOrigen);

  display.setCursor(0, 14);
  display.println("Mensaje cifrado:");

  display.setCursor(0, 28);
  display.println(mensaje);

  display.setCursor(0, 54);
  display.print("RSSI: ");
  display.print(rssi);
  display.println(" dBm");

  display.display();
}

// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(115200);   // El Monitor Serial debe estar en 115200
  delay(1000);

  Serial.println("RECEPTOR LORA INICIANDO...");

  Serial.print("Mi ID: ");
  Serial.println(MI_ID);

  // ----------------------------------------------------
  // OLED (mensaje de inicio)
  // ----------------------------------------------------

  Wire.begin();
  delay(250);

  if (!display.begin(I2C_ADDRESS, true)) {
    Serial.println("ERROR: OLED no encontrada");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("RECEPTOR LORA");

  display.setCursor(0, 18);
  display.print("ID: ");
  display.println(MI_ID);

  display.setCursor(0, 36);
  display.println("Esperando mensaje...");

  display.display();

  // ----------------------------------------------------
  // LORA
  // ----------------------------------------------------

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
  LoRa.setPins(LORA_NSS, -1, -1);   // NSS, RESET, DIO0

  if (!LoRa.begin(433E6)) {

    Serial.println("ERROR: No se pudo iniciar LoRa");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("ERROR LORA");
    display.display();

    while (true);
  }

  Serial.println("LoRa iniciado correctamente.");
  Serial.println("Esperando mensajes...");
}

// ======================================================
// LOOP
// ======================================================

void loop() {

  int packetSize = LoRa.parsePacket();

  if (packetSize) {

    String paquete = "";

    while (LoRa.available()) {
      paquete += (char)LoRa.read();
    }

    procesarMensaje(paquete);
  }
}