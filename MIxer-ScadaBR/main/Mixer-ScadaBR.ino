#ifdef ESP8266
 #include <ESP8266WiFi.h>
#else //ESP32
 #include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>

#include <LiquidCrystal.h>

//Modbus Registers Offsets
const int C1_COIL = 100;
const int C2_COIL = 101;
const int PUMP1_COIL = 102;
const int PUMP2_COIL = 103;
const int MIXER_COIL = 104;
const int VALVE_COIL = 105;
const int LOW_LEVEL_COIL = 106;
const int HIGH_LEVEL_COIL = 107;

//Used Pins
const int PUMP1 = 13;
const int PUMP2= 12; 

const int BUTTON_C1 = 14;
const int BUTTON_C2 = 27;

const int MIXER = 26;
const int VALVE = 25;

const int LOW_LEVEL = 33; 
const int HIGH_LEVEL = 32; 

const int rs = 35, en = 34, d4 = 2, d5 = 4, d6 = 5, d7 = 18;

//ModbusIP object
ModbusIP mb;

// Definição do LCD
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Estados do ciclo
enum CycleState { IDLE, WAITING_CONFIRMATION, MIXING, INTERRUPTED };
CycleState state = IDLE;

unsigned long lastPressTime = 0;
const unsigned long TIMEOUT = 10000; // 10 segundos

const unsigned long PUMP_TIME = 5000;  // 5 segundos
const unsigned long MIXER_TIME = 5000; // 5 segundos

//status components
bool status_pump1 = LOW;
bool status_pump2 = LOW;
bool status_valve = LOW;
bool status_mixer = LOW;

void setup() {
  Serial.begin(115200);
 
  WiFi.begin("IFPB-AUTO2", "");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //MODBUS
  mb.server();

  // Configuração dos pinos
  preparePumps();
  prepareButtons();
  prepareActuators();
  prepareSensors();

  // Inicializa o display LCD
  lcd.begin(16, 2);            
  lcd.clear(); 
  lcd.print("Sistema pronto");

  //MODBUS Registers
  mb.addCoil(C1_COIL);
  mb.addCoil(C2_COIL);
  mb.addCoil(LOW_LEVEL_COIL);
  mb.addCoil(HIGH_LEVEL_COIL);
  mb.addIreg(PUMP1_COIL);
  mb.addIreg(PUMP2_COIL);
  mb.addIreg(MIXER_COIL);
  mb.addIreg(VALVE_COIL);
}
 
void loop() {
  //Call once inside loop() - all magic here
  mb.task();

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

delay(10);
}

void checkButtons() {
  int C1_READ = digitalRead(BUTTON_C1) == LOW;
  int C2_READ = digitalRead(BUTTON_C2) == LOW;

  mb.Coil(C1_COIL, C1_READ);
  mb.Coil(C2_COIL, C2_READ);

  if (C1_READ) {
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
  } else if (C2_READ) {
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
  status_valve = HIGH;
  mb.Coil(VALVE_COIL, status_valve);

  while (digitalRead(LOW_LEVEL) == HIGH) {
    if (state == MIXING && checkInterrupt()) return; // Verifica se o botão C2 foi pressionado apenas no estado MIXING
  }
  digitalWrite(VALVE, LOW);
  status_valve = LOW;
  mb.Coil(VALVE_COIL, status_valve);

  displayMessage("Recipiente vazio");
  delay(2000); // Mostra a mensagem por 2 segundos
}


void turnOnMixer() {
  if (state != MIXING) return;
  
  unsigned long startTime = millis();
  displayMessage("Ligando misturador");
  Serial.println("Ligando misturador");

  digitalWrite(MIXER, HIGH);
  status_mixer = HIGH;
  mb.Coil(MIXER_COIL,status_mixer);

  while (millis() - startTime < 5000) {
    if (checkInterrupt() || state != MIXING) return;  // Verifica se o botão C2 foi pressionado ou o estado mudou
  }
  digitalWrite(MIXER, LOW);
  status_mixer = LOW;
  mb.Coil(MIXER_COIL,status_mixer);
}

void turnOnPump1() {
  if (state != MIXING) return;
  
  displayMessage("Acionando bomba1");
  Serial.println("Acionando bomba 1...");
  unsigned long startTime = millis();

  digitalWrite(PUMP1, HIGH);
  status_pump1 = HIGH;
  mb.Coil(PUMP1_COIL, status_pump1);

  while (millis() - startTime < 5000) {
    if (checkInterrupt() || state != MIXING) return; // Verifica se o botão C2 foi pressionado ou o estado mudou
  }
  digitalWrite(PUMP1, LOW);
  status_pump1 = LOW;
  mb.Coil(PUMP1_COIL, status_pump1);
}

void turnOnPump2() {
  if (state != MIXING) return;

  displayMessage("Acionando bomba2");
  Serial.println("Acionando bomba2...");

  digitalWrite(PUMP2, HIGH);
  status_pump2 = HIGH;
  mb.Coil(PUMP2_COIL, status_pump2);

  while (digitalRead(HIGH_LEVEL) == HIGH) {
    if (checkInterrupt() || state != MIXING) return; // Verifica se o botão C2 foi pressionado ou o estado mudou
  }
  digitalWrite(PUMP2, LOW);
  status_pump2 = LOW;
  mb.Coil(PUMP2_COIL, status_pump2);
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
