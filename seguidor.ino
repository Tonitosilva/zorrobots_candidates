// --- Pines de sensores ---
#define sensorIzq 2
#define sensorDer 3

// --- Pines del puente H ---
#define ENA 5
#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11
#define ENB 6

void setup() {
  pinMode(sensorIzq, INPUT);
  pinMode(sensorDer, INPUT);
  
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  int izq = digitalRead(sensorIzq);
  int der = digitalRead(sensorDer);

  Serial.print("Izq: ");
  Serial.print(izq);
  Serial.print("  Der: ");
  Serial.println(der);

  // Ambos sensores sobre línea blanca → avanzar
  if (izq == 1 && der == 1) {
    adelante();
  }
  // Sensor izquierdo detecta negro → girar a la izquierda
  else if (izq == 0 && der == 1) {
    izquierda();
  }
  // Sensor derecho detecta negro → girar a la derecha
  else if (izq == 1 && der == 0) {
    derecha();
  }
  // Ambos detectan negro → detenerse
  else {
    detener();
  }
}

// --- FUNCIONES DE MOVIMIENTO ---
void adelante() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 200);
  analogWrite(ENB, 200);
}

void izquierda() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 150);
  analogWrite(ENB, 150);
}

void derecha() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, 150);
  analogWrite(ENB, 150);
}

void detener() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
