#include <analogWrite.h>
#include "BluetoothSerial.h"
#include <Wire.h>
#include <stdint.h>

//Motores
//Motor A
int IN1 = 27; //Motor 1 Pin 1
int IN2 = 26; //Motor 1 Pin 2
//Motor B
int IN3 = 13; //Motor 2 Pin 1
int IN4 = 25; //Motor 2 Pin 2

//Control de velocidad mediante PWM
int ENA = 14; //Activa
int ENB = 18;

//Bluetooth
int lecblue;
BluetoothSerial ESP_BT;

const uint8_t SONIC_DISC_I2C_ADDRESS = 0x09;
const uint8_t NUM_OF_SENSORS = 8; // No. of ultrasonic sensors on SonicDisc
// The packet contains NUM_OF_MEASUREMENTS measurements and an error code
const uint8_t I2C_PACKET_SIZE = NUM_OF_SENSORS + 1;
// The number of measurements from each sensor to filter
const uint8_t MEASUREMENTS_TO_FILTER = 5;
const uint8_t INT_PIN = 4;
// The max valid variance in a set of MEASUREMENTS_TO_FILTER measurements
const unsigned int VARIANCE_THRESHOLD = 3;

// Sonic Disc's operational states
enum State
{
  STANDBY,  // MCU and sensors are on but no measurements are being made
  MEASURING // Sonic Disc is conducting measurements using the sensors
};

// Values to be received via I2C from master
enum I2C_RECEIPT_CODE
{
  STATE_TO_STANDBY = 0x0A,
  STATE_TO_MEASURING = 0x0B
};

// Error codes to be transmitted via I2c to the master
enum I2C_ERROR_CODE
{
  NO_ERROR,
  IN_STANDBY,
  INCOMPLETE_MEASUREMENT
};

enum DETECTAMIENTOS
{
  AVANZAR,
  DETECCIONFRONTAL,
  DETECCIONADELANTECOMPLETA, // Detecta objeto al frente
  DETECCIONADER,             // Detecta objeto en la parte frontal derecha
  DETECCIONADIZQ,            // Detecta objeto en la parte frontal derecha
  DETECCIONDERECHA,          // Detecta objeto lado derecho
  DETECCIONIZQUIERDA,        // Detecta objeto lado izquierdo
  DETECCIONATRASDER,         // Detecta objeto en la parte trasera derecha
  DETECCIONATRAS,            // Detecta objeto en la parte trasera
};

DETECTAMIENTOS DETECTAMIENTO = AVANZAR; // Holds the current parking sta
// Flag to indicate the SonicDisc is ready to send a new set of data
volatile bool newData = false;

uint8_t filterIndex = 0;
uint8_t filterBuffer[MEASUREMENTS_TO_FILTER][NUM_OF_SENSORS] = {0};
uint8_t filteredMeasurements[NUM_OF_SENSORS] = {0};
bool newFilteredMeasurements = false;

/**
   Requests an I2C packet from the SonicDisc
   @param  i2cInput         The array that will hold the incoming packet
   @param  transmissionSize The size/length of the incoming packet
   @return                  Error code contained inside the incoming packet
*/
I2C_ERROR_CODE requestPacket(uint8_t i2cInput[], const uint8_t transmissionSize = I2C_PACKET_SIZE)
{
  Wire.requestFrom(SONIC_DISC_I2C_ADDRESS, transmissionSize);
  uint8_t packetIndex = 0;
  while (Wire.available() && packetIndex < transmissionSize)
  {
    i2cInput[packetIndex++] = Wire.read();
  }
  return static_cast<I2C_ERROR_CODE>(i2cInput[0]); // Return the packet's error code
}

/**
   Sends the supplied byte to the SonicDisc
   @param byteToSend The byte to be sent
*/
void sendData(uint8_t byteToSend)
{
  Wire.beginTransmission(SONIC_DISC_I2C_ADDRESS);
  Wire.write(byteToSend);
  Wire.endTransmission(SONIC_DISC_I2C_ADDRESS);
}

