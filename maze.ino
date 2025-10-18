// --- Sensores TCRT ---
#define TCRT_IZQ A0
#define TCRT_DER A1
#define TCRT_THRESHOLD 400

// --- Sensor ultrasónico HC-SR04 ---
#define TRIG 8
#define ECHO 9

// --- Motores L298N ---
#define ENA 5
#define IN1 13
#define IN2 12
#define IN3 11
#define IN4 10
#define ENB 4

int vel = 130; 
int vel2 = 150;// velocidad PWM

void setup() {
  Serial.begin(9600);

  // Pines motores
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);

  // Pines TCRT
  pinMode(TCRT_IZQ, INPUT);
  pinMode(TCRT_DER, INPUT);

  // Pines ultrasónico
  pinMode(TRIG, OUTPUT); pinMode(ECHO, INPUT);

  detener();
}

void loop() {
  // Leer sensores
  int izq = analogRead(TCRT_IZQ);
  int der = analogRead(TCRT_DER);
  float distancia = medirDistancia();

  Serial.print("Izq: "); Serial.print(izq);
  Serial.print(" | Der: "); Serial.print(der);
  Serial.print(" | Distancia: "); Serial.println(distancia);

  // --- Evitar obstáculos ---
  if (distancia > 0 && distancia < 15) {
    Serial.println("🛑 Obstáculo cerca, retrocediendo y girando...");
    retroceder();
    delay(500);
    girarDerecha();
    delay(500);
    detener();
    return;
  }

  // --- Evitar negro ---
  if (izq < TCRT_THRESHOLD && der < TCRT_THRESHOLD) {
    Serial.println("⚫ Negro detectado delante, retrocediendo...");
    retroceder();
    delay(500);
    girarDerecha();
    delay(500);
    detener();
  }
  else if (izq < TCRT_THRESHOLD) {
    Serial.println("⚫ Negro izq, girando derecha");
    girarDerecha();
    delay(300);
  }
  else if (der < TCRT_THRESHOLD) {
    Serial.println("⚫ Negro der, girando izquierda");
    girarIzquierda();
    delay(300);
  }
  else {
    adelante();
  }

  delay(50);
}

// --- Movimiento ---
void adelante() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, vel); analogWrite(ENB, vel2);
}

void retroceder() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, vel); analogWrite(ENB, vel2);
}

void girarIzquierda() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, vel); analogWrite(ENB, vel2);
}

void girarDerecha() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, vel); analogWrite(ENB, vel2);
}

void detener() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0); analogWrite(ENB, 0);
}

// --- Medir distancia ---
float medirDistancia() {
  digitalWrite(TRIG, LOW); delayMicroseconds(2);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duracion = pulseIn(ECHO, HIGH, 25000); // timeout 25ms
  float d = duracion * 0.034 / 2.0; // cm
  return d;
}