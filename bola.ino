#include <Adafruit_TCS34725.h>
#include <Servo.h>

// --- Sensor de color (TCS34725) ---
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_600MS, TCS34725_GAIN_4X);

// --- Sensor ultrasónico HC-SR04 ---
#define TRIG 8
#define ECHO 9

// --- Motores (puente H L298N) ---
#define ENA 5
#define IN1 13
#define IN2 12
#define IN3 11
#define IN4 10
#define ENB 4

// --- Servo ---
Servo servoGarra;
int servoPin = A1;
int servoPos = 0; // posición actual del servo

// --- Variables de control ---
float distancia = 0;

// ------------------ SETUP ------------------
void setup() {
  Serial.begin(9600);

  // Motores
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Ultrasónico
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  // Servo
  servoGarra.attach(servoPin);
  servoGarra.write(servoPos);

  // Sensor de color
  if (tcs.begin()) {
    Serial.println("Sensor TCS34725 detectado.");
  } else {
    Serial.println("⚠️ No se detecta el sensor de color.");
    while (1);
  }

  detener();
}

// ------------------ LOOP ------------------
void loop() {
  // Leer distancia
  distancia = medirDistancia();

  // Si hay un obstáculo cerca
  if (distancia > 0 && distancia < 15) {
    detener();
    Serial.println("🛑 Obstáculo detectado. Esperando...");
    delay(2000);
    return; // vuelve al loop
  }

  // Leer color
  String color = detectarColor();
  Serial.print("Color detectado: ");
  Serial.println(color);

  // --- Acciones según color ---
  if (color == "verde") {
    Serial.println("➡️ Avanzar");
    avanzar(150);
  }
  else if (color == "celeste") {
    if (servoPos == 1) {
      servoPos = 0;
      servoGarra.write(0);
      Serial.println("🔵 Celeste: servo movido a 0");
    }
  }
  else if (color == "amarillo") {
    if (servoPos != 0) {
      servoPos = 0;
      servoGarra.write(0);
      Serial.println("🟡 Amarillo: servo ajustado a 0");
    }
    retroceder(150);
    delay(800);
    avanzar(200);
    delay(800);
    detener();
  }
  else {
    // Si no detecta color, sigue avanzando
    avanzar(120);
  }

  delay(200);
}

// ------------------ FUNCIONES ------------------

// --- Motor adelante ---
void avanzar(int vel) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, vel);
  analogWrite(ENB, vel);
}

// --- Motor atrás ---
void retroceder(int vel) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, vel);
  analogWrite(ENB, vel);
}

// --- Detener ---
void detener() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// --- Medir distancia con HC-SR04 ---
float medirDistancia() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duracion = pulseIn(ECHO, HIGH, 25000);
  float d = duracion * 0.034 / 2.0;
  return d;
}

// --- Detectar color ---
String detectarColor() {
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);

  float R = (float)r / (float)c * 256.0;
  float G = (float)g / (float)c * 256.0;
  float B = (float)b / (float)c * 256.0;

  // Imprimir valores
  Serial.print("R: "); Serial.print(R);
  Serial.print(" G: "); Serial.print(G);
  Serial.print(" B: "); Serial.println(B);

  // --- Clasificación por color ---
  if (G > R && G > B && G > 100) return "verde";
  if (B > R && G > 100 && B > 120) return "celeste";
  if (R > G && R > B && G > 80 && B < 100) return "amarillo";
  
  return "desconocido";
}