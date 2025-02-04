// Pins
#define ENCA 3
#define ENCB 2
#define PWM 4
#define IN1 5
#define IN2 6
#define POT A0

// globals
long prevT = 0;
int posPrev = 0;
// Use the "volatile" directive for variables
// used in an interrupt
volatile int pos_i = 0;
volatile float velocity_i = 0;
volatile long prevT_i = 0;

float v1Filt = 0;
float v1Prev = 0;
float v2Filt = 0;
float v2Prev = 0;

float eintegral = 0;
float eprev = 0;

float kp = 1e-3;
float ki = 100;
float kd = 0.1; // Ajuste do controle derivativo
float e_integral_max = 500; // Limite para o erro integral

void setup() {
  Serial.begin(115200);

  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);
  pinMode(PWM, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(POT, INPUT);

  attachInterrupt(digitalPinToInterrupt(ENCA), readEncoder, RISING);
}

void loop() {

  // Read the position and velocity
  int pos = 0;
  float velocity2 = 0;
  
  noInterrupts(); // Disable interrupts temporarily while reading
  pos = pos_i;
  velocity2 = velocity_i;
  interrupts(); // Turn interrupts back on

  // Compute velocity with method 1
  long currT = micros();
  float deltaT = ((float)(currT - prevT)) / 1.0e6;
  float velocity1 = (pos - posPrev) / deltaT;
  posPrev = pos;
  prevT = currT;

  // Convert count/s to RPM
  float v1 = velocity1 / 600.0 * 60.0;
  float v2 = velocity2 / 600.0 * 60.0;

  // Low-pass filter (25 Hz cutoff)
  v1Filt = 0.854 * v1Filt + 0.0728 * v1 + 0.0728 * v1Prev;
  v1Prev = v1;
  v2Filt = 0.854 * v2Filt + 0.0728 * v2 + 0.0728 * v2Prev;
  v2Prev = v2;

  // Set a target
  int pot = analogRead(POT);
  int pot_scaled = map(pot, 0, 1023, 0, 240);
  float vt = pot_scaled;

  // Compute the control signal u (PID)
  float e = vt - v1Filt;

  // Proteger o erro integral com limite
  eintegral += e * deltaT;
  if (eintegral > e_integral_max) {
    eintegral = e_integral_max;
  } else if (eintegral < -e_integral_max) {
    eintegral = -e_integral_max;
  }

  // Calcular erro derivativo
  float ederiv = (e - eprev) / deltaT;
  eprev = e;

  // Sinal de controle PID
  float u = kp * e + ki * eintegral + kd * ederiv;

  // Set the motor speed and direction
  int dir = 1;
  if (u < 0) {
    dir = -1;
  }

  int pwr = (int)fabs(u);
  if (pwr > 255) {
    pwr = 255;
  }
  setMotor(dir, pwr, PWM, IN1, IN2);

  // Serial output for debugging
  Serial.print(vt);
  Serial.print(" ");
  Serial.print(v1Filt);
  Serial.print(" ");
  Serial.print(v1);
  Serial.println();

  delay(50);
  attachInterrupt(digitalPinToInterrupt(ENCA), readEncoder, RISING);
}

void setMotor(int dir, int pwmVal, int pwm, int in1, int in2) {
  analogWrite(pwm, pwmVal); // Motor speed
  if (dir == 1) {
    // Turn one way
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else if (dir == -1) {
    // Turn the other way
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else {
    // Or don't turn
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
}

void readEncoder() {
  // Read encoder B when ENCA rises
  int b = digitalRead(ENCB);
  int increment = 0;
  if (b > 0) {
    // If B is high, increment forward
    increment = 1;
  } else {
    // Otherwise, increment backward
    increment = -1;
  }
  pos_i = pos_i + increment;

  // Compute velocity with method 2
  long currT = micros();
  float deltaT = ((float)(currT - prevT_i)) / 1.0e6;
  velocity_i = increment / deltaT;
  prevT_i = currT;
}
