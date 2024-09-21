#ifdef ESP8266
 #include <ESP8266WiFi.h>
#else //ESP32
 #include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>
#include <LiquidCrystal.h>

// Modbus Registers Offsets
const int C1_COIL = 100;
const int C2_COIL = 101;  
const int PUMP1_ISTS = 102;
const int PUMP2_ISTS = 103;
const int MIXER_ISTS = 104;
const int VALVE_ISTS = 105;
const int LOW_LEVEL_ISTS = 106;
const int HIGH_LEVEL_ISTS = 107;
const int IDLE_ISTS = 108;
const int WAITING_ISTS = 109;
const int MIXING_ISTS = 110;
const int INTERRUPTED_ISTS = 111;

//Used Pins
const int PUMP1 = 4;
const int PUMP2= 0; 

const int BUTTON_C1 = 16;
const int BUTTON_C2 = 5;

const int MIXER = 14;
const int VALVE = 12;

const int LOW_LEVEL = 13; 
const int HIGH_LEVEL = 15; 

//const int rs = 35, en = 34, d4 = 2, d5 = 4, d6 = 9, d7 = 18;


// ModbusIP object
ModbusIP mb;

// Definição do LCD
//LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Estados do ciclo
enum CycleState { IDLE, WAITING_CONFIRMATION, MIXING, INTERRUPTED };
CycleState state = IDLE;

unsigned long lastPressTime = 0;
const unsigned long TIMEOUT = 10000; // 10 segundos
const unsigned long PUMP_TIME = 5000;  // 5 segundos
const unsigned long MIXER_TIME = 5000; // 5 segundos

void setup() {
  Serial.begin(115200);
 
  WiFi.begin("Maxprint_MWR-150", "outrasenha");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // MODBUS
  mb.server();

  // Configuração dos pinos
  preparePumps();
  prepareButtons();
  prepareActuators();
  prepareSensors();

  // Inicializa o display LCD
  /*lcd.begin(16, 2);            
  lcd.clear(); 
  lcd.print("Sistema pronto");*/

  // Add the registers - Use addIsts() for digital inputs
  mb.addCoil(C1_COIL);
  mb.addCoil(C2_COIL);
  mb.addIsts(LOW_LEVEL_ISTS);
  mb.addIsts(HIGH_LEVEL_ISTS);
  mb.addIsts(PUMP1_ISTS);
  mb.addIsts(PUMP2_ISTS);
  mb.addIsts(MIXER_ISTS);
  mb.addIsts(VALVE_ISTS);
  mb.addIsts(IDLE_ISTS);
  mb.addIsts(WAITING_ISTS);
  mb.addIsts(MIXING_ISTS);
  mb.addIsts(INTERRUPTED_ISTS);

  Serial.println("Sistema pronto");
}

void loop() {
  // Call once inside loop() - all magic here
  mb.task();

  // Verifica o estado do ciclo
  switch (state) {
    case IDLE:  
    //Envia o estado para o supervisorio
      mb.Ists(IDLE_ISTS, HIGH);
      mb.Ists(WAITING_ISTS, LOW);
      mb.Ists(MIXING_ISTS, LOW);
      mb.Ists(INTERRUPTED_ISTS, LOW);

      checkButtons();  // Checa tanto os botões físicos quanto o estado Modbus
      break;

    case WAITING_CONFIRMATION:
      //Envia o estado para o supervisorio
      mb.Ists(IDLE_ISTS, LOW);
      mb.Ists(WAITING_ISTS, HIGH);
      mb.Ists(MIXING_ISTS, LOW);
      mb.Ists(INTERRUPTED_ISTS, LOW);

      if (millis() - lastPressTime > TIMEOUT) {
        resetToIdle();
      } else {
        checkButtons();
      }
      break;

    case MIXING:
      //Envia o estado para o supervisorio
      mb.Ists(IDLE_ISTS, LOW);
      mb.Ists(WAITING_ISTS, LOW);
      mb.Ists(MIXING_ISTS, HIGH);
      mb.Ists(INTERRUPTED_ISTS, LOW);

      runMixingCycle();
      break;

    case INTERRUPTED:
      //Envia o estado para o supervisorio
      mb.Ists(IDLE_ISTS, LOW);
      mb.Ists(WAITING_ISTS, LOW);
      mb.Ists(MIXING_ISTS, LOW);
      mb.Ists(INTERRUPTED_ISTS, HIGH);

      //displayMessage("Ciclo interrompido");
      Serial.println("Ciclo interrompido");
      delay(2000); // Aguarda 2 segundos antes de retornar ao estado IDLE
      resetToIdle();
      break;
  }
}

