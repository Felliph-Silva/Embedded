
// ## Display LCD 16x2

#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 13, d4 = A2, d5 = A3, d6 = A4, d7 = A5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define PINO_C3 A0
#define PINO_C4 A1

#define PINO_B1 8
#define PINO_B2 9

#define PINO_MISTURADOR 7
#define PINO_VALVULA 6

#define PINO_S1 5
#define PINO_S2 4
uint8_t estado_b3, estado_b4;
uint8_t estado_s1, estado_s2;


// CODE THAT RUNS ONCE 
void setup() {
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Initializing");
  pinMode(PINO_C3, INPUT_PULLUP);
  pinMode(PINO_C4, INPUT_PULLUP);
  pinMode(PINO_S1, INPUT_PULLUP);
  pinMode(PINO_S2, INPUT_PULLUP);
  estado_b3 = digitalRead(PINO_C3);
  estado_b4 = digitalRead(PINO_C4);
  estado_s1 = digitalRead(PINO_S1);
  estado_s2 = digitalRead(PINO_S2);
  pinMode(PINO_B1, OUTPUT);
  pinMode(PINO_B2, OUTPUT);
  pinMode(PINO_MISTURADOR, OUTPUT);
  pinMode(PINO_VALVULA, OUTPUT);
  digitalWrite(PINO_B1, LOW);
  digitalWrite(PINO_B2, LOW);
  digitalWrite(PINO_MISTURADOR, LOW);
  digitalWrite(PINO_VALVULA, LOW);
  delay (1000);
  montaTela(estado_b3, estado_b4, estado_s1, estado_s2);
}


void montaTela(uint8_t b3, uint8_t b4, uint8_t s1, uint8_t s2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("B3 = ");
  lcd.write(b3 + 0x30);
  lcd.print(" - B4 = ");
  lcd.write(b4 + 0x30);
  lcd.setCursor(0, 1);
  lcd.print("S1 = ");
  lcd.write(s1 + 0x30);
  lcd.print(" - S2 = ");
  lcd.write(s2 + 0x30);

}

void loop() {
  if(digitalRead(PINO_C3) != estado_b3 || digitalRead(PINO_C4) != estado_b4 ||
     digitalRead(PINO_S1) != estado_s1 || digitalRead(PINO_S2) != estado_s2)
  {
    estado_b3 = digitalRead(PINO_C3);
    estado_b4 = digitalRead(PINO_C4);
    estado_s1 = digitalRead(PINO_S1);
    estado_s2 = digitalRead(PINO_S2);
    montaTela(estado_b3, estado_b4, estado_s1, estado_s2);

    if(estado_b3 == 0 && estado_b4 == 1)
    {
      digitalWrite(PINO_B1, HIGH);
      digitalWrite(PINO_B2, LOW);
      digitalWrite(PINO_MISTURADOR, LOW);
      digitalWrite(PINO_VALVULA, LOW);
    }
    else if(estado_b3 == 1 && estado_b4 == 0) {
      digitalWrite(PINO_B1, LOW);
      digitalWrite(PINO_B2, HIGH);
      digitalWrite(PINO_MISTURADOR, LOW);
      digitalWrite(PINO_VALVULA, LOW);
    }
    else if(estado_b3 == 0 && estado_b4 == 0) {
      digitalWrite(PINO_B1, LOW);
      digitalWrite(PINO_B2, LOW);
      digitalWrite(PINO_MISTURADOR, LOW);
      digitalWrite(PINO_VALVULA, HIGH);
    }
    else {
      digitalWrite(PINO_B1, LOW);
      digitalWrite(PINO_B2, LOW);
      digitalWrite(PINO_MISTURADOR, HIGH);
      digitalWrite(PINO_VALVULA, LOW);
    }

  }
  
}