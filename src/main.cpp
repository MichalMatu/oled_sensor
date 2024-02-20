#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <Arduino.h>

#include "heltec-eink-modules.h"
#define PIN_BUSY 5
#define PIN_CS 4
#define PIN_DC 14
DEPG0213RWS800 display(PIN_DC, PIN_CS, PIN_BUSY); // 2.13" V2 - BWR - Red Tab
// ****************************************************
#include <Wire.h>
#include <SensirionI2CScd4x.h>
// ****************************************************
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme; // I2C
// ****************************************************
#include "Adafruit_SGP30.h"

Adafruit_SGP30 sgp;

/* return absolute humidity [mg/m^3] with approximation formula
 * @param temperature [°C]
 * @param humidity [%RH]
 */
uint32_t getAbsoluteHumidity(float temperature, float humidity)
{
  // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
  const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
  const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity);                                                                // [mg/m^3]
  return absoluteHumidityScaled;
}
// ****************************************************

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
// ****************************************************
int analogMQ7 = 34;     // Analog input pin for MQ-7 sensor
int ledPin = 2;         // Device internal LED
int MQ7sensorValue = 0; // Value read from the sensor
int mq2Pin = 35;
int mq2Value = 0;
int mq9Pin = 32;
int mq9Value = 0;
// ****************************************************
// Structure example to receive data
// Must match the sender structure
typedef struct struct_message
{
  char a[32];
  float b;
  float c;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Char: ");
  Serial.println(myData.a);
  Serial.print("Float: ");
  Serial.println(myData.b);
  Serial.print("Float: ");
  Serial.println(myData.c);
}
// ****************************************************
void setup()
{
  // Initialize Serial Monitor
  Serial.begin(115200);

  // ****************************************************
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
  // ****************************************************

  Serial.println(F("BME680 async test"));

  if (!bme.begin())
  {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1)
      ;
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
  Serial.println("SGP30 test");

  if (!sgp.begin())
  {
    Serial.println("Sensor not found :(");
    while (1)
      ;
  }
  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);
  // ****************************************************

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // display mac adress in serial monitor
  Serial.print("ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println(F("MQ-7 Gas Sensor Flying-Fish started"));
  ledcWrite(analogMQ7, 255);
  delay(6000);
}
// ****************************************************
int mq7()
{
  ledcWrite(analogMQ7, 71); // 28% of 255
  MQ7sensorValue = analogRead(analogMQ7);
  return MQ7sensorValue;
}

