/* Pista C - Navegación básica 4x4 con detección de color
   Hardware asumido: Arduino Uno, TCS34725 (I2C), HC-SR04, TCRT5000, 2 motores DC via H-bridge, servo, 3 LEDs.
   Librerías requeridas: Adafruit_TCS34725, NewPing, Servo
*/

#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <NewPing.h>
#include <Servo.h>

/* ------------------- CONFIGURACIÓN HARDWARE (ajustar) ------------------- */
// Motor driver pins (ejemplo L298N)
const int M1_IN1 = 2;
const int M1_IN2 = 3;
const int M1_EN  = 5; // PWM
const int M2_IN1 = 4;
const int M2_IN2 = 7;
const int M2_EN  = 6; // PWM

// Ultrasonido HC-SR04
const int SONAR_TRIG = 10;
const int SONAR_ECHO = 11;
NewPing sonar(SONAR_TRIG, SONAR_ECHO, 200); // max 200 cm

// TCS34725 (I2C) - sensor color
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

// TCRT5000 (sensor reflectancia frontal) - digital pin
const int TCRT_PIN = A0; // usar analógico para medir reflectancia

// Servo para indicación (opcional)
Servo indicatorServo;
const int SERVO_PIN = 9;

// LEDs indicadores (o LED RGB)
const int LED_BLUE = 12;
const int LED_YELLOW = 8;
const int LED_PINK = 13; // si usas LED rosa puedes conectar con resistor
// Puedes usar LED rojo/verde si quieres

/* ------------------- CONSTANTES DE MOVIMIENTO / CALIBRACIÓN ------------------- */
const int TILE_MM = 300; // mm por casilla (ajustar)
int TILE_TIME_MS = 1200; // tiempo estimado para recorrer un tile (calibrar)
int TURN_TIME_90_MS = 450; // tiempo para girar 90° en sitio (calibrar)
const int MOTOR_SPEED = 200; // 0-255 PWM

const int WALL_THRESH_CM = 18; // si hay objeto a menos de 18cm se considera pared
const int TCRT_THRESHOLD = 400; // umbral analogico para reflectancia (calibrar)
const int BLACK_CLEAR_THRESHOLD = 50; // lectura RGB sum para detectar negro (ajustar)

/* ------------------- MAPA Y POSICIÓN ------------------- */
enum Dir {NORTH=0, EAST=1, SOUTH=2, WEST=3};
enum CellState {UNKNOWN=0, FREE=1, WALL=2, BLACK_TILE=3};
enum ColorLabel {C_NONE=0, C_BLUE=1, C_YELLOW=2, C_PINK=3, C_GREEN=4, C_RED=5, C_BLACK=6};

struct Cell {
  CellState state;
  ColorLabel color;
  bool visited;
};

Cell MAP[4][4];
int curX = 0, curY = 0;
Dir curDir = NORTH;
bool foundExit = false;

/* ------------------- PROTOTIPOS ------------------- */
void motorForward(int pwm);
void motorBackward(int pwm);
void motorStop();
void motorTurnLeft90();
void motorTurnRight90();
void moveOneTile(); // avanza una casilla
int readDistanceCM();
ColorLabel readColor();
void indicateColor(ColorLabel c);
bool cellInBounds(int x,int y);
void markWallFront();
void stepTo(int nx,int ny, Dir nd); // actualizar pose
void attemptMove(Dir d);
void rotateTo(Dir target);
void backtrackToStart();
void printMap();

