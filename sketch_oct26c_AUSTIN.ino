#include <LiquidCrystal.h>

// --- LCD setup (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// --- Pin Configuration ---
#define PH_PIN A2
#define TDS_PIN A7
#define DO_PIN A5
#define TURBIDITY_PIN A1
#define AIR_TEMP_PIN A4   // TMP36 for Air Temp
#define WATER_TEMP_PIN A3 // TMP36 for Water Temp

// --- Actuators ---
#define PH_PUMP_UP 13
#define PH_PUMP_DOWN 8
#define NUTRIENT_PUMP_UP 22
#define NUTRIENT_PUMP_DOWN 23
#define AIR_PUMP 9
#define FAN 24
#define BUZZER 46
#define RED_LED 29
#define GREEN_LED 44

// --- Thresholds ---
float PH_LOW = 5.5;
float PH_HIGH = 6.5;
int TDS_LOW = 500;        // ppm
int TDS_HIGH = 800;       // ppm
float DO_THRESHOLD = 6.0; // mg/L
float AIR_TEMP_THRESHOLD = 28.0;
float WATER_TEMP_THRESHOLD = 26.0;
int TURBIDITY_THRESHOLD = 400; // higher = more light = cleaner

// --- Variables ---
float phValue, doValue, airTemp, waterTemp;
int tdsValue, turbidityValue;

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);

  pinMode(PH_PUMP_UP, OUTPUT);
  pinMode(PH_PUMP_DOWN, OUTPUT);
  pinMode(NUTRIENT_PUMP_UP, OUTPUT);
  pinMode(NUTRIENT_PUMP_DOWN, OUTPUT);
  pinMode(AIR_PUMP, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  lcd.print("Smart Hydroponic");
  delay(1000);
  lcd.clear();
}

// --- TMP36 Conversion Function ---
float readTMP36(int pin) {
  int reading = analogRead(pin);
  float voltage = reading * (5.0 / 1023.0);
  return (voltage - 0.5) * 100.0;
}

void loop() {
  // --- Read All Sensors ---
  phValue = analogRead(PH_PIN) * (14.0 / 1023.0);
  tdsValue = analogRead(TDS_PIN);
  doValue = analogRead(DO_PIN) * (10.0 / 1023.0);
  turbidityValue = analogRead(TURBIDITY_PIN);
  airTemp = readTMP36(AIR_TEMP_PIN);
  waterTemp = readTMP36(WATER_TEMP_PIN);

  // --- pH Control ---
  if (phValue > PH_HIGH) {
    digitalWrite(PH_PUMP_DOWN, HIGH);
    digitalWrite(PH_PUMP_UP, LOW);
    digitalWrite(BUZZER, HIGH);
    digitalWrite(RED_LED, HIGH);
  } else if (phValue < PH_LOW) {
    digitalWrite(PH_PUMP_UP, HIGH);
    digitalWrite(PH_PUMP_DOWN, LOW);
    digitalWrite(BUZZER, HIGH);
    digitalWrite(RED_LED, HIGH);
  } else {
    digitalWrite(PH_PUMP_UP, LOW);
    digitalWrite(PH_PUMP_DOWN, LOW);
    digitalWrite(BUZZER, LOW);
    digitalWrite(RED_LED, LOW);
  }

  // --- TDS Control ---
  if (tdsValue > TDS_HIGH) {
    digitalWrite(NUTRIENT_PUMP_DOWN, HIGH);
    digitalWrite(NUTRIENT_PUMP_UP, LOW);
  } else if (tdsValue < TDS_LOW) {
    digitalWrite(NUTRIENT_PUMP_UP, HIGH);
    digitalWrite(NUTRIENT_PUMP_DOWN, LOW);
  } else {
    digitalWrite(NUTRIENT_PUMP_UP, LOW);
    digitalWrite(NUTRIENT_PUMP_DOWN, LOW);
  }

  // --- DO Control ---
  if (doValue < DO_THRESHOLD) {
    digitalWrite(AIR_PUMP, HIGH);
  } else {
    digitalWrite(AIR_PUMP, LOW);
  }

  // --- Temperature Control (shared fan) ---
  if (airTemp > AIR_TEMP_THRESHOLD || waterTemp > WATER_TEMP_THRESHOLD) {
    digitalWrite(FAN, HIGH);
  } else {
    digitalWrite(FAN, LOW);
  }

  // --- Turbidity (LDR) Control ---
  if (turbidityValue > TURBIDITY_THRESHOLD) {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
  } else {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
  }

  // --- LCD Display (Dynamic Info) ---
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("pH:");
  lcd.print(phValue, 1);
  lcd.print(" TDS:");
  lcd.print(tdsValue);

  lcd.setCursor(0, 1);
  lcd.print("AT:");
  lcd.print(airTemp, 1);
  lcd.print(" WT:");
  lcd.print(waterTemp, 1);

  delay(300); // shorter delay = faster refresh, still stable
}
