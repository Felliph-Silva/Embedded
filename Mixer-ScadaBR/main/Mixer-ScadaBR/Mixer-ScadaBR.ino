#ifdef ESP8266
#include <ESP8266WiFi.h>
#else //ESP32
#include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>

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
const int EMPTYING_ISTS = 112;

//Used Pins
const int PUMP1 = 4; //D2
const int PUMP2 = 2;  //D4
const int BUTTON_C1 = 18; //D0
const int BUTTON_C2 = 19; //D1
const int MIXER = 13; //D7
const int VALVE = 15; //D8
const int LOW_LEVEL = 14; //D5
const int HIGH_LEVEL = 12; //D6

// ModbusIP object
ModbusIP mb;

// Estados do ciclo
enum CycleState { IDLE, WAITING_CONFIRMATION, MIXING, EMPTYING, INTERRUPTED };
CycleState state = IDLE;

unsigned long lastPressTime = 0;
const unsigned long TIMEOUT = 10000; // 10 segundos
const unsigned long PUMP_TIME = 5000;  // 5 segundos
const unsigned long MIXER_TIME = 5000; // 5 segundos

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

  // MODBUS
  mb.server();

  // Configuração dos pinos
  pinMode(PUMP1, OUTPUT);
  pinMode(PUMP2, OUTPUT);
  pinMode(MIXER, OUTPUT);
  pinMode(VALVE, OUTPUT);
//pinMode(BUTTON_C1, INPUT_PULLUP);
//pinMode(BUTTON_C2, INPUT_PULLUP);
  pinMode(LOW_LEVEL, INPUT_PULLUP);
  pinMode(HIGH_LEVEL, INPUT_PULLUP);

  // Adiciona os registros - Use addIsts() para inputs digitais
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
  mb.addIsts(EMPTYING_ISTS);

  Serial.println("Sistema pronto!");
}

void loop() {
  // Chama o Modbus
  mb.task();

  // Verifica o estado do ciclo
  switch (state) {
    case IDLE:
      //Envia o estado para o supervisorio
      mb.Ists(IDLE_ISTS, HIGH);
      mb.Ists(WAITING_ISTS, LOW);
      mb.Ists(MIXING_ISTS, LOW);
      mb.Ists(INTERRUPTED_ISTS, LOW);
      mb.Ists(EMPTYING_ISTS, LOW);

      checkButtons();
      break;

    case WAITING_CONFIRMATION:
      //Envia o estado para o supervisorio
      mb.Ists(IDLE_ISTS, LOW);
      mb.Ists(WAITING_ISTS, HIGH);
      mb.Ists(MIXING_ISTS, LOW);
      mb.Ists(INTERRUPTED_ISTS, LOW);
      mb.Ists(EMPTYING_ISTS, LOW);

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
      mb.Ists(EMPTYING_ISTS, LOW);

      runMixingCycle();
      break;

    case EMPTYING:
      //Envia o estado para o supervisorio
      mb.Ists(IDLE_ISTS, LOW);
      mb.Ists(WAITING_ISTS, LOW);
      mb.Ists(MIXING_ISTS, LOW);
      mb.Ists(INTERRUPTED_ISTS, LOW);
      mb.Ists(EMPTYING_ISTS, HIGH);

      emptyContainer();
      break;

    case INTERRUPTED:
      //Envia o estado para o supervisorio
      mb.Ists(IDLE_ISTS, LOW);
      mb.Ists(WAITING_ISTS, LOW);
      mb.Ists(MIXING_ISTS, LOW);
      mb.Ists(INTERRUPTED_ISTS, HIGH);
      mb.Ists(EMPTYING_ISTS, LOW);

      allOff();
      checkButtons();
      break;
  }
}

