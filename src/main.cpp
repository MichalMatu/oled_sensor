#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SensirionI2CScd4x.h>
#include <Fonts/FreeMono9pt7b.h>

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);
SensirionI2CScd4x scd4x;

// Define circular buffer for CO2 values
const int BUFFER_SIZE = 128;
int co2Array[BUFFER_SIZE];
int co2Index = 0;

float temperature = 0.0f;
float humidity = 0.0f;

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  uint16_t error;
  char errorMessage[256];

  scd4x.begin(Wire);

  // Temporary variables for CO2 readings
  uint16_t tempCO2;
  float tempTemp;
  float tempHum;

  // Read initial CO2 measurement
  error = scd4x.readMeasurement(tempCO2, tempTemp, tempHum);
  if (error)
  {
    Serial.print("Error trying to execute readMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }
  else
  {
    co2Array[co2Index] = tempCO2;
  }

  error = scd4x.stopPeriodicMeasurement();
  if (error)
  {
    Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }

  error = scd4x.startPeriodicMeasurement();
  if (error)
  {
    Serial.print("Error trying to execute startPeriodicMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }

  Serial.println("Waiting for first measurement... (5 sec)");

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setFont(&FreeMono9pt7b);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(50, 40);
  display.print("OK");
  display.display();
}

unsigned long previousMillis = 0;
int currentMenuOption = 1;

void loop()
{
  if (millis() - previousMillis >= 10000)
  {
    // Temporary variables for CO2 readings
    uint16_t tempCO2;
    float tempTemp;
    float tempHum;

    // Read new CO2 measurement
    uint16_t error = scd4x.readMeasurement(tempCO2, tempTemp, tempHum);
    if (!error)
    {
      // Move index circularly
      co2Index = (co2Index + 1) % BUFFER_SIZE;
      // Store CO2 reading in circular buffer
      co2Array[co2Index] = tempCO2;
    }

    previousMillis = millis();
  }

  switch (currentMenuOption)
  {
  case 0:
    display.clearDisplay();
    display.setCursor(0, 10);
    display.print("T: ");
    display.print((int)temperature);
    display.print(" C");
    display.println();
    display.print("H: ");
    display.print((int)humidity);
    display.print(" %RH");
    display.println();
    display.print("CO2:");
    display.print(co2Array[co2Index]); // Display latest CO2 value
    display.println("ppm");
    break;

  case 1:
    display.clearDisplay();
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
      display.drawPixel(i, map(co2Array[i], 400, 2000, display.height(), 0), SSD1306_WHITE); // Plot CO2 values
    }
    delay(1000);
    break;
  }
  display.display();
}
