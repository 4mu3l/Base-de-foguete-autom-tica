#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN 9
#define CSN_PIN 10

// Pinos de controle do Motor A (Esquerda, por exemplo)
int IN1 = 8;
int IN2 = 7;
int ENA = 3; // Suporta PWM

// Pinos de controle do Motor B (Direita, por exemplo)
int IN3 = 6;
int IN4 = 4; 
int ENB = 5; // Suporta PWM

RF24 radio(CE_PIN, CSN_PIN);
const byte endereco[6] = "00001"; 

// Variável de estado: 0 = Parado, 1 = Frente, 2 = Invertido
int estadoAtualDosMotores = 0; 

// --- COMPENSAÇÃO DE FORÇA DOS MOTORES ---
// Se um motor está parando, aumente o valor dele aqui (Máximo é 255)
// Se o motor A é o problema, coloque 255 nele e deixe o B em 204.
int forcaMotorA = 255; 
int forcaMotorB = 204; 

void setup() {
  Serial.begin(9600);
  
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);

  frearMotores();

  if (!radio.begin()) {
    Serial.println("Falha ao iniciar o rádio Receptor!");
    while (1); 
  }

  radio.setChannel(115); 
  radio.openReadingPipe(0, endereco); 
  radio.setPALevel(RF24_PA_MIN);      
  radio.startListening();             
  
  Serial.println("Base de Lançamento Pronta. Aguardando comando...");
}

void loop() {
  int comando = -1; 

  if (radio.available()) {
    radio.read(&comando, sizeof(comando));
    
    // Liga para FRENTE se o comando for 1 e não estiver indo para frente
    if (comando == 1 && estadoAtualDosMotores != 1) {
      Serial.println("Comando [1]: FRENTE!");
      ligarMotores();
      estadoAtualDosMotores = 1;
    } 
    // FREIA se o comando for 0 e não estiver parado
    else if (comando == 0 && estadoAtualDosMotores != 0) {
      Serial.println("Comando [0]: FREIO!");
      frearMotores();
      estadoAtualDosMotores = 0;
    }
    // INVERTE se o comando for 2 e não estiver invertido
    else if (comando == 2 && estadoAtualDosMotores != 2) {
      Serial.println("Comando [2]: TRÁS!");
      inverterMotores();
      estadoAtualDosMotores = 2;
    }
  }
}

// --- Funções Otimizadas ---

void ligarMotores() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  // KICKSTART: Pulso de força máxima (100%) para vencer a inércia
  analogWrite(ENA, 255);
  analogWrite(ENB, 255);
  // AUMENTADO para 100ms para ajudar o motor mais fraco a "pegar no tranco"
  delay(100); 

  // Reduz para a força configurada no topo do código
  analogWrite(ENA, forcaMotorA);
  analogWrite(ENB, forcaMotorB);
}

void inverterMotores() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  // KICKSTART: Pulso de força máxima (100%) para vencer a inércia
  analogWrite(ENA, 255);
  analogWrite(ENB, 255);
  delay(100); 

  // Reduz para a força configurada no topo do código
  analogWrite(ENA, forcaMotorA);
  analogWrite(ENB, forcaMotorB);
}

void frearMotores() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}