// ****************************************************
void updateDisplay()
{
  DRAW(display)
  {
    // set size of text to be bigger
    // set rotation of text to be 90 degrees
    display.setRotation(3);
    display.setTextSize(1);

    // First Column
    display.setCursor(0, 0);
    display.print("MQ-7:");
    display.print(MQ7sensorValue);

    display.setCursor(60, 0);
    display.print("MQ-2:");
    mq2Value = analogRead(mq2Pin);
    display.print(mq2Value);

    display.setCursor(130, 0);
    display.print("MQ-9:");
    mq9Value = analogRead(mq9Pin);
    display.print(mq9Value);

    display.drawLine(0, 10, 250, 10, BLACK);

    display.setCursor(0, 15); // Adjust X-coordinate for the second column
    display.print("CO2:");
    display.print(co2);

    display.setCursor(60, 15);
    display.print("T:");
    display.print(temperature);

    display.setCursor(130, 15);
    display.print("H:");
    display.print(humidity);

    display.drawLine(0, 25, 250, 25, BLACK);

    // display bme680 data
    display.setCursor(0, 30);
    display.print("T:");
    display.print(bme.temperature);

    display.setCursor(60, 30);
    display.print("P:");
    display.print(bme.pressure / 100.0);

    display.setCursor(130, 30);
    display.print("H:");
    display.print(bme.humidity);

    display.drawLine(0, 40, 250, 40, BLACK);

    display.setCursor(0, 45);
    display.print("G:");
    display.print(bme.gas_resistance / 1000.0);

    display.setCursor(60, 45);
    display.print("A:");
    display.print(bme.readAltitude(SEALEVELPRESSURE_HPA));

    display.drawLine(0, 55, 250, 55, BLACK);

    display.setCursor(0, 60);
    display.print("MAC:");
    display.print(WiFi.macAddress());
    // draw line
    display.drawLine(0, 70, 250, 70, BLACK);
    // display readings from SGP30
    display.setCursor(0, 75);
    display.print("TVOC:");
    display.print(sgp.TVOC);

    display.setCursor(60, 75);
    display.print("eCO2:");
    display.print(sgp.eCO2);

    display.drawLine(0, 85, 250, 85, BLACK);
  }
}
// ****************************************************
void scd41()
{
  // Read Measurement
  uint16_t error = scd4x.readMeasurement(co2, temperature, humidity);
  if (error)
  {
    char errorMessage[256];
    Serial.print("Error trying to execute readMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }
  else
  {
    Serial.print("CO2 concentration: ");
    Serial.print(co2);
    Serial.println(" ppm");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" degrees C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  }
}
// ****************************************************
// use millis to read mq7 every 5 seconds
unsigned long previousMillis = 0;
const long interval = 10000;

int counter = 0;

void loop()
{

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    // ****************************************************
    // Tell BME680 to begin measurement.
    unsigned long endTime = bme.beginReading();
    if (endTime == 0)
    {
      Serial.println(F("Failed to begin reading :("));
      return;
    }
    Serial.print(F("Reading started at "));
    Serial.print(millis());
    Serial.print(F(" and will finish at "));
    Serial.println(endTime);

    Serial.println(F("You can do other work during BME680 measurement."));
    delay(50); // This represents parallel work.
    // There's no need to delay() until millis() >= endTime: bme.endReading()
    // takes care of that. It's okay for parallel work to take longer than
    // BME680's measurement time.

    // Obtain measurement results from BME680. Note that this operation isn't
    // instantaneous even if milli() >= endTime due to I2C/SPI latency.
    if (!bme.endReading())
    {
      Serial.println(F("Failed to complete reading :("));
      return;
    }
    Serial.print(F("Reading completed at "));
    Serial.println(millis());

    Serial.print(F("Temperature = "));
    Serial.print(bme.temperature);
    Serial.println(F(" *C"));

    Serial.print(F("Pressure = "));
    Serial.print(bme.pressure / 100.0);
    Serial.println(F(" hPa"));

    Serial.print(F("Humidity = "));
    Serial.print(bme.humidity);
    Serial.println(F(" %"));

    Serial.print(F("Gas = "));
    Serial.print(bme.gas_resistance / 1000.0);
    Serial.println(F(" KOhms"));

    Serial.print(F("Approx. Altitude = "));
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(F(" m"));

    Serial.println();
    // delay(2000);
    // ****************************************************
    // If you have a temperature / humidity sensor, you can set the absolute humidity to enable the humditiy compensation for the air quality signals
    // float temperature = 22.1; // [°C]
    // float humidity = 45.2; // [%RH]
    // sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));

    if (!sgp.IAQmeasure())
    {
      Serial.println("Measurement failed");
      return;
    }
    Serial.print("TVOC ");
    Serial.print(sgp.TVOC);
    Serial.print(" ppb\t");
    Serial.print("eCO2 ");
    Serial.print(sgp.eCO2);
    Serial.println(" ppm");

    if (!sgp.IAQmeasureRaw())
    {
      Serial.println("Raw Measurement failed");
      return;
    }
    Serial.print("Raw H2 ");
    Serial.print(sgp.rawH2);
    Serial.print(" \t");
    Serial.print("Raw Ethanol ");
    Serial.print(sgp.rawEthanol);
    Serial.println("");

    delay(1000);

    counter++;
    if (counter == 30)
    {
      counter = 0;

      uint16_t TVOC_base, eCO2_base;
      if (!sgp.getIAQBaseline(&eCO2_base, &TVOC_base))
      {
        Serial.println("Failed to get baseline readings");
        return;
      }
      Serial.print("****Baseline values: eCO2: 0x");
      Serial.print(eCO2_base, HEX);
      Serial.print(" & TVOC: 0x");
      Serial.println(TVOC_base, HEX);
    }
    // ****************************************************

    scd41();
    delay(100);
    mq7();
    delay(100);
    Serial.print(F("MQ-7: "));
    Serial.println(MQ7sensorValue);

    // serial print start and milis
    Serial.print("Start: ");
    Serial.println(millis());
    updateDisplay();
    // show time taken to update display
    Serial.print("End: ");
    Serial.println(millis());
    // calculate time taken to update display
    Serial.print("Time taken: ");
    Serial.println(millis() - previousMillis);
  }
}