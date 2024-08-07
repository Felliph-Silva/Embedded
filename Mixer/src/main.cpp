#include <Arduino.h>
#include <LiquidCrystal.h>  // Inclua a biblioteca para controle do display LCD

// Definição dos pinos
#define PUMP1_1      2
#define PUMP1_2      3
#define PUMP2_1      4
#define PUMP2_2      5

#define BUTTON_C1    6
#define BUTTON_C2    7

#define MIXER        8
#define VALVE        9

#define LOW_LEVEL    10
#define HIGH_LEVEL   11

// Definição do LCD
LiquidCrystal lcd(12, 13, 14, 15, 16, 17); // Ajuste os pinos do LCD conforme sua ligação

// Estados do ciclo
enum CycleState { IDLE, WAITING_CONFIRMATION, MIXING, INTERRUPTED };
CycleState state = IDLE;

unsigned long lastPressTime = 0;
const unsigned long TIMEOUT = 10000; // 10 segundos

void setup() {
  // Configuração dos pinos
  preparePumps();
  prepareButtons();
  prepareActuators();
  prepareSensors();

  // Inicializa o display LCD
  lcd.begin(16, 2); // Ajuste o tamanho do LCD conforme necessário
  lcd.print("Sistema pronto");
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
    } else if (state == WAITING_CONFIRMATION) {
      state = MIXING;
      lastPressTime = millis(); // Reinicia o contador do tempo
      displayMessage("Iniciando ciclo...");
    }
    delay(200); // Debounce para evitar múltiplos acionamentos
  } else if (digitalRead(BUTTON_C2) == LOW) {
    if (state == MIXING) {
      state = INTERRUPTED;
    } else if (state == IDLE) {
      emptyContainer();
    }
    delay(200); // Debounce para evitar múltiplos acionamentos
  }
}

void runMixingCycle() {
  displayMessage("Acionando bomba 1...");
  turnOnPump1();

  displayMessage("Acionando bomba 2...");
  turnOnPump2();

  displayMessage("Ligando misturador...");
  turnOnMixer()

  displayMessage("Esvaziando recipiente...");
  emptyContainer();
  
  displayMessage("Ciclo concluído");
  delay(2000); // Aguarda 2 segundos antes de retornar ao estado IDLE
  resetToIdle();
}

void emptyContainer() {
  
  digitalWrite(VALVE, HIGH);
  while (digitalRead(LOW_LEVEL) == LOW) {
    // Aguarda até o sensor de nível vazio ser ativado
  }
  digitalWrite(VALVE, LOW);
  displayMessage("Recipiente esvaziado");
  delay(2000); // Aguarda 2 segundos antes de retornar ao estado IDLE
}

void turnOnMixer(){
   
  digitalWrite(MIXER, HIGH);
  delay(5000); // Aguarda 5 segundos
  digitalWrite(MIXER, LOW);
}

void turnOnPump1(){

  digitalWrite(PUMP1_1, HIGH);
  digitalWrite(PUMP1_2, LOW);
  delay(5000); // Aguarda 5 segundos
  digitalWrite(PUMP1_1, LOW);
  digitalWrite(PUMP1_2, LOW);
 }
 
void turnOnPump2(){
   digitalWrite(PUMP2_1, HIGH);
  digitalWrite(PUMP2_2, LOW);
  while (digitalRead(HIGH_LEVEL) == LOW) {
    // Aguarda até o sensor de nível cheio ser ativado
  }
  digitalWrite(PUMP2_1, LOW);
  digitalWrite(PUMP2_2, LOW);
 }

void resetToIdle() {
  state = IDLE;
  displayMessage("Sistema pronto");
}

void displayMessage(String message) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
}

void preparePumps() {
  pinMode(PUMP1_1, OUTPUT);
  pinMode(PUMP1_2, OUTPUT);
  pinMode(PUMP2_1, OUTPUT);
  pinMode(PUMP2_2, OUTPUT);
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
