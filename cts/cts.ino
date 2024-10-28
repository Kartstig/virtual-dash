#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal_I2C.h>
#define LED_PIN 6
#define NUMPIXELS 3

// Pins
#define PIN_PRESSURE A1
#define PIN_TEMPERATURE_1 A0
#define PIN_TEMPERATURE_2 A2
#define PIN_FUEL_1 A3

#define VCC 5.0

// Engine Coolant Temperature Sensor (2 wire for ECU)
#define ECT_STEINHART_A 0.0012695897225755855
#define ECT_STEINHART_B 0.0002648767281824261
#define ECT_STEINHART_C 1.4179393133443658e-07

// Coolant Temperature Sensor (1 wire for gauge)
#define CTS_STEINHART_A 0.0012695897225755855
#define CTS_STEINHART_B 0.0002648767281824261
#define CTS_STEINHART_C 1.4179393133443658e-07

#define OT1_STEINHART_A 0.0011219261517215182
#define OT1_STEINHART_B 0.00023520046646272318
#define OT1_STEINHART_C 8.460569525266567e-08

#define TEMP1_LOADR 169
#define TEMP2_LOADR 100
#define LCD_ADDR 0x27

// LED Positions
#define P1 0  // Oil Pressure
#define OT1 1 // Oil Temperature
#define CT1 2 // Coolant Temperature

// Init LED
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Init LCD
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

// Struct for RGB color
typedef struct
{
  int R;
  int G;
  int B;
} color;

typedef struct
{
  float Value;
  color Color;
} valueColor;

typedef struct
{
  double X;
  double Y;
  color Color;
} colorVector;

// Predefined colors
const color Blank = {0, 0, 0};
const color Red = {255, 0, 0};
const color Yellow = {128, 128, 0};
const color ColdBlue = {0, 10, 40};
const color Blue = {0, 0, 32};
const color Green = {0, 24, 0};
const color Pink = {252, 15, 192};
const color Purple = {126, 87, 194};
const color Orange = {255, 165, 0};

// Water Temperature lookup table
const valueColor WaterTempTable[] = {
    {10, ColdBlue},
    {35, Blue},
    {80, Green},
    {93, Green},
    {100, Yellow},
    {110, Red},
    {120, Red}};

// Oil Temperature lookup table
const valueColor OilTempTable[] = {
    {10, ColdBlue},
    {40, Blue},
    {60, Green}, // Operating temp by OEM spec
    {104, Green},
    {110, Yellow},
    {129, Red},
    {150, Red}};

// Pressure lookup table
const valueColor PressureTable[]{
    {-5, Red},
    {5, Red},
    {20, Yellow},
    {27, Green},
    {100, Green}};

// Fuel level lookup table
const valueColor FuelTable[]{
    {5, Green},   // Full
    {11, Green},  // 3/4
    {16, Green},  // 5/8
    {24, Green},  // 1/2
    {35, Yellow}, // 1/4
    {74, Red}};   // Empty

// Table lengths
int _pressure_table_len;
int _water_temp_table_len;
int _oil_temp_table_len;
int _fuel_Table_len;

/**
 * Setup function initializes LCD, LEDs, serial communication,
 * and calculates the length of each lookup table.
 */
