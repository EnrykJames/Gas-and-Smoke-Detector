#define BLYNK_TEMPLATE_ID "TMPL62NHGJqPv"
#define BLYNK_TEMPLATE_NAME "Smoke Detector"

#define BLYNK_FIRMWARE_VERSION        "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG

// Uncomment your board, or configure a custom board in Settings.h
//#define USE_SPARKFUN_BLYNK_BOARD
#define USE_NODE_MCU_BOARD
//#define USE_WITTY_CLOUD_BOARD
//#define USE_WEMOS_D1_MINI

#include "BlynkEdgent.h"
#include <MQ2.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>



#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BUZZ      12 //D6 
#define LED       14 //D5
int pin = A0;  // MQ2 sensor pin
float lpg, co, smoke;
MQ2 mq2(pin);

int button1 = 0;  // LPG display button
int button2 = 0;  // CO display button
bool smokeAlertSent = false;  // Flag to prevent alert spamming

SimpleTimer timer;

void setup()
{
  Serial.begin(115200);
  delay(100);

  BlynkEdgent.begin();

  pinMode(BUZZ, OUTPUT);
  pinMode(LED, OUTPUT);

  digitalWrite(BUZZ, LOW);
  digitalWrite(LED, LOW);

  // Calibrate the sensor
  mq2.begin();

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  delay(2000);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 0);
  display.println(" IoT Smoke ");
  display.setCursor(0, 20);
  display.println("  Detector  ");
  display.display();
  delay(1000);

  // Set the timer to send sensor data every second
  timer.setInterval(1000L, sendSensorData);
}

void loop() {
  timer.run();  // Initiates SimpleTimer
  BlynkEdgent.run();
}

// Function to send sensor data to Blynk and control the OLED display
void sendSensorData()
{
  float* values = mq2.read(true);  // Set false if you don't want to print the values to Serial
  co = mq2.readCO();
  smoke = mq2.readSmoke();
  lpg = mq2.readLPG();

  // Display the selected gas (LPG or CO) or smoke level by default
  if (button1 == 1)
  {
    // Display LPG
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("    LPG   ");
    display.setCursor(10, 30);
    display.print(lpg);
    display.setTextSize(1);
    display.print(" PPM");
    display.display();
  }
  else if (button2 == 1)
  {
    // Display CO
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("    CO    ");
    display.setCursor(10, 30);
    display.print(co);
    display.setTextSize(1);
    display.print(" PPM");
    display.display();
  }
  else {
    // Display Smoke
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("   SMOKE   ");
    display.setCursor(10, 30);
    display.print(smoke);
    display.setTextSize(1);
    display.print(" PPM");
    display.display();
  }

  // Send sensor data to Blynk
  Blynk.virtualWrite(V1, smoke);
  Blynk.virtualWrite(V2, lpg);
  Blynk.virtualWrite(V3, co);

  // Check if smoke levels exceed the threshold and trigger an alert
  if (smoke > 50 && !smokeAlertSent) {
    Blynk.logEvent("smoke", "Smoke Detected!");
    smokeAlertSent = true;  // Prevent alert spamming
    digitalWrite(BUZZ, HIGH);
    digitalWrite(LED, HIGH);
  }
  else if (smoke <= 50 && smokeAlertSent) {
    smokeAlertSent = false;  // Reset the alert if levels drop
    digitalWrite(BUZZ, LOW);
    digitalWrite(LED, LOW);
  }
}

// Blynk button for LPG display
BLYNK_WRITE(V4) {
  button1 = param.asInt();  // Assign incoming value from pin V4
  if (button1 == 1) {
    button2 = 0;  // Reset CO button when LPG button is pressed
  }
}

// Blynk button for CO display
BLYNK_WRITE(V5) {
  button2 = param.asInt();  // Assign incoming value from pin V5
  if (button2 == 1) {
    button1 = 0;  // Reset LPG button when CO button is pressed
  }
}