/* ------------------- SETUP ------------------- */
void setup() {
  Serial.begin(115200);
  delay(100);
  // Init pins
  pinMode(M1_IN1, OUTPUT); pinMode(M1_IN2, OUTPUT); pinMode(M1_EN, OUTPUT);
  pinMode(M2_IN1, OUTPUT); pinMode(M2_IN2, OUTPUT); pinMode(M2_EN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT); pinMode(LED_YELLOW, OUTPUT); pinMode(LED_PINK, OUTPUT);
  pinMode(TCRT_PIN, INPUT);

  // Servo
  indicatorServo.attach(SERVO_PIN);
  indicatorServo.write(90);

  // Init TCS
  if (tcs.begin()) {
    Serial.println("TCS34725 iniciado");
  } else {
    Serial.println("ERROR: TCS34725 no detectado");
  }

  // Inicializar mapa
  for (int i=0;i<4;i++) for (int j=0;j<4;j++){
    MAP[i][j].state = UNKNOWN;
    MAP[i][j].color = C_NONE;
    MAP[i][j].visited = false;
  }
  // Start cell is green and visited
  MAP[curX][curY].color = C_GREEN;
  MAP[curX][curY].visited = true;
  MAP[curX][curY].state = FREE;

  Serial.println("Setup completo. Calibra TILE_TIME_MS y TURN_TIME_90_MS si es necesario.");
}

/* ------------------- LOOP (FSM simple) ------------------- */
void loop() {
  if (foundExit) {
    Serial.println("Salida encontrada. Fin.");
    motorStop();
    while(1) delay(1000);
  }

  // Decide próxima dirección usando right-hand rule
  // Prioridades: right, forward, left, back
  Dir rights[] = {(Dir)((curDir+1)%4), curDir, (Dir)((curDir+3)%4), (Dir)((curDir+2)%4)};
  bool moved = false;
  for (int i=0;i<4;i++){
    Dir d = rights[i];
    int nx = curX, ny = curY;
    if (d==NORTH) ny--;
    if (d==SOUTH) ny++;
    if (d==EAST) nx++;
    if (d==WEST) nx--;
    if (!cellInBounds(nx,ny)) continue;
    if (MAP[nx][ny].state==WALL || MAP[nx][ny].state==BLACK_TILE) continue;
    // check front distance if the candidate is the forward cell relative to current orientation (i==1 means forward) or right (i==0)
    // For safety always check distance before attempt
    // Turn to that direction and try move
    attemptMove(d);
    moved = true;
    break;
  }

  if (!moved) {
    // Backtrack: no moves possible (shouldn't happen often). We'll rotate 180 and try.
    motorTurnLeft90();
    delay(100);
    motorTurnLeft90();
    curDir = (Dir)((curDir+2)%4);
    Serial.println("Backtracking giro 180");
  }

  delay(50);
}

/* ------------------- FUNCIONES MOVIMIENTO / MOTORES ------------------- */
void motorForward(int pwm){
  analogWrite(M1_EN, pwm);
  analogWrite(M2_EN, pwm);
  digitalWrite(M1_IN1, HIGH);
  digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN1, HIGH);
  digitalWrite(M2_IN2, LOW);
}

void motorBackward(int pwm){
  analogWrite(M1_EN, pwm);
  analogWrite(M2_EN, pwm);
  digitalWrite(M1_IN1, LOW);
  digitalWrite(M1_IN2, HIGH);
  digitalWrite(M2_IN1, LOW);
  digitalWrite(M2_IN2, HIGH);
}

void motorStop(){
  analogWrite(M1_EN, 0);
  analogWrite(M2_EN, 0);
  digitalWrite(M1_IN1, LOW);
  digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN1, LOW);
  digitalWrite(M2_IN2, LOW);
}

void motorTurnLeft90(){
  // Girar izquierda en sitio (M1 backward, M2 forward)
  analogWrite(M1_EN, MOTOR_SPEED);
  analogWrite(M2_EN, MOTOR_SPEED);
  digitalWrite(M1_IN1, LOW);
  digitalWrite(M1_IN2, HIGH);
  digitalWrite(M2_IN1, HIGH);
  digitalWrite(M2_IN2, LOW);
  delay(TURN_TIME_90_MS);
  motorStop();
}

