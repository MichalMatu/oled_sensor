
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SensirionI2CScd4x.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/Org_01.h>

// Constants
const int BUFFER_SIZE = 64;
const int DEBOUNCE_DELAY = 50;
const int MENU_OPTIONS = 4;
const int buttonPins[] = {14, 27, 26, 25};

// Global variables
Adafruit_SSD1306 display(128, 64, &Wire);
SensirionI2CScd4x scd4x;
int co2Array[BUFFER_SIZE];
float tempArray[BUFFER_SIZE];
float humArray[BUFFER_SIZE];
int co2Index = 0, tempIndex = 0, humIndex = 0;
float temperature = 0.0f, humidity = 0.0f;
int currentMenuOption = 0;
unsigned long previousMillis = 0;

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  scd4x.begin(Wire);

  for (int i = 0; i < MENU_OPTIONS; i++)
  {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setFont(&FreeMono9pt7b);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(50, 40);
  display.print("OK");
  display.display();

  Serial.println("Waiting for first measurement... (5 sec)");
}

void readButtons()
{
  int leftButtonState = digitalRead(buttonPins[0]);
  int rightButtonState = digitalRead(buttonPins[1]);

  if (leftButtonState == LOW)
  {
    currentMenuOption = (currentMenuOption + 1) % MENU_OPTIONS;
    delay(DEBOUNCE_DELAY);
  }

  if (rightButtonState == LOW)
  {
    currentMenuOption = (currentMenuOption - 1 + MENU_OPTIONS) % MENU_OPTIONS;
    delay(DEBOUNCE_DELAY);
  }
}

void readSensors()
{
  if (millis() - previousMillis >= 11000)
  {
    uint16_t CO2;
    float Temp, Hum;
    uint16_t error = scd4x.readMeasurement(CO2, Temp, Hum);
    delay(100);

    if (!error)
    {
      co2Index = (co2Index + 1) % BUFFER_SIZE;
      co2Array[co2Index] = CO2;
      tempIndex = (tempIndex + 1) % BUFFER_SIZE;
      tempArray[tempIndex] = Temp;
      humIndex = (humIndex + 1) % BUFFER_SIZE;
      humArray[humIndex] = Hum;
      temperature = Temp;
      humidity = Hum;
    }
    else
    {
      Serial.print("Sensor read error: ");
      Serial.println(error);
    }

    previousMillis = millis();
  }
}

void drawMenu()
{
  display.clearDisplay();
  switch (currentMenuOption)
  {
  case 0:
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
    if (co2Array[co2Index] > 1000)
    {
      // blink if CO2 is more than 1000
      if (millis() % 1000 > 500)
      {
        display.print(co2Array[co2Index]);
      }
    }
    else
    {
      display.print(co2Array[co2Index]);
    }
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
      display.drawPixel(i, 40, SSD1306_WHITE); // Plot CO2 values
    }
    // in bottom left corner dispaly current CO2
    display.setCursor(0, 60);
    // set font to be bigger
    display.setFont(&FreeMono9pt7b);
    display.print(co2Array[co2Index]);
    display.print("ppm");

    for (int i = 0; i < BUFFER_SIZE; i++)
    {
      display.drawPixel(i, map(co2Array[i], 400, 2000, display.height(), 0), SSD1306_WHITE); // Plot CO2 values
    }
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
    // in bottom left corner dispaly current temperature
    display.setCursor(0, 60);
    // set font to be bigger
    display.setFont(&FreeMono9pt7b);
    display.print(tempArray[tempIndex]);
    display.print("C");
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
    break;

  case 3:
    display.clearDisplay();
    // in top left corner draw 24m humidity graph
    display.setCursor(0, 10);
    display.setFont(&Org_01);
    display.print("24m humidity");
    // below draw points from left to right representinh 1 hour pass
    for (int i = 0; i < 128; i = i + 11)
    {
      display.drawPixel(i, 15, SSD1306_WHITE); // Plot CO2 values
    }
    // in bottom left corner dispaly current humidity
    display.setCursor(0, 60);
    // set font to be bigger
    display.setFont(&FreeMono9pt7b);
    display.print(humArray[humIndex]);
    display.print("%");

    // in top right  corner draw 2000
    display.setCursor(100, 10);
    // use tiny font
    display.setFont(&Org_01);
    display.print("100%");
    // in bottom right corner draw 400
    display.setCursor(100, 60);
    display.print("0%");
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
      display.drawPixel(i, map(humArray[i], 0, 100, display.height(), 0), SSD1306_WHITE); // Plot CO2 values
    }
    break;
  }
  display.display();
}

void loop()
{
  readButtons();
  readSensors();
  drawMenu();
}