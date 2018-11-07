#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <EEPROM.h>

#define SEALEVELPRESSURE_HPA (1019.25)

Adafruit_BMP280 bmp; // I2C
//float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
float altitude;
float initialAltitude;
float maxAltitude = 0;
uint8_t altiByte[501]; //altitude in a byte
unsigned long nextSampleTime; //time when next sample has to be taken
int ringBufferPosition = 0;
boolean apogee = false;
int apogeeCount = 0; //number of samples after apogee;
int EEaddr = -1;



//////////////////////////////////////// SETUP

void setup(void)
{
  Serial.begin(9600);
  if (!bmp.begin())/* Initialise the sensor */
  {
  Serial.print("Ooops, no BMP280 detected ... Check your wiring or I2C ADDR!");
    while (1);
  }
  pinMode(2, INPUT_PULLUP); //button
}

//////////////////////////////////////// LOOP

void loop(void)
{
  digitalWrite(13, HIGH);
  while (digitalRead(2)) { //wait for button to be pushed
    //if (EEaddr == -1) {
    //  Serial.write(255);
    //}
    //write EEPROM to Serial port while waiting for button
    if (Serial.available() > 0){
      if (EEaddr >= 0 && EEaddr <= 500) {
        //uint8_t valueToSend = EEPROM.read(EEaddr);
        //if (valueToSend == 255) {
        //  valueToSend = 254;
        //}
        //Serial.write(valueToSend);
        uint8_t valueToSend = EEPROM.read(EEaddr);
        Serial.print(valueToSend);
        Serial.println();
        delay(5);
      }
      EEaddr++;
      if (EEaddr > 500) {
        delay(20);
      }
      if (EEaddr == 1000) {
        EEaddr == -1;
      }
    }
  }
  
  Serial.println("BUTTON PUSHED");

  Serial.print("MEASURE INITIAL ALTITUDE:");
  initialAltitude = 0;
  for (int i = 0; i < 10; i++) {
    initialAltitude += bmp.readAltitude(SEALEVELPRESSURE_HPA);
  }
  initialAltitude = initialAltitude / 10;

  Serial.print("Initial altitude:");
  Serial.print(initialAltitude);
  Serial.print("m");
  Serial.println();

  // button is pushed: device is armed

  while (apogeeCount <= 300) {  //after apogee: write still 300 samples into ringbuffer
    if (!apogee) {
      blink1Hz(); //blink with 1Hz in order to indicate that device is armed but apogee has not been detected
    } else {
      blink10Hz(); //blink with 10Hz in order to indicate that device is armed and apogee has been detected
    }

    if (millis() >= nextSampleTime) {
      nextSampleTime = nextSampleTime + 50; // take altitude samples with 20Hz
      altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA) - initialAltitude;
      //Serial.print(altitude);
      //Serial.print("m");
      //Serial.println();

      //detect apogee
      if (altitude > maxAltitude) {
        maxAltitude = altitude;           //save highest altitude
      }
      if (maxAltitude > 2 && altitude < (maxAltitude - 3)) { //if height is lower than maximum height -3m and maxaltitude > 2
        apogee = true;
        Serial.print("Max altitude detected:");
        Serial.print(maxAltitude);
        Serial.println();
      }

      if (apogee) {
        apogeeCount++;  //keep track of how many values have been sampled since apogee
      }

      //convert altitude in a byte value
      altitude = altitude * 4;
      if (altitude < 0) {
        altitude = 0;
      }
      if (altitude > 254) {
        altitude = 254;
      }
      altiByte[ringBufferPosition] = int(altitude); //write altitude samples into ringbuffer
      ringBufferPosition++;
      if (ringBufferPosition == 500) {
        ringBufferPosition = 0;
      }
    }
  }

  //recording has ended - save values in EEPROM
  digitalWrite(13, HIGH);
  byte value = EEPROM.read(0);
  if (value >= 199) {
    value = 0;
  }
  value++;
  EEPROM.write(0, value); //write number of recording in address 0
  
  //int IVin = analogRead(A6); //read battery voltage
  //IVin = IVin / 4;
  //uint8_t Vwrite = IVin;
  //if (Vwrite == 255) {
  //  Vwrite = 254;
  //}
  //EEPROM.write(1, Vwrite); //write battery voltage in address 1

  for (int i = 1; i < 501; i++) { //write 500 samples in addresses 2...501
    ringBufferPosition++;
    if (ringBufferPosition == 500) {
      ringBufferPosition = 0;
    }
    delay(1);
    uint8_t writeVal = altiByte[ringBufferPosition];
    if (writeVal == 255) {
      writeVal = 254;
    }
    EEPROM.write(i, writeVal);
  }
  EEPROM.write(502, 0x00);

  while (1) {
    for (int i = 0; i < 10; i++) {
      digitalWrite(13, HIGH);
      delay(50);
      digitalWrite(13, LOW);
      delay(50);
    }
    delay(1000);
  }
}

////////////////////////////////////////FUNCTIONS

void blink1Hz() {
  unsigned long tm = millis();
  tm = tm / 500;
  digitalWrite(13, tm % 2);
}

void blink10Hz() {
  unsigned long tm = millis();
  tm = tm / 50;
  digitalWrite(13, tm % 2);
}


