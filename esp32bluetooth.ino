#include "BluetoothSerial.h"

//Motor
//Motor A
int IN1 = 27;//Motor 1 Pin 1
int IN2 = 26; //Motor 1 Pin 2
//Motor B
int IN3 = 13; //Motor 2 Pin 1
int IN4 = 25; //Motor 2 Pin 2

//Control de velocidad mediante PWM
//int ENAB = 14; //Activa los dos motores
// Propiedades
// const int freq = 30000;
// const int pwmChannel = 0;
// const int resolution = 8;
// int dutyCycle = 100;

//Bluetooth
int lecblue;
BluetoothSerial ESP_BT;

void setup() {
  //Configuración de los pines como salida
  //Motor A
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  //Motor B
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  //ENA y ENB
  //pinMode(ENAB, OUTPUT);
  //Configuración PWM
  //Configuración de LEDC para usar con PWM
  //ledcSetup(pwmChannel, freq, resolution);
  //Control motores
  //ledcAttachPin(ENAB, pwmChannel);
  ESP_BT.begin("ESP32");
  Serial.begin(115200);
}

void loop(){
  if (ESP_BT.available()){
    lecblue = ESP_BT.read(); //Leer transmisión
    Serial.print("Dato recibido:");
    Serial.println(lecblue);
  }
  
  //ASCII 
  // 65 - A --- 68 - D --- 73 - I --- 82 - R --- 83 - S -- 66 - B
if (lecblue == 65){
    Adelante();
    }
  if (lecblue == 68){
    Derecha();
    }
  if (lecblue == 73){
    Izquierda();
    }
  if (lecblue == 82){
    Reversa();
    }
  if (lecblue == 83){
    Stop();
    }
   if(lecblue == 66){
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
    
    void Adelante(){
  Serial.println("↑ ↑ ↑ ↑");
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW); 
}

void Reversa(){
  Serial.println("↓ ↓ ↓ ↓");
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void Izquierda(){
  Serial.println("← ← ← ←");
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void Derecha(){
  Serial.println("→ → → →");
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void Stop(){
  Serial.println("Motor en espera");
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW); 
}