void checkButtons() {
  // Verifica o estado dos botões via Modbus
  bool modbusC1Pressed = mb.Coil(C1_COIL);
  bool modbusC2Pressed = mb.Coil(C2_COIL);

  // Lida com o botão C1 (início ou confirmação do ciclo)
  if (modbusC1Pressed) {
    //Reseta o botão virtual
    mb.Coil(C1_COIL,LOW);

    if (state == IDLE) {
      // Atualiza o estado para WAITING_CONFIRMATION
      state = WAITING_CONFIRMATION;
      lastPressTime = millis();
      Serial.println("Iniciar ciclo? Pressione C1 para confirmar.");
    } else if (state == WAITING_CONFIRMATION) {
      // Atualiza o estado para MIXING
      state = MIXING;
      lastPressTime = millis(); // Reinicia o contador do tempo
      Serial.println("Iniciando ciclo...");
    } else if (state == INTERRUPTED) {
      // Volta para o estado IDLE
      resetToIdle();
    }
    
    delay(200); //debounce
  }

  // Lida com o botão C2 (interrupção ou esvaziamento)
  else if (modbusC2Pressed) {
    //Reseta o botão virtual
    mb.Coil(C2_COIL,LOW);

    if (state == MIXING) {
      // Atualiza o estado para INTERRUPTED
      state = INTERRUPTED;
    } else if (state == WAITING_CONFIRMATION) {
      // Cancela a operação e volta para o estado IDLE
      resetToIdle();
    } else if (state == IDLE || state == INTERRUPTED) {
      // Atualiza o estado para EMPTYING
      state = EMPTYING;
    }
    
    delay(200); // debounce
  }
}

void runMixingCycle() {
  static unsigned long stepStartTime = 0;
  static int step = 0;

  switch (step) {
    case 0:
    //checkButtons();
      Serial.println("Acionando bomba 1...");
      digitalWrite(PUMP1, HIGH);
      mb.Ists(PUMP1_ISTS, HIGH);
      stepStartTime = millis();
      step = 1;
      break;

    case 1:
      if (millis() - stepStartTime >= PUMP_TIME) {
        digitalWrite(PUMP1, LOW);
        mb.Ists(PUMP1_ISTS, LOW);
        Serial.println("Bomba 1 desligada");
        step = 2;
      }
      break;

    case 2:
      Serial.println("Acionando bomba 2...");
      digitalWrite(PUMP2, HIGH);
      mb.Ists(PUMP2_ISTS, HIGH);
      step = 3;
      break;

    case 3:{
      int HighLevel = digitalRead(HIGH_LEVEL);

      if (HighLevel == LOW) {
        digitalWrite(PUMP2, LOW);
        mb.Ists(PUMP2_ISTS, LOW);
        Serial.println("Bomba 2 desligada");
        step = 4;
      }
      break;
    }

    case 4:
      Serial.println("Ligando misturador");
      digitalWrite(MIXER, HIGH);
      mb.Ists(MIXER_ISTS, HIGH);
      stepStartTime = millis();
      step = 5;
      break;

    case 5:
      if (millis() - stepStartTime >= MIXER_TIME) {
        digitalWrite(MIXER, LOW);
        mb.Ists(MIXER_ISTS, LOW);
        Serial.println("Misturador desligado");
        state = EMPTYING;
        step = 0;//reseta o step
      }
      break;
  }
}

void emptyContainer() {
  static int step = 0;
  
  switch (step) {
    case 0:
      // Válvula fechada, pronta para abrir
      Serial.println("Abrindo válvula...");
      digitalWrite(VALVE, HIGH);  // Abre a válvula
      mb.Ists(VALVE_ISTS, HIGH);  // Atualiza o status via Modbus
      step = 1;  // Passa para o próximo passo
      break;

    case 1: 

      if (digitalRead(LOW_LEVEL) == LOW) {
        // O sensor de nível baixo foi atingido, fecha a válvula
        digitalWrite(VALVE, LOW);  // Fecha a válvula
        mb.Ists(VALVE_ISTS, LOW);  // Atualiza o status via Modbus
        Serial.println("Válvula fechada, recipiente vazio");
        resetToIdle();
        step = 0;//reseta o step
      }
      break;
  }
}

bool checkInterrupt() {
  bool modbusC2Pressed = mb.Coil(C2_COIL);
  if (modbusC2Pressed) {
    mb.Coil(C2_COIL, LOW);
    allOff();
    state = INTERRUPTED;
    return true;
  }
  return false;
}

void allOff() {
  digitalWrite(PUMP1, LOW);
  mb.Ists(PUMP1_ISTS, LOW);
  digitalWrite(PUMP2, LOW);
  mb.Ists(PUMP2_ISTS, LOW);
  digitalWrite(MIXER, LOW);
  mb.Ists(MIXER_ISTS, LOW);
  digitalWrite(VALVE, LOW);
  mb.Ists(VALVE_ISTS, LOW);
}

void resetToIdle(){
  state = IDLE;
  Serial.println("Sistema pronto!");
}
