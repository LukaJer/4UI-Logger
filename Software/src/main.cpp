#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_INA228.h>
#include <SD.h>

#define ADS1115_ADDRESS 0x48
#define INA228_ADDRESS 0x40

#define SCK 14
#define MISO 2
#define MOSI 15
#define CS 13

Adafruit_ADS1115 ads;
Adafruit_INA228 ina228 = Adafruit_INA228();

void startRecording();
void stopRecording();
void bttnChg();

#define SERIALOUT
// #define SDCARD

const int chipSelect = 13;
const int buttonPin = 25;
const int LEDPin = 33;
const int Samplerate = 1; // in Hz

File dataFile;
int fileNumber = 0;
unsigned long startMillis;
volatile byte recording = 0;
byte oldRecording = 0;
byte SDCardOK = 0;
SPIClass spi = SPIClass(VSPI);

void setup()
{
  Serial.begin(115200);
  spi.begin(SCK, MISO, MOSI, CS);

  pinMode(buttonPin, INPUT_PULLDOWN);
  pinMode(LEDPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(buttonPin), bttnChg, FALLING);

#ifdef SDCARD
  if (!SD.begin(CS, spi, 80000000))
  {
    Serial.println("Card Mount Failed");
    SDCardOK = 1;
  }
#endif

  ina228.begin();
  ina228.setShunt(0.02, 8.0);

  if (Samplerate == 100)
  {
    ina228.setAveragingCount(INA228_COUNT_16);
    ina228.setVoltageConversionTime(INA228_TIME_50_us);
    ina228.setCurrentConversionTime(INA228_TIME_50_us);
  }
  else
  {
    ina228.setAveragingCount(INA228_COUNT_64);
    ina228.setVoltageConversionTime(INA228_TIME_150_us);
    ina228.setCurrentConversionTime(INA228_TIME_150_us);
  }

  ads.begin();
}

void bttnChg()
{
  recording = !recording;
}

void loop()
{
  if (oldRecording == recording)
  {
    if (recording && !(millis() % (100 / Samplerate)))
    {
      digitalWrite(LEDPin, HIGH);
      int16_t adc0, adc1, adc2, adc3;
      float volts0, volts1, volts2, volts3;
      float inaVoltage, inaCurrent, inaPower;

      adc0 = ads.readADC_SingleEnded(0);
      adc1 = ads.readADC_SingleEnded(1);
      adc2 = ads.readADC_SingleEnded(2);
      adc3 = ads.readADC_SingleEnded(3);
      volts0 = ads.computeVolts(adc0) * 10.0;
      volts1 = ads.computeVolts(adc1) * 10.0;
      volts2 = ads.computeVolts(adc2) * 10.0;
      volts3 = ads.computeVolts(adc3) * 10.0;

      inaVoltage = ina228.readBusVoltage();
      inaCurrent = ina228.readCurrent();
      inaPower = ina228.readPower();
#ifdef SDCARD
      if (dataFile)
      {
        // Write data to the file
        dataFile.print(millis() - startMillis); // Timestamp
        dataFile.print(",");
        dataFile.print(volts0); //
        dataFile.print(",");
        dataFile.print(volts1); //
        dataFile.print(",");
        dataFile.print(volts2); //
        dataFile.print(",");
        dataFile.print(volts3); //
        dataFile.print(",");

        dataFile.print(inaVoltage); //
        dataFile.print(",");
        dataFile.print(inaCurrent); //
        dataFile.print(",");
        dataFile.println(inaPower); //
      }
#endif

#ifdef SERIALOUT
      Serial.print(millis() - startMillis); // Timestamp
      Serial.print(",");
      Serial.print(volts0); //
      Serial.print(",");
      Serial.print(volts1); //
      Serial.print(",");
      Serial.print(volts2); //
      Serial.print(",");
      Serial.print(volts3); //
      Serial.print(",");

      Serial.print(inaVoltage); //
      Serial.print(",");
      Serial.print(inaCurrent); //
      Serial.print(",");
      Serial.println(inaPower); //
#endif
    }
    digitalWrite(LEDPin, LOW);
  }
  else if (recording) // Start Recording
  {
    oldRecording = 1;
    startMillis = millis();

#ifdef SERIALOUT
    Serial.println("Serial Output Started");
    Serial.print("Time"); // Timestamp
    Serial.print(",");
    Serial.print("ADC0"); // Timestamp
    Serial.print(",");
    Serial.print("ADC1"); // Timestamp
    Serial.print(",");
    Serial.print("ADC2"); // Timestamp
    Serial.print(",");
    Serial.print("ADC3"); // Timestamp
    Serial.print(",");

    Serial.print("Current"); // Timestamp
    Serial.print(",");
    Serial.print("Voltage"); // Timestamp
    Serial.print(",");
    Serial.println("Power"); // Timestamp
#endif                       //

#ifdef SDCARD
    if (SDCardOK)
    {
      char filename[15];
      sprintf(filename, "/data_%04d.csv", fileNumber++);
      dataFile = SD.open(filename, FILE_WRITE);

      if (dataFile)
      {
        Serial.print("Recording started - File: ");
        Serial.println(filename);
        dataFile.print("Time"); // Timestamp
        dataFile.print(",");
        dataFile.print("ADC0"); // Timestamp
        dataFile.print(",");
        dataFile.print("ADC1"); // Timestamp
        dataFile.print(",");
        dataFile.print("ADC2"); // Timestamp
        dataFile.print(",");
        dataFile.print("ADC3"); // Timestamp
        dataFile.print(",");

        dataFile.print("Current"); // Timestamp
        dataFile.print(",");
        dataFile.print("Voltage"); // Timestamp
        dataFile.print(",");
        dataFile.println("Power"); // Timestamp
      }
      else
      {
        Serial.println("Error opening file for recording");
      }
    }
#endif
  }
  else
  {
    oldRecording = 0;
#ifdef SDCARD
    dataFile.close();
#endif
    recording = false;
    Serial.println("Recording stopped");
  }
}
