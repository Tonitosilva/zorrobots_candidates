// --- Pines de velocidad (PWM) ---
int VelocidadMotor1 = 6; 
int VelocidadMotor2 = 5;

// --- Pines de control de giro ---
int Motor1A = 13; 
int Motor1B = 12;  
int Motor2C = 11; 
int Motor2D = 10; 

// --- Sensores infrarrojos ---
int sensorIzq   = 2;
int sensorCentro = 3;
int sensorDer   = 4;

// --- Variables para lectura ---
int valIzq = 0;
int valCentro = 0;
int valDer = 0;

void setup() {
  Serial.begin(9600);
  delay(1000);

  pinMode(sensorIzq, INPUT);
  pinMode(sensorCentro, INPUT);
  pinMode(sensorDer, INPUT);

  pinMode(Motor1A, OUTPUT);
  pinMode(Motor1B, OUTPUT);
  pinMode(Motor2C, OUTPUT);
  pinMode(Motor2D, OUTPUT);
  pinMode(VelocidadMotor1, OUTPUT);
  pinMode(VelocidadMotor2, OUTPUT);

  analogWrite(VelocidadMotor1, 200); 
  analogWrite(VelocidadMotor2, 200);  

  // Motores apagados al inicio
  digitalWrite(Motor1A, LOW);
  digitalWrite(Motor1B, LOW);
  digitalWrite(Motor2C, LOW);
  digitalWrite(Motor2D, LOW);
}

void loop() {
  // Lectura de sensores
  valIzq = digitalRead(sensorIzq);
  valCentro = digitalRead(sensorCentro);
  valDer = digitalRead(sensorDer);

  Serial.print("Izq: ");
  Serial.print(valIzq);
  Serial.print(" | Centro: ");
  Serial.print(valCentro);
  Serial.print(" | Der: ");
  Serial.println(valDer);

  // --- Lógica de seguimiento ---

  // Caso 1: Solo centro detecta negro → recto
  if (valCentro == 1 && valIzq == 0 && valDer == 0) {
    Serial.println("Recto");
    adelante();
  }
  // Caso 2: Izquierda detecta negro → girar izquierda
  else if (valIzq == 1 && valCentro == 0) {
    Serial.println("Izquierda");
    izquierda();
  }
  // Caso 3: Derecha detecta negro → girar derecha
  else if (valDer == 1 && valCentro == 0) {
    Serial.println("Derecha");
    derecha();
  }
  // Caso 4: Todos detectan negro → detener (final o intersección)
  else if (valIzq == 1 && valCentro == 1 && valDer == 1) {
    Serial.println("Stop o Checkpoint");
    detener();
  }
  // Caso 5: Todos detectan blanco → buscar línea
  else {
    Serial.println("Buscando línea...");
    detener();
  }

  delay(20);
}

// --- Funciones de movimiento ---
void adelante() {
  digitalWrite(Motor1A, HIGH);
  digitalWrite(Motor1B, LOW);
  digitalWrite(Motor2C, HIGH);
  digitalWrite(Motor2D, LOW);
}

void izquierda() {
  digitalWrite(Motor1A, LOW);
  digitalWrite(Motor1B, LOW);
  digitalWrite(Motor2C, HIGH);
  digitalWrite(Motor2D, LOW);
}

void derecha() {
  digitalWrite(Motor1A, HIGH);
  digitalWrite(Motor1B, LOW);
  digitalWrite(Motor2C, LOW);
  digitalWrite(Motor2D, LOW);
}

void detener() {
  digitalWrite(Motor1A, LOW);
  digitalWrite(Motor1B, LOW);
  digitalWrite(Motor2C, LOW);
  digitalWrite(Motor2D, LOW);
}