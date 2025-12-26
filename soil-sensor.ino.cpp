/***************************************************
   Smart Plant Watering System using ESP8266 + Blynk IoT
   Relay default OFF, only ON when commanded
****************************************************/

#define BLYNK_TEMPLATE_ID "TMPL3eKjuJ4Ka"
#define BLYNK_TEMPLATE_NAME "Smart plant watering system"
#define BLYNK_AUTH_TOKEN  "3zCMuUZ2A29d9IEmNzwlOv1i9HTuLUkR"

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal_I2C.h>

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// WiFi credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Redmi12";
char pass[] = "123456789";

BlynkTimer timer;
bool Relay = 0;
bool motorTestDone = false; // new flag

// Pin setup
#define sensor A0
#define waterPump 14  // GPIO14 (D5) connected to relay IN pin

// Relay trigger type (LOW for low-level trigger relay modules)
#define RELAY_TRIGGER LOW
#define RELAY_OFF (RELAY_TRIGGER == LOW ? HIGH : LOW)
#define RELAY_ON  (RELAY_TRIGGER == LOW ? LOW  : HIGH)

int dryValue = 850;  // Calibration dry soil
int wetValue = 350;  // Calibration wet soil

void setup() {
  Serial.begin(9600);

  pinMode(waterPump, OUTPUT);
  digitalWrite(waterPump, RELAY_OFF);  // Ensure pump is OFF from the start

  lcd.init();
  lcd.backlight();

  Serial.println("Connecting to WiFi...");
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected!");
  delay(500);

  // Read initial calibration (fast, no big delay)
  dryValue = analogRead(sensor);
  wetValue = dryValue - 500; // initial guess, adjust after testing

  // Start moisture check every 2 sec
  timer.setInterval(2000L, soilMoistureSensor);
}

// Blynk button control (V1)
BLYNK_WRITE(V1) {
  Relay = param.asInt();
  if (Relay == 1) {
    digitalWrite(waterPump, RELAY_ON); // Pump ON
    lcd.setCursor(0, 1);
    lcd.print("Motor is ON ");
    Serial.println("Pump: ON");
  } else {
    digitalWrite(waterPump, RELAY_OFF); // Pump OFF
    lcd.setCursor(0, 1);
    lcd.print("Motor is OFF");
    Serial.println("Pump: OFF");
  }
}

// Soil moisture check
void soilMoistureSensor() {
  int raw = analogRead(sensor);
  int moisture = map(raw, dryValue, wetValue, 0, 100);
  moisture = constrain(moisture, 0, 100);

  Blynk.virtualWrite(V0, moisture);

  lcd.setCursor(0, 0);
  lcd.print("Moisture:");
  lcd.print(moisture);
  lcd.print("%   ");

  Serial.print("Raw: ");
  Serial.print(raw);
  Serial.print(" | Moisture: ");
  Serial.print(moisture);
  Serial.println("%");
}

// Run motor test once after connect
void testMotorOnce() {
  if (!motorTestDone && Blynk.connected()) {
    Serial.println("Testing motor for 2 seconds...");
    lcd.setCursor(0, 0);
    lcd.print("Testing Motor");
    digitalWrite(waterPump, RELAY_ON);
    delay(2000);
    digitalWrite(waterPump, RELAY_OFF);
    lcd.clear();
    motorTestDone = true;
  }
}

void loop() {
  Blynk.run();
  timer.run();
  testMotorOnce();
}