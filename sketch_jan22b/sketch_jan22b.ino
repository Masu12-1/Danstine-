#include <LiquidCrystal.h>

// --- LCD setup (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// --- Analog Sensors ---
#define PH_PIN A2
#define TDS_PIN A7
#define DO_PIN A5
#define TURBIDITY_PIN A1
#define AIR_TEMP_PIN A4
#define WATER_TEMP_PIN A3

// --- Water Level Potentiometer & Inlet Valve ---
#define WATER_LEVEL_POT_PIN A8   // Potentiometer (HG)
#define INLET_VALVE_MOTOR 12    // Motor / relay for inlet valve

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
int TDS_LOW = 500;
int TDS_HIGH = 800;
float DO_THRESHOLD = 6.0;
float AIR_TEMP_THRESHOLD = 28.0;
float WATER_TEMP_THRESHOLD = 26.0;
int TURBIDITY_THRESHOLD = 400;

// --- Variables ---
float phValue, doValue, airTemp, waterTemp;
int tdsValue, turbidityValue;
int waterLevelRaw;
int waterLevelPercent;

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);

  // Actuators
  pinMode(PH_PUMP_UP, OUTPUT);
  pinMode(PH_PUMP_DOWN, OUTPUT);
  pinMode(NUTRIENT_PUMP_UP, OUTPUT);
  pinMode(NUTRIENT_PUMP_DOWN, OUTPUT);
  pinMode(AIR_PUMP, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  pinMode(INLET_VALVE_MOTOR, OUTPUT);
  digitalWrite(INLET_VALVE_MOTOR, LOW); // valve initially CLOSED

  lcd.print("Smart Hydroponic");
  delay(1000);
  lcd.clear();
}

// --- TMP36 Conversion ---
float readTMP36(int pin) {
  int reading = analogRead(pin);
  float voltage = reading * (5.0 / 1023.0);
  return (voltage - 0.5) * 100.0;
}

void loop() {
  // --- Read Sensors ---
  phValue = analogRead(PH_PIN) * (14.0 / 1023.0);
  tdsValue = analogRead(TDS_PIN);
  doValue = analogRead(DO_PIN) * (10.0 / 1023.0);
  turbidityValue = analogRead(TURBIDITY_PIN);
  airTemp = readTMP36(AIR_TEMP_PIN);
  waterTemp = readTMP36(WATER_TEMP_PIN);

  // --- Read Potentiometer (Water Level Simulation) ---
  waterLevelRaw = analogRead(WATER_LEVEL_POT_PIN);
  waterLevelPercent = map(waterLevelRaw, 0, 1023, 0, 100);

  // --- Inlet Valve Control ---
  // < 50% → OPEN valve
  // 50–99% → keep filling (OPEN)
  // 100% → CLOSE valve
  if (waterLevelPercent < 100) {
    digitalWrite(INLET_VALVE_MOTOR, HIGH); // OPEN valve
  } else {
    digitalWrite(INLET_VALVE_MOTOR, LOW);  // CLOSE valve
  }

  // --- pH Control ---
  if (phValue > PH_HIGH) {
    digitalWrite(PH_PUMP_DOWN, HIGH);
    digitalWrite(PH_PUMP_UP, LOW);
    digitalWrite(BUZZER, HIGH);
    digitalWrite(RED_LED, HIGH);
  }
  else if (phValue < PH_LOW) {
    digitalWrite(PH_PUMP_UP, HIGH);
    digitalWrite(PH_PUMP_DOWN, LOW);
    digitalWrite(BUZZER, HIGH);
    digitalWrite(RED_LED, HIGH);
  }
  else {
    digitalWrite(PH_PUMP_UP, LOW);
    digitalWrite(PH_PUMP_DOWN, LOW);
    digitalWrite(BUZZER, LOW);
    digitalWrite(RED_LED, LOW);
  }

  // --- TDS Control ---
  if (tdsValue > TDS_HIGH) {
    digitalWrite(NUTRIENT_PUMP_DOWN, HIGH);
    digitalWrite(NUTRIENT_PUMP_UP, LOW);
  }
  else if (tdsValue < TDS_LOW) {
    digitalWrite(NUTRIENT_PUMP_UP, HIGH);
    digitalWrite(NUTRIENT_PUMP_DOWN, LOW);
  }
  else {
    digitalWrite(NUTRIENT_PUMP_UP, LOW);
    digitalWrite(NUTRIENT_PUMP_DOWN, LOW);
  }

  // --- DO Control ---
  digitalWrite(AIR_PUMP, doValue < DO_THRESHOLD ? HIGH : LOW);

  // --- Temperature Control ---
  digitalWrite(FAN,
    (airTemp > AIR_TEMP_THRESHOLD || waterTemp > WATER_TEMP_THRESHOLD)
    ? HIGH : LOW
  );

  // --- Turbidity Indicator ---
  digitalWrite(GREEN_LED, turbidityValue > TURBIDITY_THRESHOLD);
  digitalWrite(RED_LED, turbidityValue <= TURBIDITY_THRESHOLD);

  // --- LCD Display ---
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("pH:");
  lcd.print(phValue, 1);
  lcd.print(" TDS:");
  lcd.print(tdsValue);

  lcd.setCursor(0, 1);
  lcd.print("WL:");
  lcd.print(waterLevelPercent);
  lcd.print("%");

  delay(300);
}
