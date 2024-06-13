#include <Arduino.h>
#include <Wire.h>
#include <AHT10.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>

AHT10 aht10;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Dirección I2C y dimensiones del display LCD

void setup() {
  Serial.begin(115200);
  Wire.begin(); // Inicializa la comunicación I2C
  lcd.init();   // Inicializa el display LCD
  lcd.backlight();
  
  if (!aht10.begin()) {
    Serial.println("Error al inicializar el sensor AHT10");
    while (1);
  }
}

void loop() {
  delay(2000); // Espera 2 segundos entre lecturas
  
  float temp = aht10.readTemperature(); // Lee la temperatura en Celsius
  float hum = aht10.readHumidity();     // Lee la humedad relativa
  
  // Imprime los datos en el monitor serial
  Serial.print("Temperatura: ");
  Serial.print(temp);
  Serial.print(" °C\t");
  Serial.print("Humedad: ");
  Serial.print(hum);
  Serial.println("%");

  // Imprime los datos en el display LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Humedad: ");
  lcd.print(hum);
  lcd.print("%");
}
