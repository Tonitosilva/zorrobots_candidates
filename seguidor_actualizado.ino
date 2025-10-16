// Definición pines EnA y EnB para el control de la velocidad
int VelocidadMotor1 = 6;
int VelocidadMotor2 = 5;

// Definición de los pines de control de giro de los motores In1, In2, In3 e In4
int Motor1A = 13;
int Motor1B = 12; 
int Motor2C = 11;
int Motor2D = 8;

// Sensores infrarrojos (no se usan aún, pero definidos)
int infraPin  = 2;   
int infraPin1 = 4;

void setup() {
  // Configuración de pines como salida
  pinMode(VelocidadMotor1, OUTPUT);
  pinMode(VelocidadMotor2, OUTPUT);
  pinMode(Motor1A, OUTPUT);
  pinMode(Motor1B, OUTPUT);
  pinMode(Motor2C, OUTPUT);
  pinMode(Motor2D, OUTPUT);

  pinMode(infraPin, INPUT);
  pinMode(infraPin1, INPUT);

  Serial.begin(9600);
  Serial.println("Inicio de prueba de motores");
}

void loop() {
  // --- Avanzar ---
  Serial.println("Avanzando");
  digitalWrite(Motor1A, LOW);
  digitalWrite(Motor1B, HIGH);
  digitalWrite(Motor2C, LOW);
  digitalWrite(Motor2D, HIGH);
  analogWrite(VelocidadMotor1, 200); // velocidad (0–255)
  analogWrite(VelocidadMotor2, 200);
  delay(2000);


}