/**
   ISR that raises a flag whenever SonicDisc is ready to transmit new data.
*/
void newSonicDiscData()
{
  newData = true;
}

/*
   Adds the specified i2c packet in the buffer to be sorted later.
*/
void addInputToFilterBuffer(uint8_t i2cInput[], const uint8_t bufferIndex)
{
  // Copy the whole packet (except error code) in the specified row of the buffer
  for (int i = 0, j = 1; i < NUM_OF_SENSORS; i++, j++)
  {
    filterBuffer[bufferIndex][i] = i2cInput[j];
  }
}

/*
    Sorts the measurements of each sensor for every cycle of measurements.
*/
void sortMeasurements()
{
  // For each sensor
  for (int s = 0; s < NUM_OF_SENSORS; s++)
  {
    // Use bubble sort to sort all measurements throughout the cycle
    for (int i = 0; i < MEASUREMENTS_TO_FILTER - 1; i++)
    {
      for (int j = 0; j < MEASUREMENTS_TO_FILTER - i - 1; j++)
      {
        if (filterBuffer[j][s] > filterBuffer[j + 1][s])
        {
          uint8_t toSwap = filterBuffer[j][s];
          filterBuffer[j][s] = filterBuffer[j + 1][s];
          filterBuffer[j + 1][s] = toSwap;
        }
      }
    }
  }
}

void filterMeasurements()
{
  // Go through all the measurements taken for each sensor
  for (int i = 0; i < NUM_OF_SENSORS; i++)
  {
    // Calculate the variance across the different measurements
    // by subtracting the first and the last element
    // of the *sorted* measurement cycle.
    int variance = filterBuffer[0][i] - filterBuffer[MEASUREMENTS_TO_FILTER - 1][i];
    if (abs(variance) > VARIANCE_THRESHOLD)
    {
      filteredMeasurements[i] = 0;
    }
    else
    {
      filteredMeasurements[i] = filterBuffer[MEASUREMENTS_TO_FILTER / 2][i];
    }
  }
}

void setup()
{
  analogReadResolution(12);
  //Configuración de los pines como salida
  //Motor A
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  //Motor B
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  //ENA y ENB
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  //Configuración PWM
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  Wire.begin();
  ESP_BT.begin("ESP32");
  Serial.begin(115200);
  attachInterrupt(digitalPinToInterrupt(INT_PIN), newSonicDiscData, RISING);
  Serial.println("Solicitando paquete de SonicDisc");
  uint8_t dummyInput[I2C_PACKET_SIZE] = {0}; // A throw-away array
  // Do not proceed unless the SonicDisc is in "MEASURING" state
  while (requestPacket(dummyInput, I2C_PACKET_SIZE) == IN_STANDBY)
  {
    Serial.println("Poniendo en estado MEASURING");
    sendData(STATE_TO_MEASURING);
  }
  Serial.println("Comunicacion establecida, SonicDisc midiendo distancias");
}

