#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SensirionI2CScd4x.h>
#include <Fonts/FreeMono9pt7b.h>
// include Fonts/Tiny3x3a2pt7b.h
#include <Fonts/Org_01.h>

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);
SensirionI2CScd4x scd4x;

// Define circular buffer for CO2 values
const int BUFFER_SIZE = 128;
int co2Array[BUFFER_SIZE];
int co2Index = 0;

// Define circular buffer for temperature values
const int TEMP_BUFFER_SIZE = 128;
float tempArray[TEMP_BUFFER_SIZE];
int tempIndex = 0;

// Define circular buffer for humidity values
const int HUM_BUFFER_SIZE = 128;
float humArray[HUM_BUFFER_SIZE];
int humIndex = 0;

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
int currentMenuOption = 2;

void loop()
{
  if (millis() - previousMillis >= 11000)
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
      co2Array[co2Index] = tempCO2;
      tempIndex = (tempIndex + 1) % TEMP_BUFFER_SIZE;
      tempArray[tempIndex] = tempTemp;
      humIndex = (humIndex + 1) % HUM_BUFFER_SIZE;
      humArray[humIndex] = tempHum;
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
    // in top left corner draw 24m CO2 graph
    display.setCursor(0, 10);
    display.setFont(&Org_01);
    display.print("24m CO2");
    // below draw points from left to right representinh 1 hour pass
    for (int i = 0; i < 128; i = i + 11)
    {
      display.drawPixel(i, 15, SSD1306_WHITE); // Plot CO2 values
    }

    // in top right  corner draw 2000
    display.setCursor(100, 10);
    // use tiny font
    display.setFont(&Org_01);
    display.print("2000");
    // in bottom right corner draw 400
    display.setCursor(100, 60);
    display.print("400");
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
      display.drawPixel(i, map(co2Array[i], 400, 2000, display.height(), 0), SSD1306_WHITE); // Plot CO2 values
    }
    delay(1000);
    break;

  case 2:
    display.clearDisplay();
    // in top left corner draw 24m CO2 graph
    display.setCursor(0, 10);
    display.setFont(&Org_01);
    display.print("24m temperature");
    // below draw points from left to right representinh 1 hour pass
    for (int i = 0; i < 128; i = i + 11)
    {
      display.drawPixel(i, 15, SSD1306_WHITE); // Plot CO2 values
    }

    // in top right  corner draw 2000
    display.setCursor(100, 10);
    // use tiny font
    display.setFont(&Org_01);
    display.print("50");
    // in bottom right corner draw 400
    display.setCursor(100, 60);
    display.print("0");
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
      display.drawPixel(i, map(tempArray[i], 0, 50, display.height(), 0), SSD1306_WHITE); // Plot CO2 values
    }
    delay(1000);
    break;
  }
  display.display();
}
