
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD configuration
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Entry ultrasonic sensor
#define ENTRY_TRIG 5
#define ENTRY_ECHO 18

// Exit ultrasonic sensor
#define EXIT_TRIG 17
#define EXIT_ECHO 16

// LED pins
#define GREEN_LED 25
#define YELLOW_LED 26
#define RED_LED 27

// Buzzer pin
#define BUZZER 19

// Queue variables
int queueCount = 0;

bool entryArmed = true;
bool exitArmed = true;

unsigned long lastUpdate = 0;
unsigned long lastAlarm = 0;

// Detection settings
const int DETECT_DISTANCE = 80;
const int CLEAR_DISTANCE = 100;

// Average service time per person
const int SERVICE_TIME = 30;

// Read ultrasonic sensor distance
float getDistance(int trigPin, int echoPin) {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 25000);

  if (duration == 0) {
    return -1;
  }

  float distance = duration * 0.0343 / 2;

  return distance;
}

// Display queue information
void updateQueueStatus() {

  int waitingTime =
    (queueCount * SERVICE_TIME + 59) / 60;

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Queue: ");
  lcd.print(queueCount);
  lcd.print(" People");

  lcd.setCursor(0, 1);
  lcd.print("Wait: ");
  lcd.print(waitingTime);
  lcd.print(" min");

  // Turn off all LEDs
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);

  // Low queue
  if (queueCount <= 3) {

    digitalWrite(GREEN_LED, HIGH);

  }

  // Medium queue
  else if (queueCount <= 7) {

    digitalWrite(YELLOW_LED, HIGH);

  }

  // High queue
  else {

    digitalWrite(RED_LED, HIGH);

  }

  Serial.print("Queue Count: ");
  Serial.println(queueCount);

  Serial.print("Waiting Time: ");
  Serial.print(waitingTime);
  Serial.println(" minutes");
}

// Setup
void setup() {

  Serial.begin(115200);

  pinMode(ENTRY_TRIG, OUTPUT);
  pinMode(ENTRY_ECHO, INPUT);

  pinMode(EXIT_TRIG, OUTPUT);
  pinMode(EXIT_ECHO, INPUT);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  pinMode(BUZZER, OUTPUT);

  Wire.begin(21, 22);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("SMART QUEUE");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  delay(2000);

  updateQueueStatus();

  Serial.println("Smart Queue System Started");
}

// Main loop
void loop() {

  float entryDistance =
    getDistance(ENTRY_TRIG, ENTRY_ECHO);

  delay(60);

  float exitDistance =
    getDistance(EXIT_TRIG, EXIT_ECHO);

  // Person entering
  if (entryDistance > 0 &&
      entryDistance < DETECT_DISTANCE &&
      entryArmed) {

    queueCount++;

    entryArmed = false;

    Serial.println("Person Entered");

    updateQueueStatus();
  }

  // Reset entry sensor
  if (entryDistance < 0 ||
      entryDistance > CLEAR_DISTANCE) {

    entryArmed = true;
  }

  // Person leaving
  if (exitDistance > 0 &&
      exitDistance < DETECT_DISTANCE &&
      exitArmed) {

    if (queueCount > 0) {
      queueCount--;
    }

    exitArmed = false;

    Serial.println("Person Exited");

    updateQueueStatus();
  }

  // Reset exit sensor
  if (exitDistance < 0 ||
      exitDistance > CLEAR_DISTANCE) {

    exitArmed = true;
  }

  // High queue buzzer alert
  if (queueCount > 7) {

    if (millis() - lastAlarm >= 2000) {

      tone(BUZZER, 2000, 200);

      lastAlarm = millis();
    }

  }

  delay(100);
}