void loop()
{
  if (ESP_BT.available())
  {
    lecblue = ESP_BT.read(); //Leer transmisión
    Serial.print("Dato recibido:");
    Serial.println(lecblue);
  }
  //ASCII
  // 65 - A --- 68 - D --- 73 - I --- 82 - R --- 83 - S -- 66 - B
  if (lecblue == 65)
  {
    Adelante();
  }
  if (lecblue == 68)
  {
    Derecha();
  }
  if (lecblue == 73)
  {
    Izquierda();
  }
  if (lecblue == 82)
  {
    Reversa();
  }
  if (lecblue == 83)
  {
    Stop();
  }
  if (lecblue == 66)
  {
    Adelante();
    delay(1000);
    Stop();
    delay(1000);
    Reversa();
    delay(1000);
    Stop();
    delay(1000);
    Izquierda();
    delay(1000);
    Stop();
    delay(1000);
    Derecha();
    delay(1000);
    Stop();
    delay(1000);
  }
  if (lecblue = 67)
  {
    if (newData)
    {
      newData = false;                               // Indicate that we have read the latest data
      uint8_t sonicDiscInput[I2C_PACKET_SIZE] = {0}; // Get the I2C packet
      I2C_ERROR_CODE ret = requestPacket(sonicDiscInput, I2C_PACKET_SIZE);
      // Now sonicDiscInput contains the latest measurements from the sensors.
      // However we need to make sure that the data is also of good quality
      // Process the packet only if it is a valid one
      if (ret == NO_ERROR)
      {
        addInputToFilterBuffer(sonicDiscInput, filterIndex);
        // When we have filled up the filter buffer, time to filter the measurements
        if (filterIndex + 1 == MEASUREMENTS_TO_FILTER)
        {
          // For each measurement
          sortMeasurements();
          filterMeasurements();
          // Indicate that the measurements are filtered
          newFilteredMeasurements = true;
        }
        // Move along the index
        filterIndex = (filterIndex + 1) % MEASUREMENTS_TO_FILTER;
      }
    }
    if (newFilteredMeasurements)
    {
      newFilteredMeasurements = false;
      // Based on the current orientation of the SonicDisc
      // fetch the measurements we are interested in.
      uint8_t front = filteredMeasurements[6];
      uint8_t frontRight = filteredMeasurements[7];
      uint8_t frontLeft = filteredMeasurements[5];
      uint8_t left = filteredMeasurements[4];
      uint8_t right = filteredMeasurements[0];
      uint8_t backLeft = filteredMeasurements[3];
      uint8_t back = filteredMeasurements[2];

      switch (DETECTAMIENTO){
        static const uint8_t valdetec = 12; // In cm
        static const uint8_t mindetec = 10; // In cm
        // static const uint8_t motorSpeed = 60; // Percentage power to motors

      case AVANZAR:
      {
        if (front && frontRight && frontLeft > valdetec)
        {
          Adelante();
        }
      }
      break;
      case DETECCIONFRONTAL:
      {
        if (front <= valdetec)
        {
          Derecha();
        }
      }
      case DETECCIONADELANTECOMPLETA:
      {
        if (front <= valdetec && frontRight <= mindetec && frontLeft <= mindetec)
        {
          Reversa();
          delay(10000);
        }
      }
      break;
      case DETECCIONADER:
      {
        if (frontRight <= valdetec)
        {
          Izquierda();
          delay(500);
        }
      }
      break;
      case DETECCIONADIZQ:
      {
        if (frontLeft <= valdetec)
        {
          Derecha();
          delay(500);
        }
      }
      break;
      case DETECCIONDERECHA:
      {
        if (right <= mindetec)
        {
          Izquierda();
          delay(100);
        }
      }
      break;
      case DETECCIONIZQUIERDA:
      {
        if (left <= mindetec)
        {
          Derecha();
          delay(100);
        }
      }
      break;
      case DETECCIONATRASDER:
      {
        if (backLeft <= valdetec)
        {
          Stop();
          delay(50);
          Derecha();
          delay(100);
          Adelante();
        }
      }
      break;
      case DETECCIONATRAS:
      {
        if (back <= valdetec)
        {
          Stop();
        }
        break;
      }
      default:
        break;
      }
    }
  }
}

void Adelante()
{
  Serial.println("↑ ↑ ↑ ↑");
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 4095);
  analogWrite(ENB, 4095);
}

void Reversa()
{
  Serial.println("↓ ↓ ↓ ↓");
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, 1365);
  analogWrite(ENB, 1365);
}

void Izquierda()
{
  Serial.println("← ← ← ←");
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 1365);
  analogWrite(ENB, 0);
}

void Derecha()
{
  Serial.println("→ → → →");
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, 0);
  analogWrite(ENB, 1365);
}

void Stop()
{
  Serial.println("Motor en espera");
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
