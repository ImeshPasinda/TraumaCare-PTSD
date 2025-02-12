#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TinyGPS++.h>
#include <Adafruit_SSD1306.h>
#define OLED_Address 0x3C
Adafruit_SSD1306 display(128, 64);

#define DHTPIN 13
#define RXD2 16
#define TXD2 17
#define vibrate 15
#define sos_Button 23
#define oneWireBus 27
#define DHTTYPE DHT11

HardwareSerial Sender(1);

#define REPORTING_PERIOD_MS     1000
MAX30100 sensor;
PulseOximeter pox;
bool bb;
uint16_t ir, red;
uint16_t avg_ir = 0, avg_red = 0;
uint32_t tsLastReport = 0;
int lastOccurrence = LOW;
int beatAvg, spo2;

int t, h, body_t;
String prediction;
String  sos_status = "No emergency";
String  Latitude = "6.42";
String  Longitude = "80.82";
struct tm timeinfo;

int  resolution = 12;
unsigned long lastTempRequest = 0;
int  delayInMillis = 0;
int  idle = 0;

DHT dht(DHTPIN, DHTTYPE);
TinyGPSPlus gps;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;

const unsigned char myBitmap [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x86, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc6, 0x3c, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x03, 0xc6, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xce, 0xe0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0xcf, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x03, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xc0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x07, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xf0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x0e, 0x7f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0xf9, 0xf8, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x19, 0xf0, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0xf0, 0xf8, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x1f, 0xe0, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xe0, 0x04, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x1f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x80, 0x00, 0x40, 0x00,
  0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x40, 0x00,
  0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xc0, 0x00,
  0x00, 0x00, 0x07, 0xf0, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x10, 0x00, 0x60, 0x00,
  0x00, 0x00, 0x3f, 0xff, 0x78, 0x00, 0x60, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe, 0x00, 0x60, 0x00,
  0x00, 0x00, 0x7f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xe0, 0x00, 0x00,
  0x00, 0x00, 0x7f, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xfc, 0x00, 0x00,
  0x00, 0x00, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00,
  0x00, 0x00, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
  0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
  0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0x00, 0x00,
  0x00, 0x00, 0x7f, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xfe, 0x00, 0x00,
  0x00, 0x00, 0x3f, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xfc, 0x00, 0x00,
  0x00, 0x00, 0x1f, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xfc, 0x00, 0x00,
  0x00, 0x00, 0x0f, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xf0, 0x00, 0x00,
  0x00, 0x00, 0x03, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xe0, 0x00, 0x00,
  0x00, 0x00, 0x01, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xc0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x0f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xfc, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
void setup() {
  Serial.begin(9600);
  Sender.begin(9600, SERIAL_8N1, RXD2, TXD2);
  dht.begin();

  sensors.begin();
  sensors.getAddress(tempDeviceAddress, 0);
  sensors.setResolution(tempDeviceAddress, resolution);

  sensors.setWaitForConversion(false);
  sensors.requestTemperatures();
  delayInMillis = 750 / (1 << (12 - resolution));
  lastTempRequest = millis();

  Wire.setClock(400000UL);

  pinMode(sos_Button, INPUT_PULLUP);
  pinMode(vibrate, OUTPUT);
  digitalWrite(vibrate, LOW);

  if (!sensor.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
  }

  configureMax30100();

  takeSampleReadings();

  if (!pox.begin()) //Use default I2C port, 400kHz speed //Wire, I2C_SPEED_FAST
  {
    Serial.println("MAX30102 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);

  display.begin(SSD1306_SWITCHCAPVCC, OLED_Address);
  display.clearDisplay();

}

void loop() {


  if (digitalRead(sos_Button) == LOW) {
    sos_status = "emergency";
    digitalWrite(vibrate, HIGH);
    delay(1000);
    digitalWrite(vibrate, LOW);
  }

  dhtRead();
  bodytempRead();
  Display();
  max30102Read();
  delay(1000);

  //  while (Sender.available() > 0)
  //    if (gps.encode(Sender.read()))
  //      displayInfo();

}
void dhtRead() {

  h = dht.readHumidity();
  t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));
}
void bodytempRead() {

  if (millis() - lastTempRequest >= 4000) {
    sensors.requestTemperatures();
    body_t = sensors.getTempCByIndex(0);
    Serial.print(body_t);
    Serial.println("ºC");
    idle = 0;
    resolution++;
    if (resolution > 12) resolution = 9;
    sensors.setResolution(tempDeviceAddress, resolution);
    sensors.requestTemperatures();
    delayInMillis = 750 / (1 << (12 - resolution));
    lastTempRequest = millis();
  }
  idle++;
}
void displayInfo() {
  if (gps.location.isValid()) {
    Latitude = gps.location.lat();
    Longitude = gps.location.lng();

    Serial.print(F("Latitude: "));
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(F("Longitude: "));
    Serial.println(gps.location.lng(), 6);

  } else {
    Serial.print(F("INVALID"));
  }
  Serial.println();
}
void max30102Read() {

  sensor.update();
  pox.update();

  while (sensor.getRawValues(&ir, &red)) {
    if (ir > 10 * avg_ir && red > 10 * avg_red) {
      if (lastOccurrence == LOW) {
        Serial.println("Something is there!");
        bb = true;
        lastOccurrence = HIGH;
      }
    }
    else {
      lastOccurrence = LOW;
      bb = false;
    }
  }
  while (bb) {
    sensor.update();
    pox.update();

    while (sensor.getRawValues(&ir, &red)) {
      if (ir > 10 * avg_ir && red > 10 * avg_red) {
        if (lastOccurrence == LOW) {
          Serial.println("Something is there!");
          bb = true;
          lastOccurrence = HIGH;
        }
      }
      else {
        lastOccurrence = LOW;
        bb = false;
      }
    }
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
      beatAvg = pox.getHeartRate();
      spo2 = pox.getSpO2();
      Serial.print("Heart rate:");
      Serial.print(pox.getHeartRate());
      Serial.println("bpm");
      tsLastReport = millis();
    }
  }

}
void Display() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(25, 0);
  display.println("Health Band");
  display.setCursor(0, 20);
  display.print("Temperature : ");
  display.println(t);
  display.print("Humidity : ");
  display.println(h);
  display.print("Body Temerature : ");
  display.println(body_t);
  display.print("Heart Rate : ");
  display.print(beatAvg);
  display.println(" BPM");
  display.display();
}
void onBeatDetected() {
  Serial.println("Beat!");
  Serial.print("Heart rate:");
  Serial.print(pox.getHeartRate());
  Serial.println("bpm");
  display.clearDisplay();
  display.drawBitmap(0, 0, myBitmap, 64, 64, WHITE);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(75, 25);
  display.print(beatAvg);
  display.setCursor(65, 45);
  display.print(" BPM");
  display.display();
}
void configureMax30100() {
  sensor.setMode(MAX30100_MODE_SPO2_HR);
  sensor.setLedsCurrent(MAX30100_LED_CURR_50MA, MAX30100_LED_CURR_27_1MA);
  sensor.setLedsPulseWidth(MAX30100_SPC_PW_1600US_16BITS);
  sensor.setSamplingRate(MAX30100_SAMPRATE_100HZ);
  sensor.setHighresModeEnabled(true);
}

void takeSampleReadings() {
  delay(50);
  for (int i = 0; i <= 9; i++) {
    sensor.update();
    sensor.getRawValues(&ir, &red);
    avg_ir += ir;
    avg_red += red;
    delay(50);
  }
  avg_ir /= 10;
  avg_red /= 10;
}