void checkButtons() {
  // Verifica o estado dos botões via Modbus
  bool modbusC1Pressed = mb.Coil(C1_COIL);
  bool modbusC2Pressed = mb.Coil(C2_COIL);

  // Lida com o botão C1 (início ou confirmação do ciclo)
  if (digitalRead(BUTTON_C1) == LOW || modbusC1Pressed) {
    if (state == IDLE) {
      //atualiza o estado
      state = WAITING_CONFIRMATION;
      lastPressTime = millis();
      //displayMessage("Iniciar ciclo? Pressione C1");
      Serial.println("Iniciar ciclo? Pressione C1");
    } else if (state == WAITING_CONFIRMATION) {
      //atualiza o estado
      state = MIXING;
      lastPressTime = millis(); // Reinicia o contador do tempo
      //displayMessage("Iniciando ciclo...");
      Serial.println("Iniciando ciclo...");
    }
    delay(200); // Debounce para evitar múltiplos acionamentos

  }

  // Lida com o botão C2 (interrupção ou esvaziamento)
  if (digitalRead(BUTTON_C2) == LOW || modbusC2Pressed) {
    if (state == MIXING || state == WAITING_CONFIRMATION) {
     //atualiza o estado
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
    //displayMessage("Ciclo concluido");
    Serial.println("Ciclo concluido");
    delay(2000); // Aguarda 2 segundos antes de retornar ao estado IDLE
    resetToIdle();
  }
}

bool checkInterrupt() {
  // Verifica o estado do botão via Modbus
  bool modbusC2Pressed = mb.Coil(C2_COIL);

  if (digitalRead(BUTTON_C2) == LOW || modbusC2Pressed) {
    allOff(); // desliga todos os equipamentos
    state = INTERRUPTED;
    return true;
  }
  return false;
}

void allOff() {
  // Desliga todos os equipamentos e envia status ao supervisório
  digitalWrite(PUMP1, LOW);
  mb.Ists(PUMP1_ISTS, LOW);
  
  digitalWrite(PUMP2, LOW);
  mb.Ists(PUMP2_ISTS, LOW);
  
  digitalWrite(MIXER, LOW);
  mb.Ists(MIXER_ISTS, LOW);
  
  digitalWrite(VALVE, LOW);
  mb.Ists(VALVE_ISTS, LOW);
}

void emptyContainer() {
  //displayMessage("Esvaziando recipiente...");
  Serial.println("Esvaziando recipiente...");

  digitalWrite(VALVE, HIGH);
  mb.Ists(VALVE_ISTS, HIGH); // Envia status do atuador VALVE

  while (digitalRead(LOW_LEVEL) == HIGH) {
    if (state == MIXING && checkInterrupt()) return;
    yield();
  }
  
  digitalWrite(VALVE, LOW);
  mb.Ists(VALVE_ISTS, LOW); // Atualiza status do atuador VALVE
  //displayMessage("Recipiente vazio");
  Serial.println("Recipiente vazio");
  delay(2000);
}

void turnOnMixer() {
  if (state != MIXING) return;
  
  unsigned long startTime = millis();
  //displayMessage("Ligando misturador");
  Serial.println("Ligando misturador");

  digitalWrite(MIXER, HIGH);
  mb.Ists(MIXER_ISTS, HIGH); // Envia status do atuador MIXER

  while (millis() - startTime < MIXER_TIME) {
    if (checkInterrupt() || state != MIXING) return;
    yield();
  }
  
  digitalWrite(MIXER, LOW);
  mb.Ists(MIXER_ISTS, LOW); // Atualiza status do atuador MIXER
}

void turnOnPump1() {
  if (state != MIXING) return;
  
  //displayMessage("Acionando bomba1");
  Serial.println("Acionando bomba 1...");
  unsigned long startTime = millis();

  digitalWrite(PUMP1, HIGH);
  mb.Ists(PUMP1_ISTS, HIGH); // Envia status do atuador PUMP1

  while (millis() - startTime < PUMP_TIME) {
    if (checkInterrupt() || state != MIXING) return;
    yield();
  }
  
  digitalWrite(PUMP1, LOW);
  mb.Ists(PUMP1_ISTS, LOW); // Atualiza status do atuador PUMP1
}

void turnOnPump2() {
  if (state != MIXING) return;

  //displayMessage("Acionando bomba2");
  Serial.println("Acionando bomba 2...");

  digitalWrite(PUMP2, HIGH);
  mb.Ists(PUMP2_ISTS, HIGH); // Envia status do atuador PUMP2

  while (digitalRead(HIGH_LEVEL) == HIGH) {
    if (checkInterrupt() || state != MIXING) return;
    yield();
  }
  
  digitalWrite(PUMP2, LOW);
  mb.Ists(PUMP2_ISTS, LOW); // Atualiza status do atuador PUMP2
}

void resetToIdle() {
  state = IDLE;
  //displayMessage("Sistema pronto");
  Serial.println("Sistema pronto");
}
/*
void displayMessage(String message) {
  if(message.length() >= 16) {
    String message1 = message.substring(0, 16);
    String message2 = message.substring(16, message.length());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message1);
    lcd.setCursor(0, 1);
    lcd.print(message2);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message);
  }
}*/

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
