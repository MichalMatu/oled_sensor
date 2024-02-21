#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SensirionI2CScd4x.h>
#include <Fonts/FreeMono9pt7b.h>
#define WIRE Wire

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &WIRE);

SensirionI2CScd4x scd4x;

// Read Measurement
uint16_t co2 = 0;
float temperature = 0.0f;
float humidity = 0.0f;
bool isDataReady = false;

void printUint16Hex(uint16_t value)
{
  Serial.print(value < 4096 ? "0" : "");
  Serial.print(value < 256 ? "0" : "");
  Serial.print(value < 16 ? "0" : "");
  Serial.print(value, HEX);
}

void printSerialNumber(uint16_t serial0, uint16_t serial1, uint16_t serial2)
{
  Serial.print("Serial: 0x");
  printUint16Hex(serial0);
  printUint16Hex(serial1);
  printUint16Hex(serial2);
  Serial.println();
}

void setup()
{
  Serial.begin(115200);
  // *************** SCD4x ***************
  Wire.begin();

  uint16_t error;
  char errorMessage[256];

  scd4x.begin(Wire);
  scd4x.readMeasurement(co2, temperature, humidity);
  // stop potentially previously started measurement
  error = scd4x.stopPeriodicMeasurement();
  if (error)
  {
    Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }

  uint16_t serial0;
  uint16_t serial1;
  uint16_t serial2;
  error = scd4x.getSerialNumber(serial0, serial1, serial2);
  if (error)
  {
    Serial.print("Error trying to execute getSerialNumber(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }
  else
  {
    printSerialNumber(serial0, serial1, serial2);
  }

  // Start Measurement
  error = scd4x.startPeriodicMeasurement();
  if (error)
  {
    Serial.print("Error trying to execute startPeriodicMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }

  Serial.println("Waiting for first measurement... (5 sec)");
  // wait for first measurement

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  display.setFont(&FreeMono9pt7b);
  // Clear the buffer.
  display.clearDisplay();
  // text display tests
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(50, 40);
  display.print("OK");
  display.display(); // actually display all of the above
}

unsigned long previousMillis = 0;
int currentMenuOption = 0;

// delcare array to store co2 values
int co2Array[128];

void loop()
{
  // Check if 10 seconds have passed since last measurement
  if (millis() - previousMillis >= 10000)
  {
    // Read new measurement
    scd4x.readMeasurement(co2, temperature, humidity);

    previousMillis = millis();
  }

  // Handle menu navigation and actions using a switch-case
  switch (currentMenuOption)
  {
  case 0: // Display sensor readings
    // Clear the buffer and set cursor position
    display.clearDisplay();
    display.setCursor(0, 10);

    // Display values with labels
    display.print("T: ");
    display.print((int)temperature);
    display.print(" C");
    display.println();
    display.print("H: ");
    display.print((int)humidity);
    display.print(" %RH");
    display.println();
    display.print("CO2:");
    display.print(co2);
    display.println("ppm");

    break;

  case 1:
    display.clearDisplay();
    display.drawPixel(10, 10, SSD1306_WHITE);
    // read from eeprom co2, temperature, humidity and display in serial monitor

    delay(1000);
    break;

    // Add more cases for other menu options
  }

  // Display the updated content
  display.display();
}