void motorTurnRight90(){
  analogWrite(M1_EN, MOTOR_SPEED);
  analogWrite(M2_EN, MOTOR_SPEED);
  digitalWrite(M1_IN1, HIGH);
  digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN1, LOW);
  digitalWrite(M2_IN2, HIGH);
  delay(TURN_TIME_90_MS);
  motorStop();
}

void moveOneTile(){
  // Avanzar durante TILE_TIME_MS monitoreando sonar y TCRT
  unsigned long start = millis();
  motorForward(MOTOR_SPEED);
  while (millis() - start < TILE_TIME_MS) {
    int d = readDistanceCM();
    if (d>0 && d < WALL_THRESH_CM) {
      // Pared inesperada: detener y retroceder
      motorStop();
      Serial.println("Obstaculo detectado durante avance. Retrocediendo.");
      motorBackward(MOTOR_SPEED);
      delay(300);
      motorStop();
      return;
    }
    // Si el TCRT indica que hemos avanzado mucho (ejemplo > umbral), continuar
    int refl = analogRead(TCRT_PIN);
    // (No lo usamos aquí para cortar, pero se podría)
    delay(10);
  }
  motorStop();
}

/* ------------------- SENSORES ------------------- */
int readDistanceCM(){
  unsigned int uS = sonar.ping();
  if (uS==0) return -1;
  int cm = uS / US_ROUNDTRIP_CM;
  return cm;
}

ColorLabel readColor(){
  // Read raw from TCS34725
  uint16_t r,g,b,c;
  tcs.getRawData(&r,&g,&b,&c);
  // Sum low -> black
  uint32_t sum = (uint32_t)r + g + b;
  Serial.print("Color RAW R G B C: "); Serial.print(r); Serial.print(" "); Serial.print(g); Serial.print(" "); Serial.print(b); Serial.print(" "); Serial.println(c);
  if (sum < BLACK_CLEAR_THRESHOLD) return C_BLACK;

  // Simple ratio based classification (necesita calibración):
  float fr = (float)r / (float)sum;
  float fg = (float)g / (float)sum;
  float fb = (float)b / (float)sum;

  // Estas reglas son heurísticas; calibrar con muestras
  if (fb > 0.35 && fr < 0.30) return C_BLUE;        // azul: B dominante
  if (fr > 0.45 && fg > 0.30) return C_YELLOW;      // amarillo: R+G altos
  if (fr > 0.35 && fb > 0.25) return C_PINK;        // rosa: R y B medios
  if (fg > 0.45 && fr < 0.35 && fb < 0.35) return C_GREEN;
  if (fr > 0.60 && fg < 0.25 && fb < 0.25) return C_RED;
  return C_NONE;
}

/* ------------------- INDICADOR ------------------- */
void indicateColor(ColorLabel c){
  // Apagar
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_PINK, LOW);
  indicatorServo.write(90);

  if (c==C_BLUE) {
    digitalWrite(LED_BLUE, HIGH);
    indicatorServo.write(60);
    Serial.println("INDICATOR: AZUL");
  } else if (c==C_YELLOW) {
    digitalWrite(LED_YELLOW, HIGH);
    indicatorServo.write(120);
    Serial.println("INDICATOR: AMARILLO");
  } else if (c==C_PINK) {
    digitalWrite(LED_PINK, HIGH);
    indicatorServo.write(30);
    Serial.println("INDICATOR: ROSA");
  } else if (c==C_RED) {
    // Exit
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(LED_PINK, HIGH);
    Serial.println("INDICATOR: ROJO - SALIDA");
  }
  delay(700);
  // apagar todo al final
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_PINK, LOW);
  indicatorServo.write(90);
}

/* ------------------- MAPA Y NAVEGACIÓN ------------------- */
bool cellInBounds(int x,int y){
  return (x>=0 && x<4 && y>=0 && y<4);
}

void markWallFront(){
  int nx = curX, ny = curY;
  if (curDir==NORTH) ny--;
  else if (curDir==SOUTH) ny++;
  else if (curDir==EAST) nx++;
  else if (curDir==WEST) nx--;
  if (cellInBounds(nx,ny)) {
    MAP[nx][ny].state = WALL;
    Serial.print("Marcada pared en "); Serial.print(nx); Serial.print(","); Serial.println(ny);
  }
}

