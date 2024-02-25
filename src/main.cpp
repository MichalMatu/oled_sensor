#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SensirionI2CScd4x.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/Org_01.h>

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);
SensirionI2CScd4x scd4x;

// *****************************************************************************

// Define the pins for the fans
const int fanPin1 = 13; // PWM pin for fan 1
const int fanPin2 = 12; // PWM pin for fan 2

int fanSpeed1 = 0;
int fanSpeed2 = 0;

// *****************************************************************************

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

// *****************************************************************************
// define 4 button pins left, right, up, down 14, 27, 26, 25

int buttonPins[] = {14, 27, 26, 25};

// *****************************************************************************

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  uint16_t error;
  char errorMessage[256];

  scd4x.begin(Wire);

  // Temporary variables for CO2 readings
  uint16_t CO2;
  float Temp;
  float Hum;

  // set button pins to high
  for (int i = 0; i < 4; i++)
  {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  pinMode(fanPin1, OUTPUT);
  pinMode(fanPin2, OUTPUT);

  // Read initial CO2 measurement
  error = scd4x.readMeasurement(CO2, Temp, Hum);
  if (error)
  {
    Serial.print("Error trying to execute readMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }
  else
  {
    co2Array[co2Index] = CO2;
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
int currentMenuOption = 4;

unsigned long menuMillis = 0;
// implement debouncing for buttons 100ms
int debounceDelay = 100;

// Define variables to store previous values
float previousTemperature = 0.0;
float previousHumidity = 0.0;
int previousCO2 = 0;

void loop()
{
  // Read button states
  int leftButtonState = digitalRead(buttonPins[0]);
  int rightButtonState = digitalRead(buttonPins[1]);
  int upButtonState = digitalRead(buttonPins[2]);
  int downButtonState = digitalRead(buttonPins[3]);

  // *****************************************************************************

  // // Set the speed of fan 1 to 50% duty cycle
  // analogWrite(fanPin1, 128); // 50% duty cycle out of 255

  // // Set the speed of fan 2 to 75% duty cycle
  // analogWrite(fanPin2, 191); // 75% duty cycle out of 255

  // *****************************************************************************

  // Check if left button is pressed
  if (leftButtonState == LOW)
  {
    currentMenuOption = (currentMenuOption + 1) % 5;
    delay(debounceDelay); // Debounce delay
  }

  // Check if right button is pressed
  if (rightButtonState == LOW)
  {
    currentMenuOption = (currentMenuOption - 1 + 5) % 5;
    delay(debounceDelay); // Debounce delay
  }

  // Check if up button is pressed
  if (upButtonState == LOW)
  {
    // Action for up button
    delay(debounceDelay); // Debounce delay
  }

  // Check if down button is pressed
  if (downButtonState == LOW)
  {
    // Action for down button
    delay(debounceDelay); // Debounce delay
  }

  // Read CO2 measurements every 11 seconds
  if (millis() - previousMillis >= 11000)
  {
    // Read CO2 measurements
    uint16_t CO2;
    float Temp;
    float Hum;
    uint16_t error = scd4x.readMeasurement(CO2, Temp, Hum);
    if (!error)
    {
      // Update circular buffers
      co2Index = (co2Index + 1) % BUFFER_SIZE;
      co2Array[co2Index] = CO2;
      tempIndex = (tempIndex + 1) % TEMP_BUFFER_SIZE;
      tempArray[tempIndex] = Temp;
      humIndex = (humIndex + 1) % HUM_BUFFER_SIZE;
      humArray[humIndex] = Hum;
      // Update temperature and humidity
      temperature = Temp;
      humidity = Hum;
    }
    previousMillis = millis();
  }

  switch (currentMenuOption)
  {
  case 0:
    // Check if temperature has changed
    if (temperature != previousTemperature || humidity != previousHumidity || co2Array[co2Index] != previousCO2)
    {
      display.clearDisplay();
      display.setFont(&FreeMono12pt7b);
      display.setTextSize(1);
      display.setCursor(0, 15);
      display.print("T: ");
      display.print((int)temperature);
      display.print(" C");
      display.println();
      display.print("H: ");
      display.print((int)humidity);
      display.print(" %");
      display.println();
      display.print("CO2:");
      display.print(co2Array[co2Index]);

      // Update previous values
      previousTemperature = temperature;
      previousHumidity = humidity;
      previousCO2 = co2Array[co2Index];
    }
    break;

  case 1:
    display.clearDisplay();
    display.setCursor(0, 10);
    display.setFont(&Org_01);
    display.print("24 MIN CO2");

    for (int i = 0; i < 128; i = i + 11)
    {
      display.drawPixel(i, 40, SSD1306_WHITE);
    }

    display.setCursor(0, 60);
    display.setFont(&FreeMono9pt7b);
    display.print(co2Array[co2Index]);
    display.print(" ppm");

    for (int i = 0; i < BUFFER_SIZE; i++)
    {
      display.drawPixel(i, map(co2Array[i], 400, 2000, display.height(), 0), SSD1306_WHITE); // Plot CO2 values
    }
    delay(100);
    break;

  case 2:
    display.clearDisplay();
    display.setCursor(0, 10);
    display.setFont(&Org_01);
    display.print("24 min temperature");

    for (int i = 0; i < 128; i = i + 11)
    {
      display.drawPixel(i, 35, SSD1306_WHITE);
    }

    display.setCursor(0, 60);
    display.setFont(&FreeMono9pt7b);
    display.print((int)tempArray[tempIndex]);
    display.print(" C");

    for (int i = 0; i < BUFFER_SIZE; i++)
    {
      display.drawPixel(i, map(tempArray[i], 0, 50, display.height(), 0), SSD1306_WHITE);
    }
    delay(100);
    break;

  case 3:
    display.clearDisplay();
    // in top left corner draw 24m humidity graph
    display.setCursor(0, 10);
    display.setFont(&Org_01);
    display.print("24m humidity");

    for (int i = 0; i < 128; i = i + 11)
    {
      display.drawPixel(i, 30, SSD1306_WHITE);
    }
    // in bottom left corner dispaly current humidity
    display.setCursor(0, 60);
    display.setFont(&FreeMono9pt7b);
    display.print((int)humArray[humIndex]);
    display.print("%");

    for (int i = 0; i < BUFFER_SIZE; i++)
    {
      display.drawPixel(i, map(humArray[i], 0, 100, display.height(), 0), SSD1306_WHITE); // Plot CO2 values
    }
    delay(100);
    break;
  case 4:
    // Check if up button is pressed
    if (upButtonState == LOW)
    {
      // Increase fan speed
      fanSpeed1 = min(fanSpeed1 + 8, 255); // Increment fan speed (limit to max)
      fanSpeed2 = min(fanSpeed2 + 8, 255); // Increment fan speed (limit to max)
      delay(debounceDelay);                // Debounce delay
      // set analog write to fan pins to fan speed 1 and 2
      analogWrite(fanPin1, fanSpeed1);
    }

    // Check if down button is pressed
    if (downButtonState == LOW)
    {
      // Decrease fan speed
      fanSpeed1 = max(fanSpeed1 - 8, 0); // Decrement fan speed (limit to min)
      fanSpeed2 = max(fanSpeed2 - 8, 0); // Decrement fan speed (limit to min)
      delay(debounceDelay);              // Debounce delay
      // set analog write to fan pins to fan speed 1 and 2
      analogWrite(fanPin1, fanSpeed1);
    }

    // Display fan speeds on OLED screen
    display.clearDisplay();
    display.setFont(&FreeMono9pt7b);
    display.setCursor(0, 15);
    display.print("Fan 1: ");
    display.print(map(fanSpeed1, 0, 255, 0, 100));
    display.println("%");
    display.print("Fan 2: ");
    display.print(map(fanSpeed2, 0, 255, 0, 100));
    display.println("%");
    display.display();

    break;
  }
  display.display();
}