void setup()
{
  lcd.init();
  lcd.backlight();
  pixels.begin();
  Serial.begin(115200);
  pinMode(PIN_PRESSURE, INPUT);
  pinMode(PIN_TEMPERATURE_1, INPUT);
  pinMode(PIN_TEMPERATURE_2, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // Calculate lookup table lengths
  _pressure_table_len = sizeof(PressureTable) / sizeof(valueColor);
  _water_temp_table_len = sizeof(WaterTempTable) / sizeof(valueColor);
  _oil_temp_table_len = sizeof(OilTempTable) / sizeof(valueColor);
  _fuel_Table_len = sizeof(FuelTable) / sizeof(valueColor);
}

/**
 * Looks up a value in a given value-color table and returns an
 * interpolated color based on the input value.
 *
 * @param pt Pointer to the table of valueColor.
 * @param tableLen Length of the table.
 * @param P Value to look up.
 * @return valueColor Interpolated valueColor struct.
 */
valueColor lookupValue(valueColor *pt, const int tableLen, const float P)
{
  // Check for out-of-bounds low
  if (P < pt[0].Value)
    return {P, Purple};

  // Check for out-of-bounds high
  if (P >= pt[tableLen - 1].Value)
    return {P, Orange};

  for (int i = 0; i < tableLen - 1; i++)
  {
    if (pt[i].Value == P)
      return {P, pt[i].Color};

    if (pt[i].Value <= P && P <= pt[i + 1].Value)
    {
      // Calculate the interpolated color
      const color blend = {
          (pt[i].Color.R + pt[i + 1].Color.R) / 2,
          (pt[i].Color.G + pt[i + 1].Color.G) / 2,
          (pt[i].Color.B + pt[i + 1].Color.B) / 2};

      return {P, blend}; // Return the interpolated value and color
    }
  }
  return {P, Pink}; // Fallback color for unhandled cases
}

volatile color colors[3];

/**
 * Reads and processes oil pressure sensor data, returning a corresponding color.
 *
 * @return valueColor Interpolated valueColor for pressure
 */
valueColor getPressure()
{
  const int presRawADC = analogRead(PIN_PRESSURE);
  if (presRawADC < 102)
    return {-999.9, Purple};
  const float P = (float)(25 * (presRawADC - 102) * VCC / 1024);
  return lookupValue(PressureTable, _pressure_table_len, P);
}

/**
 * Calculates the resistance of the temperature sensor.
 *
 * @param pin Analog pin number
 * @param loadR Load resistor value
 * @return float Calculated resistance in ohms
 */
float getResistance(const int pin, const int loadR)
{
  const int tempRawADC = analogRead(pin);
  const float Vt = (float)tempRawADC * VCC / 1024;
  const float R = (float)(Vt * loadR) / (VCC - Vt);
  return R;
}

/**
 * Main loop function that reads sensor data, updates LCD, and controls LEDs.
 */
void loop()
{
  // T1: Coolant Temperature
  const float t1R = getResistance(PIN_TEMPERATURE_1, TEMP1_LOADR);
  const float t1T = steinhartFromR(CTS_STEINHART_A, CTS_STEINHART_B, CTS_STEINHART_C, t1R);
  const valueColor t1Lookup = lookupValue(WaterTempTable, _water_temp_table_len, t1T);
  lcd.setCursor(0, 0);
  lcd.print("T1: ");
  lcd.print(t1Lookup.Value);
  Serial.print("T1:");
  Serial.print(t1Lookup.Value);
  Serial.print(",");

  // T2: Oil Temperature
  const float t2R = getResistance(PIN_TEMPERATURE_2, TEMP2_LOADR);
  const float t2T = steinhartFromR(OT1_STEINHART_A, OT1_STEINHART_B, OT1_STEINHART_C, t2R);
  const valueColor t2Lookup = lookupValue(OilTempTable, _oil_temp_table_len, t2T);
  lcd.setCursor(0, 10);
  lcd.print("T2: ");
  lcd.print(t2Lookup.Value);
  Serial.print("T2:");
  Serial.print(t2Lookup.Value);
  Serial.print(",");

  // P1: Oil Pressure
  const valueColor P = getPressure();
  lcd.setCursor(1, 0);
  lcd.print("Pressure: ");
  lcd.print(P.Value);
  Serial.print("P1:");
  Serial.print(P.Value);

  // F1: Fuel
  // TODO: Fuel Display

  Serial.print("\n");

  updateLEDs(P.Color, t2Lookup.Color, t1Lookup.Color);
  writeLEDs();
  delay(10);
}

/**
 * Calculates temperature using Steinhart-Hart equation.
 *
 * @param A Steinhart-Hart coefficient A
 * @param B Steinhart-Hart coefficient B
 * @param C Steinhart-Hart coefficient C
 * @param R Resistance of the sensor
 * @return float Calculated temperature in Celsius
 */
float steinhartFromR(float A, float B, float C, float R)
{
  float Temp = log(R);
  Temp = (1 / (A + (B * Temp) + (C * (Temp * Temp * Temp)))) - 273.15;
  // Temp = (Temp * 9 / 5) + 32.0; // Convert to F
  return Temp;
}

void updateLEDs(const color colorP1, const color colorT1, const color colorT2)
{
  colors[OT1] = colorT1;
  colors[CT1] = colorT2;
  colors[P1] = colorP1;
}

/**
 * Writes the current colors to the NeoPixel LEDs.
 *
 * This function iterates over all the LEDs and sets their color based on the
 * current values stored in the `colors` array. The `pixels.show()` function
 * is called once after setting all the colors, which updates all the LEDs simultaneously.
 */
void writeLEDs()
{
  // Set the color for each pixel
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(colors[i].R, colors[i].G, colors[i].B));
  }

  // Update the LEDs once after setting all the colors
  pixels.show();
}
