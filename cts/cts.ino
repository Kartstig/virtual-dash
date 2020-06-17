#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal_I2C.h>
#define LED_PIN 6
#define NUMPIXELS 3

#define PIN_TEMPERATURE_1 A0
#define PIN_PRESSURE A1
#define VCC 5.0
#define TEMP1_LOADR 169
#define LCD_ADDR 0x27

#define P1 0  // Oil Pressure
#define CT1 1 // Oil Temperature
#define CT2 2 // Coolant Temperature

const int loadR = 100;

// Init LED
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Init LCD
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

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
  double T;
  double R;
  color Color;
} temperatureVector;

typedef struct
{
  double X;
  double Y;
  color Color;
} colorVector;

const color Blank = {0, 0, 0};
const color Red = {255, 0, 0};
const color Yellow = {128, 128, 0};
const color Blue = {0, 0, 32};
const color Green = {0, 24, 0};

#define TEMPLENGTH 18
const temperatureVector TemperatureTable[TEMPLENGTH] = {
    {70, 600.0, Blue},
    {85, 450.0, Blue},
    {129, 179.7, Blue},
    {135, 174, Blue},
    {141, 150, Blue},
    {144, 127.5, Blue},
    {155, 98.7, Blue},
    {158, 84.3, Blue},
    {160, 69.2, Green},
    {165, 66, Green},
    {170, 64.7, Green},
    {180, 63.5, Green},
    {185, 63, Green},
    {190, 62.8, Green},
    {200, 64.0, Green},
    {212, 56, Yellow},
    {230, 52, Red},
    {240, 50.5, Red}};

#define PRESSURE_TABLE_LENGTH 6
const valueColor PressureTable[PRESSURE_TABLE_LENGTH]{
    {-5, Red},
    {5, Red},
    {18, Red},
    {20, Yellow},
    {27, Green},
    {100, Green}};

void setup()
{
  lcd.init();
  lcd.backlight();
  pixels.begin();
  Serial.begin(115200);
  pinMode(PIN_PRESSURE, INPUT);
  pinMode(PIN_TEMPERATURE_1, INPUT);
  pinMode(LED_PIN, OUTPUT);
}

valueColor lookupTemperature(temperatureVector *c, float R)
{
  Serial.print("Predicted resistance: ");
  Serial.println(R);
  for (int i = TEMPLENGTH - 1; i >= 0; i--)
  {
    // Out of bounds: High Temp / Low R
    if (i == TEMPLENGTH - 1 && R < c[i].R)
      return {999.9, Red};
    // Out of bounds: Low Temp / High R
    if (i == 0 && R > c[i].R)
      return {-999.0, Blue};
    if (c[i].R == R)
      return {c[i].T, c[i].Color};
    if (c[i].R <= R && R <= c[i - 1].R)
    {
      const float value = interpolate(c[i - 1].R, c[i].R, c[i - 1].T, c[i].T, R);
      const color blend = {
          (c[i].Color.R + c[i - 1].Color.R) / 2,
          (c[i].Color.G + c[i - 1].Color.G) / 2,
          (c[i].Color.B + c[i - 1].Color.B) / 2};
      return {value, blend};
    }
  }
}

valueColor lookupPressure(valueColor *pt, float P)
{
  for (int i = 0; i < PRESSURE_TABLE_LENGTH - 1; i++)
  {
    if (i == 0 && P < pt[i].Value)
      return {P, Red};
    if (i == PRESSURE_TABLE_LENGTH - 1 && P >= pt[i].Value)
      return {P, Blue};
    if (i == PRESSURE_TABLE_LENGTH - 1)
      return {P, pt[i].Color};
    if (pt[i].Value == P)
      return {P, pt[i].Color};
    if (pt[i].Value <= P && P <= pt[i + 1].Value)
    {
      const color blend = {
          (pt[i].Color.R + pt[i + 1].Color.R) / 2,
          (pt[i].Color.G + pt[i + 1].Color.G) / 2,
          (pt[i].Color.B + pt[i + 1].Color.B) / 2};
      return {P, blend};
    }
  }
}

float interpolate(float xa, float xb, float ya, float yb, float x)
{
  return ya + (yb - ya) * (x - xa) / (xb - xa);
}

volatile color colors[3];

valueColor getPressure()
{
  const int presRawADC = analogRead(PIN_PRESSURE);
  if (presRawADC < 102)
    return {-999.9, Red};
  const float P = (float)(25 * (presRawADC - 102) * VCC / 1024);
  return lookupPressure(PressureTable, P);
}

valueColor getTemperature()
{
  const int tempRawADC = analogRead(PIN_TEMPERATURE_1);
  const float Vt = (float)tempRawADC * VCC / 1024;
  const float R = (float)(Vt * TEMP1_LOADR) / (VCC - Vt);
  return lookupTemperature(TemperatureTable, R);
}

void loop()
{
  const valueColor ct2 = getTemperature();
  lcd.setCursor(0, 0);
  lcd.write("Temperature: ");
  lcd.write(ct2.Value);
  Serial.print("Temperature: ");
  Serial.println(ct2.Value);

  const valueColor P = getPressure();
  lcd.setCursor(1, 0);
  lcd.write("Pressure: ");
  lcd.write(P.Value);
  Serial.print("Pressure: ");
  Serial.println(P.Value);

  updateLEDs(P.Color, Blank, ct2.Color);
  writeLEDs();
  delay(100);
}

void updateLEDs(const color colorP1, const color colorT1, const color colorT2)
{
  colors[CT1] = colorT1;
  colors[CT2] = colorT2;
  colors[P1] = colorP1;
}

void writeLEDs()
{
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(colors[i].R, colors[i].G, colors[i].B));
    pixels.show();
    delay(5);
  }
}