void stepTo(int nx,int ny, Dir nd){
  curX = nx; curY = ny; curDir = nd;
}

void attemptMove(Dir d){
  // Girar a la dir d
  rotateTo(d);

  // Comprobar pared con sonar
  int dist = readDistanceCM();
  if (dist>0 && dist < WALL_THRESH_CM) {
    Serial.println("PARED detectada frente. Marcando y saliendo.");
    // marcar pared en mapa según d
    int wx = curX, wy = curY;
    if (d==NORTH) wy--;
    if (d==SOUTH) wy++;
    if (d==EAST) wx++;
    if (d==WEST) wx--;
    if (cellInBounds(wx,wy)) MAP[wx][wy].state = WALL;
    return;
  }

  // Mover 1 tile
  moveOneTile();

  // Estimar nueva coordenada
  int nx = curX, ny = curY;
  if (d==NORTH) ny--;
  if (d==SOUTH) ny++;
  if (d==EAST) nx++;
  if (d==WEST) nx--;

  if (!cellInBounds(nx,ny)) {
    Serial.println("Fuera de bounds tras movimiento, retrocediendo.");
    motorBackward(MOTOR_SPEED); delay(300);
    motorStop();
    return;
  }

  // Leer color en la celda
  delay(150); // estabilizar
  ColorLabel c = readColor();

  if (c==C_BLACK) {
    // Retroceder fuera de la casilla
    Serial.println("CASILLA NEGRA detectada. Retrocediendo y marcando.");
    motorBackward(MOTOR_SPEED);
    delay(400); motorStop();
    MAP[nx][ny].state = BLACK_TILE;
    return;
  }

  // Actualizar mapa y posición
  stepTo(nx, ny, d);
  MAP[nx][ny].visited = true;
  MAP[nx][ny].state = FREE;
  if (c != C_NONE) {
    MAP[nx][ny].color = c;
    // Indicar si es uno de los colores puntuables
    if (c==C_BLUE || c==C_YELLOW || c==C_PINK) {
      indicateColor(c);
    } else if (c==C_RED) {
      indicateColor(c);
      foundExit = true;
    }
  }
  Serial.print("Pos actual: "); Serial.print(curX); Serial.print(","); Serial.print(curY); Serial.print(" Dir: "); Serial.println(curDir);
  printMap();
}

void rotateTo(Dir target){
  int diff = (target - curDir + 4) % 4;
  if (diff==0) return;
  if (diff==1) {
    motorTurnRight90();
  } else if (diff==2) {
    motorTurnRight90(); delay(100); motorTurnRight90();
  } else if (diff==3) {
    motorTurnLeft90();
  }
  curDir = target;
}

/* ------------------- UTILIDADES ------------------- */
void backtrackToStart(){
  // Función simple: girar 180 y moverse 4 tiles (asumiendo camino libre)
  Serial.println("Back to start (simple) invoked");
  motorTurnLeft90(); motorTurnLeft90();
  curDir = (Dir)((curDir+2)%4);
  for (int i=0;i<4;i++){
    moveOneTile();
  }
  // No es una ruta robusta; mejor implementar navegación por mapa (A*)
}

void printMap(){
  Serial.println("Map:");
  for (int y=0;y<4;y++){
    for (int x=0;x<4;x++){
      char ch = '?';
      if (MAP[x][y].state==UNKNOWN) ch='.';
      if (MAP[x][y].state==FREE) ch='o';
      if (MAP[x][y].state==WALL) ch='W';
      if (MAP[x][y].state==BLACK_TILE) ch='B';
      if (x==curX && y==curY) ch='X';
      Serial.print(ch);
      Serial.print(' ');
    }
    Serial.println();
  }
  Serial.println("----");
}