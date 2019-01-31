#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

LiquidCrystal_PCF8574 lcd(0x27);  // set the LCD address to 0x27 for a 16 chars and 2 line display (0x3f)

// LCD
int show;

// DUST
int measurePin = A0;
int ledPower = 6;

int samplingTime = 280; //280
int deltaTime = 40;
int sleepTime = 9620;

float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

static unsigned long VoRawTotal = 0;
static int VoRawCount = 0;
// Set the typical output voltage in Volts when there is zero dust. 
static float Voc = 0.6;
// Use the typical sensitivity in units of V per 100ug/m3.
const float K = 0.5;

void setup()
{
  Serial.begin(9600);
  pinMode(ledPower,OUTPUT);
  lcd.setBacklight(255);
  lcd.begin(16, 2); //initialize the lcd
//  lcd.setCursor(0, 0);
//  lcd.print("Dust Density: ");
//  lcd.setCursor(0, 1);
//  lcd.print("-");
} // setup()

void loop()
{
  digitalWrite(ledPower,LOW); // power on the LED
  delayMicroseconds(samplingTime);

  // voMeasured = analogRead(measurePin); // read the dust value
  int VoRaw = analogRead(measurePin);
  
  // Wait for remainder of the 10ms cycle = 10000 - 280 - 100 microseconds.
//  delayMicroseconds(deltaTime);
  digitalWrite(ledPower,HIGH); // turn the LED off
  delayMicroseconds(sleepTime);

  // 0 - 5.0V mapped to 0 - 1023 integer values 
  // calcVoltage = voMeasured * (5.0 / 1024); 
  
  // linear eqaution taken from http://www.howmuchsnow.com/arduino/airquality/
  // Chris Nafis (c) 2012
  // dustDensity = (0.17 * calcVoltage - 0.1)*1000; 

  // Use this fomular
  // https://github.com/sharpsensoruser/sharp-sensor-demos/blob/master/sharp_gp2y1014au0f_demo/sharp_gp2y1014au0f_demo.ino
  
  
  Serial.print("Raw Signal Value (0-1023): ");
  Serial.print(VoRaw);
  Serial.println("mg/m3");

  float Vo = VoRaw;
// Use average value
//  VoRawTotal += VoRaw;
//  VoRawCount++;
//  if (VoRawCount >= 100) {
//    Vo = 1.0 * VoRawTotal / 100;
//    VoRawCount = 0;
//    VoRawTotal = 0;
//  } else {
//    return;
//  }

  // Compute the output voltage in Volts.
  Vo = Vo / 1024.0 * 5.0;
  Serial.print("Vo: ");
  Serial.print(Vo * 1000.0);
  Serial.println("mV");
  
  // Convert to Dust Density in units of ug/m3.
  float dV = Vo - Voc;
  if ( dV < 0 ) {
    dV = 0;
    Voc = Vo;
  }
  float dustDensity = dV / K * 100.0;
  Serial.print("Dust Density: ");
  Serial.print(dustDensity);
  Serial.println("ug/m3");

  String pmPrint = "PM2.5: " + String(calculateAqi(dustDensity));
  String dustPrint = "ug/m3: " + String(dustDensity, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(pmPrint);
  lcd.setCursor(0, 1);
  lcd.print(dustPrint);
  
  Serial.println(pmPrint);
  
//  Serial.print(" - Voltage: ");
//  Serial.print(calcVoltage);
  
//  Serial.print(" - Dust Density [ug/m3]: ");
//  Serial.println(dustDensity);
  
  delay(1000);  
} // loop()

int calculateAqi(float val) {
  float aqi = 0;
  float range[8] = {0,50,100,150,200,300,400,500};
  float scale[8] = {0, 12, 35.5, 55.5, 150.5, 250.5, 350.5, 500.5};
  for (int i = 0; i < 7; i++) {
    if (val >= scale[i] && val < scale[i+1]) {
      aqi = range[i] + (val - scale[i]) * (range[i+1]-range[i])/(scale[i+1]-scale[i]);
      aqi = round(aqi);
      return aqi;
    }
  }
  return -1;
}
