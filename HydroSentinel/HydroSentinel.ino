#define BLYNK_TEMPLATE_ID "TMPL65I_UU9i9"
#define BLYNK_TEMPLATE_NAME "HydroSentinel"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "PGS_FKK9_bvIKYaYtAPR3kXi2rtHJ9e1";
char ssid[] = "TP-LINK_A4D3C8";
char pass[] = "W.cfT3#*CA@";

BlynkTimer timer;

// Pin assignments
#define MOISTURE_PIN A0
#define RELAY_PIN    D7   // Pump (active LOW)
#define GREEN_PIN    D5   // Wet indicator
#define RED_PIN      D6   // Dry indicator
#define BLUE_PIN     D1   // Pump active LED

// System variables
int threshold = 35;
bool manualPump = false;
int lastPumpState = -1;    // Prevent repeating logs

// ----------- Moisture Reading -----------
void readMoisture() {
  int raw = analogRead(MOISTURE_PIN);

  // Map resistive sensor (HIGH = dry, LOW = wet)
  int moisture = map(raw, 1023, 0, 0, 100);
  moisture = constrain(moisture, 0, 100);

  // Send to Blynk widgets
  Blynk.virtualWrite(V0, moisture);
  Blynk.virtualWrite(V6, moisture);

  controlSystem(moisture);
}

// ----------- Pump + LED Control -----------
void turnPumpOn() {
  digitalWrite(RELAY_PIN, LOW);  // Active LOW → ON
  digitalWrite(BLUE_PIN, HIGH);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(RED_PIN, HIGH);

  // Dashboard LEDs
  Blynk.virtualWrite(V1, 0); // Green OFF
  Blynk.virtualWrite(V2, 1); // Red ON
  Blynk.virtualWrite(V3, 1); // Blue ON

  if (lastPumpState != 1) {
    Blynk.virtualWrite(V8, "Pump ON");
    lastPumpState = 1;
  }
}

void turnPumpOff() {
  digitalWrite(RELAY_PIN, HIGH); // Pump OFF
  digitalWrite(BLUE_PIN, LOW);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(RED_PIN, LOW);

  // Dashboard LEDs
  Blynk.virtualWrite(V1, 1); // Green ON
  Blynk.virtualWrite(V2, 0); // Red OFF
  Blynk.virtualWrite(V3, 0); // Blue OFF

  if (lastPumpState != 0) {
    Blynk.virtualWrite(V8, "Pump OFF");
    lastPumpState = 0;
  }
}

// ----------- Main Control Logic -----------
void controlSystem(int moisture) {

  if (manualPump) {
    turnPumpOn();
    Blynk.virtualWrite(V5, "Pump ON (Manual)");
    return;
  }

  if (moisture < threshold) {
    turnPumpOn();
    Blynk.virtualWrite(V5, "Pump ON (Dry Soil)");
  }
  else {
    turnPumpOff();
    Blynk.virtualWrite(V5, "Pump OFF (Wet Soil)");
  }
}

// ----------- Sync Blynk Values on Connect -----------
BLYNK_CONNECTED() {
  Blynk.syncAll();
  Blynk.virtualWrite(V7, threshold);  // Refresh slider
}

// ----------- Manual Pump Switch (V3) -----------
BLYNK_WRITE(V3) {
  manualPump = param.asInt();

  if (manualPump) {
    turnPumpOn();
    Blynk.virtualWrite(V8, "Manual Pump ON");
  } else {
    turnPumpOff();
    Blynk.virtualWrite(V8, "Manual Pump OFF");
  }
}

// ----------- Threshold Slider (V7) -----------
BLYNK_WRITE(V7) {
  threshold = param.asInt();
}

// ----------- Setup -----------
void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, HIGH); // Pump OFF initially

  Blynk.begin(auth, ssid, pass);

  timer.setInterval(2000L, readMoisture); // Read every 2 seconds
}

// ----------- Loop -----------
void loop() {
  Blynk.run();
  timer.run();
}
