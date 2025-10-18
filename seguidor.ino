// Pines de sensores
#define SENSOR_IZQ A0
#define SENSOR_DER A1

// Pines de motor
#define ENA 5    // Velocidad motor izquierdo (PWM)
#define IN1 13   // Cambiado de 4 → 13
#define IN2 12   // Cambiado de 3 → 12
#define ENB 6    // Velocidad motor derecho (PWM)
#define IN3 11   // Cambiado de 7 → 11
#define IN4 10   // Cambiado de 8 → 10

int valorIzq, valorDer;
float Kp = 0.05;
int velocidadBase = 120;

void setup() {
  pinMode(SENSOR_IZQ, INPUT);
  pinMode(SENSOR_DER, INPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  valorIzq = analogRead(SENSOR_IZQ);
  valorDer = analogRead(SENSOR_DER);

  int error = valorIzq - valorDer;
  int ajuste = Kp * error;

  int velocidadIzq = velocidadBase - ajuste;
  int velocidadDer = velocidadBase + ajuste;

  velocidadIzq = constrain(velocidadIzq, 0, 255);
  velocidadDer = constrain(velocidadDer, 0, 255);

  moverAdelante(velocidadIzq, velocidadDer);
}

void moverAdelante(int velIzq, int velDer) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, velIzq);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, velDer);
}