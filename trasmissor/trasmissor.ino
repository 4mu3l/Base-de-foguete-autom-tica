#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN 9
#define CSN_PIN 10

// Pinos dos Botões
#define BOTAO_START 8
#define BOTAO_STOP  5

RF24 radio(CE_PIN, CSN_PIN);
const byte endereco[6] = "00001"; 

// --- AS DUAS VARIÁVEIS QUE ESTAVAM FALTANDO ---
// Variáveis de memória para o controle do botão STOP
int ultimoEstadoStop = HIGH;
int proximaAcaoStop = 0; // 0 = Parar, 2 = Inverter

void setup() {
  Serial.begin(9600);
  
  // Configura os botões com resistores internos
  pinMode(BOTAO_START, INPUT_PULLUP); 
  pinMode(BOTAO_STOP, INPUT_PULLUP); 

  if (!radio.begin()) {
    Serial.println("Falha ao iniciar o rádio NRF24L01!");
    while (1); 
  }

  radio.setChannel(115); 
  radio.openWritingPipe(endereco);
  radio.setPALevel(RF24_PA_MIN); 
  radio.stopListening();         
  
  Serial.println("Controle Iniciado: START (Pino 8) | STOP (Pino 5)");
}

void loop() {
  int estadoStart = digitalRead(BOTAO_START);
  int estadoStop = digitalRead(BOTAO_STOP);
  
  int comando = -1; // Variável para guardar o que será enviado

  // Lógica do botão START
  if (estadoStart == LOW) {
    comando = 1; 
    proximaAcaoStop = 0; // Reseta a lógica: o próximo clique no STOP vai apenas parar
    Serial.print("Botao START pressionado! Enviando comando 1 (Ligar)... ");
  } 
  // Lógica do botão STOP (detecta apenas o momento do clique)
  else if (estadoStop == LOW && ultimoEstadoStop == HIGH) {
    
    if (proximaAcaoStop == 0) {
      comando = 0; 
      proximaAcaoStop = 2; // Prepara para que o próximo clique seja "Inverter"
      Serial.print("Botao STOP (1º click)! Enviando comando 0 (Parar)... ");
    } 
    else {
      comando = 2; // Usaremos o número 2 para significar "Inverter"
      proximaAcaoStop = 0; // Prepara para que o próximo clique seja "Parar"
      Serial.print("Botao STOP (2º click)! Enviando comando 2 (Inverter)... ");
    }
  }

  // Atualiza o último estado do botão para o próximo ciclo do loop
  ultimoEstadoStop = estadoStop;

  // Só aciona o rádio se algum comando válido foi gerado
  if (comando != -1) {
    bool sucesso = radio.write(&comando, sizeof(comando));
    
    if (sucesso) {
      Serial.println("[OK] Chegou!");
    } else {
      Serial.println("[FALHA] Perdido no ar.");
    }
    
    // Pequeno atraso para não sobrecarregar e atuar como "debounce"
    delay(200); 
  }
}