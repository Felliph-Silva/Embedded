#include <Arduino.h>
#include <LiquidCrystal.h>

// Definição dos pinos
#define PUMP1        A5
#define PUMP2        A4

const int rs = 13, en = 12, d4 = 5, d5 = 4, d6 = 3, d7 = 2;

#define BUTTON_C1    6
#define BUTTON_C2    7

#define MIXER        8
#define VALVE        9

#define LOW_LEVEL    10
#define HIGH_LEVEL   11

// Definição do LCD
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Estados do ciclo
enum CycleState { IDLE, WAITING_CONFIRMATION, MIXING, INTERRUPTED };
CycleState state = IDLE;


unsigned long lastPressTime = 0;
const unsigned long TIMEOUT = 10000; // 10 segundos

const unsigned long PUMP_TIME = 5000;  // 5 segundos
const unsigned long MIXER_TIME = 5000; // 5 segundos

//Configurações dos sensores

void checkButtons();
bool checkInterrupt();
void preparePumps();
void prepareButtons();
void prepareActuators();
void prepareSensors();
void runMixingCycle();
void displayMessage(String message);
void resetToIdle();
void allOff();
void emptyContainer();
void turnOnMixer();
void turnOnPump1();
void turnOnPump2();
void resetToIdle();

void setup() {
  // Configuração dos pinos
  preparePumps();
  prepareButtons();
  prepareActuators();
  prepareSensors();

  // Inicializa o display LCD
  lcd.begin(16, 2);            
  lcd.clear(); 
  lcd.print("Sistema pronto");
  
  Serial.begin(9600);
  Serial.println("Sistema pronto");
}

void loop() {
  // Verifica o estado do ciclo
  switch (state) {
    case IDLE:
      checkButtons();
      break;

    case WAITING_CONFIRMATION:
      if (millis() - lastPressTime > TIMEOUT) {
        resetToIdle();
      } else {
        checkButtons();
      }
      break;

    case MIXING:
      runMixingCycle();
      break;

    case INTERRUPTED:
      displayMessage("Ciclo interrompido");
      Serial.println("Ciclo interrompido");
      delay(2000); // Aguarda 2 segundos antes de retornar ao estado IDLE
      resetToIdle();
      break;
  }
}
void checkButtons() {
  if (digitalRead(BUTTON_C1) == LOW) {
    if (state == IDLE) {
      state = WAITING_CONFIRMATION;
      lastPressTime = millis();
      displayMessage("Iniciar ciclo? Pressione C1");
      Serial.println("Iniciar ciclo? Pressione C1");
    } else if (state == WAITING_CONFIRMATION) {
      state = MIXING;
      lastPressTime = millis(); // Reinicia o contador do tempo
      displayMessage("Iniciando ciclo...");
    }
    delay(200); // Debounce para evitar múltiplos acionamentos
  } else if (digitalRead(BUTTON_C2) == LOW) {
    if (state == MIXING || state == WAITING_CONFIRMATION) {
      state = INTERRUPTED;
    } else if (state == IDLE) {
      emptyContainer();
      resetToIdle();
    }
    delay(200); // Debounce para evitar múltiplos acionamentos
  }
}

void runMixingCycle() {
  if (state != MIXING) return; // Verifica se ainda está no estado MIXING
  turnOnPump1();

  if (state != MIXING) return; // Verifica se ainda está no estado MIXING
  turnOnPump2();

  if (state != MIXING) return; // Verifica se ainda está no estado MIXING
  turnOnMixer();

  if (state != MIXING) return; // Verifica se ainda está no estado MIXING
  emptyContainer();

  if (state == MIXING) {
    displayMessage("Ciclo concluido");
    Serial.println("Ciclo concluido");
    delay(2000); // Aguarda 2 segundos antes de retornar ao estado IDLE
    resetToIdle();
  }
}

bool checkInterrupt() {
  if (digitalRead(BUTTON_C2) == LOW) {
    allOff(); // desliga todos os equipamentos
    state = INTERRUPTED;
    return true;
  }
  return false;
}

void allOff(){
    // Desliga todos os equipamentos
    digitalWrite(PUMP1, LOW);
    digitalWrite(PUMP2, LOW);
    digitalWrite(MIXER, LOW);
    digitalWrite(VALVE, LOW);
}

void emptyContainer() {
  displayMessage("Esvaziando recipiente...");
  Serial.println("Esvaziando recipiente...");

  digitalWrite(VALVE, HIGH);
  while (digitalRead(LOW_LEVEL) == HIGH) {
    if (state == MIXING && checkInterrupt()) return; // Verifica se o botão C2 foi pressionado apenas no estado MIXING
  }
  digitalWrite(VALVE, LOW);
  displayMessage("Recipiente vazio");
  delay(2000); // Mostra a mensagem por 2 segundos
}


void turnOnMixer() {
  if (state != MIXING) return;
  
  unsigned long startTime = millis();
  displayMessage("Ligando misturador");
  Serial.println("Ligando misturador");

  digitalWrite(MIXER, HIGH);
  while (millis() - startTime < 5000) {
    if (checkInterrupt() || state != MIXING) return;  // Verifica se o botão C2 foi pressionado ou o estado mudou
  }
  digitalWrite(MIXER, LOW);
}

void turnOnPump1() {
  if (state != MIXING) return;
  
  displayMessage("Acionando bomba1");
  Serial.println("Acionando bomba 1...");
  unsigned long startTime = millis();

  digitalWrite(PUMP1, HIGH);
  while (millis() - startTime < 5000) {
    if (checkInterrupt() || state != MIXING) return; // Verifica se o botão C2 foi pressionado ou o estado mudou
  }
  digitalWrite(PUMP1, LOW);
}

void turnOnPump2() {
  if (state != MIXING) return;

  displayMessage("Acionando bomba2");
  Serial.println("Acionando bomba2...");

  digitalWrite(PUMP2, HIGH);
  while (digitalRead(HIGH_LEVEL) == HIGH) {
    if (checkInterrupt() || state != MIXING) return; // Verifica se o botão C2 foi pressionado ou o estado mudou
  }
  digitalWrite(PUMP2, LOW);
}

void resetToIdle() {
  state = IDLE;
  displayMessage("Sistema pronto");
}

void displayMessage(String message) {
  if(message.length() >= 16)
  {
    String message1 = message.substring(0, 16);
    String message2 = message.substring(16, message.length());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message1);
    lcd.setCursor(0, 1);
    lcd.print(message2);
  }
  
  else
  {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
  }
}

void preparePumps() {
  pinMode(PUMP1, OUTPUT);
  pinMode(PUMP2, OUTPUT);
}

void prepareButtons() {
  pinMode(BUTTON_C1, INPUT_PULLUP);
  pinMode(BUTTON_C2, INPUT_PULLUP);
}

void prepareActuators() {
  pinMode(MIXER, OUTPUT);
  pinMode(VALVE, OUTPUT);
}

void prepareSensors() {
  pinMode(LOW_LEVEL, INPUT_PULLUP);
  pinMode(HIGH_LEVEL, INPUT_PULLUP);
}